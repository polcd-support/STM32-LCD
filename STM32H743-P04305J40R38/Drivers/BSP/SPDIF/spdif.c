/**
 ****************************************************************************************************
 * @file        spdif.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-25
 * @brief       SPDIF 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230325
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/SPDIF/spdif.h"
#include "./SYSTEM/delay/delay.h"
#include "os.h"


spdif_rx_dev spdif_dev;     /* SPDIF控制结构体 */


/**
 * @brief       SPDIF RX初始化
 * @param       无
 * @retval      无
 */
void spdif_rx_init(void)
{
    uint32_t tempreg;
    
    RCC->APB1LENR |= 1 << 16;            /* 使能SPDIF RX时钟 */

    SPDIF_RX_GPIO_CLK_ENABLE();         /* 使能SPDIF RX脚GPIO时钟 */

    sys_gpio_set(SPDIF_RX_GPIO_PORT,    /* SPDIF RX 所在GPIO 端口 */
                 SPDIF_RX_GPIO_PIN,     /* SPDIF RX 所在GPIO 引脚号 */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);

    sys_gpio_af_set(SPDIF_RX_GPIO_PORT,  SPDIF_RX_GPIO_PIN,  SPDIF_RX_GPIO_AF); /* SPDIF RX脚, AF功能设置 */

    RCC->D2CCIP1R &= ~(3 << 20);        /* SPDIFSEL[1:0]清零 */
    RCC->D2CCIP1R |= 0 << 20;           /* SPDIFSEL[1:0]=0,选择pll1_q_ck,作为SPDIF时钟源,频率为200Mhz */
    spdif_dev.clock = 200000000;        /* 设置SPDIF CLK的频率,为200Mhz,要支持192Khz采样率必须保证clock≥135.2Mhz */
    
    spdif_rx_mode(SPDIF_RX_IDLE);       /* 先进入IDLE模式 */
    tempreg = 1 << 16;                  /* 选择SPDIF RX通道1(可选：0,1,2,3) */
    tempreg |= 1 << 14;                 /* 等待SPDIF RX线路上的活动 */
    tempreg |= 2 << 12;                 /* 同步阶段,允许最大重试15次 */
    tempreg |= 0 << 11;                 /* 从通道A获取通道状态 */
    tempreg |= 0 << 10;                 /* SPDIF传输通道状态和用户信息不使用DMA接收 */
    tempreg |= 1 << 9;                  /* 报头类型不写入DR */
    tempreg |= 1 << 8;                  /* 通道状态和用户位不写入DR */
    tempreg |= 1 << 7;                  /* 有效性位不写入DR */
    tempreg |= 1 << 6;                  /* 奇偶校验错误位不写入DR */
    tempreg |= 0 << 4;                  /* 数据采用右对齐(LSB),最高支持24位音频数据流 */
    tempreg |= 1 << 3;                  /* 立体声模式 */
    tempreg |= 1 << 2;                  /* SPDIF音频数据使用DMA来接收 */
    SPDIFRX->CR = tempreg;
    SPDIFRX->IMR |= (1 << 6) | (1 << 2);    /* 使能奇偶校验错误中断和串行接口错误中断 */

    sys_nvic_init(1, 0, SPDIF_RX_IRQn, 2);  /* 设置SPDIF RX中断,抢占1，子优先级0，组2 */
}

/**
 * @brief       设置SPDIF的工作模式
 * @param       mode        : 0, IDLE模式
 *                            1, RX同步模式
 *                            2, 保留
 *                            3, 正常工作模式
 * @retval      无
 */
void spdif_rx_mode(uint8_t mode)
{
    uint32_t tempreg = 0;
    tempreg = SPDIFRX->CR;
    tempreg &= ~(0X03);
    tempreg |= mode & 0X03;         /* 设置模式 */
    SPDIFRX->CR = tempreg;
}

/**
 * @brief       SPDIF RX数据DMA配置
 *  @note       设置为双缓冲模式,并开启DMA传输完成中断
 * @param       buf0        : M0AR地址
 * @param       buf1        : M1AR地址
 * @param       num         : 每次传输数据量
 * @param       width       : 位宽(存储器和外设,同时设置),0,8位; 1,16位; 2,32位;
 * @retval      无
 */
void spdif_rx_dma_init(uint32_t *buf0, uint32_t *buf1, uint16_t num, uint8_t width)
{
    RCC->AHB1ENR |= 1 << 0;             /* DMA1时钟使能 */

    while (DMA1_Stream1->CR & 0X01);    /* 等待DMA1_Stream1可配置 */

    /* 详见<<STM32H7xx参考手册>>16.3.2节,Table 116 */
    DMAMUX1_Channel1->CCR = 93;         /* DMA1_Stream1的通道选择: 93,即SPDIFRX_DT对应的通道 */
    
    DMA1->LIFCR |= 0X3D << 6 * 0;       /* 清空通道1上所有中断标志 */
    DMA1_Stream1->FCR = 0X0000021;      /* 设置为默认值 */

    DMA1_Stream1->PAR = (uint32_t)&SPDIFRX->DR; /* 外设地址为:SPDIFRX->DR */
    DMA1_Stream1->M0AR = (uint32_t)buf0;        /* 内存1地址 */
    DMA1_Stream1->M1AR = (uint32_t)buf1;        /* 内存2地址 */
    DMA1_Stream1->NDTR = num;                   /* 设置长度 */
    DMA1_Stream1->CR = 0;                       /* 先全部复位CR寄存器值 */
    DMA1_Stream1->CR |= 0 << 6;                 /* 外设到存储器模式 */
    DMA1_Stream1->CR |= 1 << 8;                 /* 循环模式 */
    DMA1_Stream1->CR |= 0 << 9;                 /* 外设非增量模式 */
    DMA1_Stream1->CR |= 1 << 10;                /* 存储器增量模式 */
    DMA1_Stream1->CR |= (uint16_t)width << 11;  /* 外设数据长度:16位/32位 */
    DMA1_Stream1->CR |= (uint16_t)width << 13;  /* 存储器数据长度:16位/32位 */
    DMA1_Stream1->CR |= 2 << 16;                /* 高优先级 */
    DMA1_Stream1->CR |= 1 << 18;                /* 双缓冲模式 */
    DMA1_Stream1->CR |= 0 << 21;                /* 外设突发单次传输 */
    DMA1_Stream1->CR |= 0 << 23;                /* 存储器突发单次传输 */
    DMA1_Stream1->CR |= 0 << 25;                /* 选择通道0 SPDIF RX DR通道 */

    DMA1_Stream1->FCR &= ~(1 << 2);             /* 不使用FIFO模式 */
    DMA1_Stream1->FCR &= ~(3 << 0);             /* 无FIFO 设置 */
}

/**
 * @brief       等待进入同步状态
 *  @note       同步完成以后自动进入接收状态
 * @param       无
 * @retval      0, 未同步
 *              1, 已同步
 */
uint8_t spdif_rx_wait_sync(void)
{
    uint8_t res = 0;
    uint8_t timeout = 0;
    spdif_rx_mode(SPDIF_RX_SYNC);       /* 设置为同步模式 */

    while (1)
    {
        timeout++;
        delay_ms(2);

        if (timeout > 100)break;

        if (SPDIFRX->SR & (1 << 5))     /* 同步完成? */
        {
            res = 1;                    /* 标记同步完成 */
            spdif_rx_mode(SPDIF_RX_RCV);/* 进入接收模式 */
            break;
        }
    }

    return res;
}

/**
 * @brief       获取SPDIF RX收到的音频采样率
 * @param       无
 * @retval      0, 错误的采样率
 *              其他值, 音频采样率
 */
uint32_t spdif_rx_get_samplerate(void)
{
    uint16_t spdif_w5;
    uint32_t samplerate;
    
    spdif_w5 = SPDIFRX->SR >> 16;
    samplerate = (spdif_dev.clock * 5) / (spdif_w5 & 0X7FFF);
    samplerate >>= 6;       /* 除以64 */

    if ((8000 - 1500 <= samplerate) && (samplerate <= 8000 + 1500))samplerate = 8000;           /* 8K的采样率 */
    else if ((11025 - 1500 <= samplerate) && (samplerate <= 11025 + 1500))samplerate = 11025;   /* 11.025K的采样率 */
    else if ((16000 - 1500 <= samplerate) && (samplerate <= 16000 + 1500))samplerate = 16000;   /* 16K的采样率 */
    else if ((22050 - 1500 <= samplerate) && (samplerate <= 22050 + 1500))samplerate = 22050;   /* 22.05K的采样率 */
    else if ((32000 - 1500 <= samplerate) && (samplerate <= 32000 + 1500))samplerate = 32000;   /* 32K的采样率 */
    else if ((44100 - 1500 <= samplerate) && (samplerate <= 44100 + 1500))samplerate = 44100;   /* 44.1K的采样率 */
    else if ((48000 - 1500 <= samplerate) && (samplerate <= 48000 + 1500))samplerate = 48000;   /* 48K的采样率 */
    else if ((88200 - 1500 <= samplerate) && (samplerate <= 88200 + 1500))samplerate = 88200;   /* 88.2K的采样率 */
    else if ((96000 - 1500 <= samplerate) && (samplerate <= 96000 + 1500))samplerate = 96000;   /* 96K的采样率 */
    else if ((176400 - 6000 <= samplerate) && (samplerate <= 176400 + 6000))samplerate = 176400;/* 176.4K的采 */
    else if ((192000 - 6000 <= samplerate) && (samplerate <= 192000 + 6000))samplerate = 192000;/* 192K的采 */
    else samplerate = 0;

    return samplerate;
}

/* SAI DMA回调函数指针 */
void (*spdif_rx_stop_callback)(void);   /* 回调函数 */

/**
 * @brief       SPDIF接收中断服务函数
 * @param       无
 * @retval      无
 */
void SPDIF_RX_IRQHandler(void)
{
    uint32_t sr = SPDIFRX->SR;
    
    OSIntEnter();

    /* 发生超时、同步和帧错误中断,这三个中断一定要处理！*/
    if (sr & ((1 << 8) | (1 << 7) | (1 << 6)))  /* 超时错误/同步错误/帧错误 */
    {
        spdif_rx_stop();                /* 发生错误，关闭SPDIF播放 */
        spdif_rx_stop_callback();       /* 调用回调函数 */
        spdif_rx_mode(SPDIF_RX_IDLE);   /* 当发生超时、同步和帧错误的时候要将SPDIFRXEN写0来清除中断 */
    }

    if (sr & (1 << 3))                  /* 发生了上溢错误 */
    {
        SPDIFRX->IFCR |= 1 << 3;        /* 清除上溢错误 */
    }

    if (sr & (1 << 2))                  /* 发生了奇偶校验错误 */
    {
        SPDIFRX->IFCR |= 1 << 2;        /* 清除奇偶校验错误 */
    }
    
    OSIntExit(); 
}

/**
 * @brief       SPDIF开始播放
 * @param       无
 * @retval      无
 */
void spdif_rx_start(void)
{
    spdif_dev.consta = 1;           /* 标记已经打开SPDIF */
    DMA1_Stream1->CR |= 1 << 0;     /* 开启SPDIF DR RX传输 */
}

/**
 * @brief       SPDIF关闭播放
 * @param       无
 * @retval      无
 */
void spdif_rx_stop(void)
{
    spdif_dev.consta = 0;           /* 标记已经关闭SPDIF */
    DMA1_Stream1->CR &= ~(1 << 0);  /* 结束SPDIF DMA传输 */
}











