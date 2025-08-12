/**
 ****************************************************************************************************
 * @file        sai.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2023-03-25
 * @brief       SAI 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230325
 * 第一次发布
 * V1.1 20230325
 * 添加sai1_saib_init/sai1_rx_dma_init/sai1_rec_start/sai1_rec_stop等函数,支持录音
 *
 ****************************************************************************************************
 */

#include "./BSP/SAI/sai.h"
#include "./BSP/LCD/ltdc.h"
#include "os.h"


/**
 * @brief       SAI1 Block A初始化, I2S,飞利浦标准
 * @param       mode    : 00,主发送器;01,主接收器;10,从发送器;11,从接收器
 * @param       cpol    : 0,时钟下降沿选通;1,时钟上升沿选通
 * @param       datalen : 数据大小,0/1,未用到,2,8位;3,10位;4,16位;5,20位;6,24位;7,32位.
 * @retval      无
 */
void sai1_saia_init(uint8_t mode, uint8_t cpol, uint8_t datalen)
{
    uint32_t tempreg = 0;

    SAI1_SAI_CLK_ENABLE();              /* SAI 时钟使能 */
    SAI1_MCLK_GPIO_CLK_ENABLE();        /* SAI MCLK GPIO 时钟使能 */
    SAI1_SCK_GPIO_CLK_ENABLE();         /* SAI SCK  GPIO 时钟使能 */
    SAI1_FS_GPIO_CLK_ENABLE();          /* SAI FS GPIO 时钟使能 */
    SAI1_SDA_GPIO_CLK_ENABLE();         /* SAI SDA GPIO 时钟使能 */
    SAI1_SDB_GPIO_CLK_ENABLE();         /* SAI SDB GPIO 时钟使能 */

    RCC->D2CCIP1R &= ~(7 << 0);         /* SAI1SEL[2:0]清零 */
    RCC->D2CCIP1R |= 2 << 0;            /* SAI1SEL[2:0]=2,选择pll3_p_ck作为SAI1的时钟源 */
    
    RCC->APB2RSTR |= 1 << 22;           /* 复位SAI1 */
    RCC->APB2RSTR &= ~(1 << 22);        /* 结束复位 */

    sys_gpio_set(SAI1_MCLK_GPIO_PORT, SAI1_MCLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* MCLK 引脚模式设置(复用输出) */

    sys_gpio_set(SAI1_SCK_GPIO_PORT, SAI1_SCK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SCK 引脚模式设置(复用输出) */

    sys_gpio_set(SAI1_FS_GPIO_PORT, SAI1_FS_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* FS 引脚模式设置(复用输出) */
    
    sys_gpio_set(SAI1_SDA_GPIO_PORT, SAI1_SDA_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SDA 引脚模式设置(复用输出) */
    
    sys_gpio_set(SAI1_SDB_GPIO_PORT, SAI1_SDB_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SDB 引脚模式设置(复用输出) */

    sys_gpio_af_set(SAI1_MCLK_GPIO_PORT,  SAI1_MCLK_GPIO_PIN,  SAI1_MCLK_GPIO_AF);  /* MCLK脚, AF功能设置 */
    sys_gpio_af_set(SAI1_SCK_GPIO_PORT,  SAI1_SCK_GPIO_PIN,  SAI1_SCK_GPIO_AF);     /* SCK脚, AF功能设置 */
    sys_gpio_af_set(SAI1_FS_GPIO_PORT, SAI1_FS_GPIO_PIN, SAI1_FS_GPIO_AF);          /* FS脚, AF功能设置 */
    sys_gpio_af_set(SAI1_SDA_GPIO_PORT,  SAI1_SDA_GPIO_PIN,  SAI1_SDA_GPIO_AF);     /* SDA脚, AF功能设置 */
    sys_gpio_af_set(SAI1_SDB_GPIO_PORT,  SAI1_SDB_GPIO_PIN,  SAI1_SDB_GPIO_AF);     /* SDB脚, AF功能设置 */

    tempreg |= mode << 0;           /* 设置SAI1工作模式 */
    tempreg |= 0 << 2;              /* 设置SAI1协议为:自由协议(支持I2S/LSB/MSB/TDM/PCM/DSP等协议) */
    tempreg |= datalen << 5;        /* 设置数据大小 */
    tempreg |= 0 << 8;              /* 数据MSB位优先 */
    tempreg |= (uint16_t)cpol << 9; /* 数据在时钟的上升/下降沿选通 */
    tempreg |= 0 << 10;             /* 音频模块异步 */
    tempreg |= 0 << 12;             /* 立体声模式 */
    tempreg |= 1 << 13;             /* 立即驱动音频模块输出 */
    tempreg |= 0 << 19;             /* 使能主时钟分频器(MCKDIV) */
    SAI1_SAIA->CR1 = tempreg;       /* 设置CR1寄存器 */

    tempreg = (64 - 1) << 0;        /* 设置帧长度为64,左通道32个SCK,右通道32个SCK */
    tempreg |= (32 - 1) << 8;       /* 设置帧同步有效电平长度,在I2S模式下=1/2帧长 */
    tempreg |= 1 << 16;             /* FS信号为SOF信号+通道识别信号 */
    tempreg |= 0 << 17;             /* FS低电平有效(下降沿) */
    tempreg |= 1 << 18;             /* 在slot0的第一位的前一位使能FS,以匹配飞利浦标准 */
    SAI1_SAIA->FRCR = tempreg;

    tempreg = 0 << 0;               /* slot偏移(FBOFF)为0 */
    tempreg |= 2 << 6;              /* slot大小为32位 */
    tempreg |= (2 - 1) << 8;        /* slot数为2个 */
    tempreg |= (1 << 17) | (1 << 16);   /* 使能slot0和slot1 */
    SAI1_SAIA->SLOTR = tempreg;     /* 设置slot */

    SAI1_SAIA->CR2 = 1 << 0;        /* 设置FIFO阀值:1/4 FIFO */
    SAI1_SAIA->CR2 |= 1 << 3;       /* FIFO刷新 */
}

/**
 * @brief       SAI1 Block B初始化, I2S,飞利浦标准, 同步模式, 不初始化IO
 * @param       mode    : 00,主发送器;01,主接收器;10,从发送器;11,从接收器
 * @param       cpol    : 0,时钟下降沿选通;1,时钟上升沿选通
 * @param       datalen : 数据大小,0/1,未用到,2,8位;3,10位;4,16位;5,20位;6,24位;7,32位.
 * @retval      无
 */
void sai1_saib_init(uint8_t mode, uint8_t cpol, uint8_t datalen)
{
    uint32_t tempreg = 0;

    tempreg |= mode << 0;           /* 设置SAI1工作模式 */
    tempreg |= 0 << 2;              /* 设置SAI1协议为:自由协议(支持I2S/LSB/MSB/TDM/PCM/DSP等协议) */
    tempreg |= datalen << 5;        /* 设置数据大小 */
    tempreg |= 0 << 8;              /* 数据MSB位优先 */
    tempreg |= (uint16_t)cpol << 9; /* 数据在时钟的上升/下降沿选通 */
    tempreg |= 1 << 10;             /* 使能同步模式 */
    tempreg |= 0 << 12;             /* 立体声模式 */
    tempreg |= 1 << 13;             /* 立即驱动音频模块输出 */
    SAI1_Block_B->CR1 = tempreg;    /* 设置CR1寄存器 */

    tempreg = (64 - 1) << 0;        /* 设置帧长度为64,左通道32个SCK,右通道32个SCK */
    tempreg |= (32 - 1) << 8;       /* 设置帧同步有效电平长度,在I2S模式下=1/2帧长 */
    tempreg |= 1 << 16;             /* FS信号为SOF信号+通道识别信号 */
    tempreg |= 0 << 17;             /* FS低电平有效(下降沿) */
    tempreg |= 1 << 18;             /* 在slot0的第一位的前一位使能FS,以匹配飞利浦标准 */
    SAI1_Block_B->FRCR = tempreg;

    tempreg = 0 << 0;               /* slot偏移(FBOFF)为0 */
    tempreg |= 2 << 6;              /* slot大小为32位 */
    tempreg |= (2 - 1) << 8;        /* slot数为2个 */
    tempreg |= (1 << 17) | (1 << 16);   /* 使能slot0和slot1 */
    SAI1_Block_B->SLOTR = tempreg;  /* 设置slot */

    SAI1_Block_B->CR2 = 1 << 0;     /* 设置FIFO阀值:1/4 FIFO */
    SAI1_Block_B->CR2 |= 1 << 3;    /* FIFO刷新 */
    SAI1_Block_B->CR1 |= 1 << 17;   /* 使能DMA */
    SAI1_Block_B->CR1 |= 1 << 16;   /* 使能SAI1 Block B */
}

/**
 * SAI Block A采样率设置
 * 采样率计算公式:
 * MCKDIV!=0: Fs=SAI_CK_x/[512*MCKDIV]
 * MCKDIV==0: Fs=SAI_CK_x/256
 * SAI_CK_x=(HSE/pllm)*PLLI2SN/PLLI2SQ/(PLLI2SDIVQ+1)
 * 一般HSE=25Mhz
 * pllm:在sys_stm32_clock_init设置的时候确定，一般是25
 * PLLI2SN      : 192~432
 * PLLI2SQ      : 2~15
 * PLLI2SDIVQ   : 0~31
 * MCKDIV       : 0~15
 * SAI A分频系数表@pllm = 25, HSE = 25Mhz,即vco输入频率为1Mhz
 */

/* SAI Block A采样率设置 
 * 采样率计算公式(以NOMCK=0,OSR=0为前提):
 * Fmclk = 256*Fs = sai_x_ker_ck / MCKDIV[5:0]
 * Fs = sai_x_ker_ck / (256 * MCKDIV[5:0])
 * Fsck = Fmclk * (FRL[7:0]+1) / 256 = Fs * (FRL[7:0] + 1)
 * 其中:
 * sai_x_ker_ck = (HSE / PLL3DIVM) * (PLL3DIVN + 1) / (PLL3DIVP + 1)
 * HSE:一般为25Mhz
 * PLL3DIVM     :1~63,表示1~63分频
 * PLL3DIVN     :3~511,表示4~512倍频
 * PLL3DIVP     :0~127,表示1~128分频
 * MCKDIV       :0~63,表示1~63分频,0也是1分频,推荐设置为偶数
 * SAI A分频系数表@PLL3DIVM=25,HSE=25Mhz,即vco输入频率为1Mhz
 * 表格式:
 * 采样率|(PLL3DIVN+1)|(PLL3DIVP+1)|MCKDIV
 */
const uint16_t SAI_PSC_TBL[][5] =
{
    {800, 256, 5, 25},      /* 8Khz采样率 */
    {1102, 302, 107, 0},    /* 11.025Khz采样率 */
    {1600, 426, 2, 52},     /* 16Khz采样率 */
    {2205, 429, 38, 2},     /* 22.05Khz采样率 */
    {3200, 426, 1, 52},     /* 32Khz采样率 */
    {4410, 429, 1, 38},     /* 44.1Khz采样率 */
    {4800, 467, 1, 38},     /* 48Khz采样率 */
    {8820, 429, 1, 19},     /* 88.2Khz采样率 */
    {9600, 467, 1, 19},     /* 96Khz采样率 */
    {17640, 271, 1, 6},     /* 176.4Khz采样率 */
    {19200, 295, 6, 0},     /* 192Khz采样率 */
};


/******************************************************************************************/
/* 以下代码用于适配RGB屏的像素时钟，避免SAI改变PLL3设置后，像素时钟过高导致异常的问题
 * 如果使用的是MCU屏，则客户忽略这部分代码 
 */

/* 分辨率:像素时钟对应表,前面为横向分辨率,后面为目标像素时钟频率 */
const uint16_t ltdc_clk_table[4][2] =
{
    480, 9,
    800, 33,
    1024, 50,
    1280, 50,
};

/**
 * @brief       专门针对LTDC的一个时钟设置函数
 * @param       pll3n     : 当前的pll3倍频系数
 * @retval      无
 */
void sai_ltdc_clkset(uint16_t pll3n)
{
    uint8_t i = 0;
    uint8_t pll3r = 0;

    if (lcdltdc.pwidth == 0)return;         /* 不是RGB屏,不需要设置 */

    for (i = 0; i < 4; i++)                 /* 查找是否有对应RGB屏的分辨率 */
    {
        if (lcdltdc.pwidth == ltdc_clk_table[i][0])
        {
            break;                          /* 找到了则直接退出 */
        }
    }

    if (i == 4)return;                      /* 找遍了也没找到对应的分辨率 */

    pll3r = pll3n / ltdc_clk_table[i][1];   /* 得到计算后的pll3r的值 */

    if (pll3n > (pll3r * ltdc_clk_table[i][1]))
    {
        pll3r += 1;
    }
    
    ltdc_clk_set(pll3n, 25, pll3r);         /* 重新设置PLL3的R分频 */
}
/******************************************************************************************/

/**
 * @brief       设置SAI1的采样率(@MCKEN)
 * @param       samplerate  : 采样率, 单位:Hz
 * @retval      0,设置成功
 *              1,无法设置
 */
uint8_t sai1_samplerate_set(uint32_t samplerate)
{
    uint16_t retry = 0;
    uint8_t i = 0;
    uint32_t tempreg = 0;
    samplerate /= 10;   /* 缩小10倍 */

    for (i = 0; i < (sizeof(SAI_PSC_TBL) / 10); i++)    /* 看看改采样率是否可以支持 */
    {
        if (samplerate == SAI_PSC_TBL[i][0])break;
    }

    RCC->CR &= ~(1 << 28);                              /* 先关闭PLLI3 */

    if (i == (sizeof(SAI_PSC_TBL) / 10))return 1;       /* 搜遍了也找不到 */

    while (((RCC->CR & (1 << 29))) && (retry < 0X1FFF)) /* 等待PLL3时钟失锁 */
    {
        retry++; 
    }
    
    RCC->PLLCKSELR &= ~(0X3F << 20);                    /* 清除DIVM3[5:0]原来的设置 */
    RCC->PLLCKSELR |= 25 << 20;                         /* DIVM3[5:0]=25,设置PLL3的预分频系数 */
    tempreg = RCC->PLL3DIVR;                            /* 读取PLL3DIVR的值 */
    tempreg &= 0XFFFF0000;                              /* 清除DIVN和PLL3DIVP原来的设置 */
    tempreg |= (uint32_t)(SAI_PSC_TBL[i][1] - 1) << 0;  /* 设置DIVN[8:0] */
    tempreg |= (uint32_t)(SAI_PSC_TBL[i][2] - 1) << 9;  /* 设置DIVP[6:0] */
    RCC->PLL3DIVR = tempreg;                            /* 设置PLL3DIVR寄存器 */
    RCC->PLLCFGR |= 1 << 22;                            /* DIVP3EN=1,使能pll3_p_ck */
    RCC->CR |= 1 << 28;                                 /* 开启PLL3 */

    while ((RCC->CR & 1 << 29) == 0);                   /* 等待PLL3开启成功 */

    tempreg = SAI1_Block_A->CR1;
    tempreg &= ~(0X3F << 20);                           /* 清除MCKDIV[5:0]设置 */
    tempreg |= (uint32_t)SAI_PSC_TBL[i][3] << 20;       /* 设置MCKDIV[5:0] */
    tempreg |= 1 << 16;                                 /* 使能SAI1 Block A */
    tempreg |= 1 << 17;                                 /* 使能DMA */
    SAI1_Block_A->CR1 = tempreg;                        /* 配置MCKDIV[5:0],同时使能SAI1 Block A */

    sai_ltdc_clkset(SAI_PSC_TBL[i][1]);                 /* 针对RGB屏,需要重新修改PLL3R的值 */
    
    return 0;
}

/**
 * @brief       SAI1 TX DMA配置
 *  @note       设置为双缓冲模式,并开启DMA传输完成中断
 * @param       buf0    : M0AR地址.
 * @param       buf1    : M1AR地址.
 * @param       num     : 每次传输数据量
 * @param       width   : 位宽(存储器和外设,同时设置),0,8位;1,16位;2,32位;
 * @retval      无
 */
void sai1_tx_dma_init(uint8_t *buf0, uint8_t *buf1, uint16_t num, uint8_t width)
{
    SAI1_TX_DMA_CLK_ENABLE();           /* SAI1 DMA时钟使能 */

    while (SAI1_TX_DMASx->CR & 0X01);   /* 等待SAI1_TX_DMASx可配置 */

    /* 详见<<STM32H7xx参考手册>>16.3.2节,Table 119 */
    DMAMUX1_Channel11->CCR = SAI1_TX_DMASx_Channel; /* DMA2_Stream3的通道选择: 87,即SAI1_A对应的通道 */

    SAI1_TX_DMASx_CLR_TC();             /* 清除一次中断 */ 
    SAI1_TX_DMASx->FCR = 0X0000021;     /* 设置为默认值 */ 

    SAI1_TX_DMASx->PAR = (uint32_t)&SAI1_SAIA->DR;  /* 外设地址为:SAI1_SAI->DR */ 
    SAI1_TX_DMASx->M0AR = (uint32_t)buf0;           /* 内存1地址 */ 
    SAI1_TX_DMASx->M1AR = (uint32_t)buf1;           /* 内存2地址 */ 
    SAI1_TX_DMASx->NDTR = num;          /* 暂时设置长度为1 */ 
    SAI1_TX_DMASx->CR = 0;              /* 先全部复位CR寄存器值 */ 
    SAI1_TX_DMASx->CR |= 1 << 6;        /* 存储器到外设模式 */ 
    SAI1_TX_DMASx->CR |= 1 << 8;        /* 循环模式 */ 
    SAI1_TX_DMASx->CR |= 0 << 9;        /* 外设非增量模式 */ 
    SAI1_TX_DMASx->CR |= 1 << 10;       /* 存储器增量模式 */ 
    SAI1_TX_DMASx->CR |= (uint16_t)width << 11;     /* 外设数据长度:16位/32位 */ 
    SAI1_TX_DMASx->CR |= (uint16_t)width << 13;     /* 存储器数据长度:16位/32位 */ 
    SAI1_TX_DMASx->CR |= 2 << 16;       /* 高优先级 */ 
    SAI1_TX_DMASx->CR |= 1 << 18;       /* 双缓冲模式 */ 
    SAI1_TX_DMASx->CR |= 0 << 21;       /* 外设突发单次传输 */ 
    SAI1_TX_DMASx->CR |= 0 << 23;       /* 存储器突发单次传输 */ 
    SAI1_TX_DMASx->CR |= 0 << 25;       /* 选择通道0 SAI1_A通道 */ 

    SAI1_TX_DMASx->FCR &= ~(1 << 2);    /* 不使用FIFO模式 */ 
    SAI1_TX_DMASx->FCR &= ~(3 << 0);    /* 无FIFO 设置 */ 

    SAI1_TX_DMASx->CR |= 1 << 4;        /* 开启传输完成中断 */ 
    sys_nvic_init(0, 0, SAI1_TX_DMASx_IRQn, 2);     /* 抢占1，子优先级0，组2 */ 
}

/**
 * @brief       SAI1 TX DMA配置
 *  @note       设置为双缓冲模式,并开启DMA传输完成中断
 * @param       buf0    : M0AR地址.
 * @param       buf1    : M1AR地址.
 * @param       num     : 每次传输数据量
 * @param       width   : 位宽(存储器和外设,同时设置),0,8位;1,16位;2,32位;
 * @retval      无
 */
void sai1_rx_dma_init(uint8_t *buf0, uint8_t *buf1, uint16_t num, uint8_t width)
{
    SAI1_RX_DMA_CLK_ENABLE();           /* SAI1 RX DMA时钟使能 */

    while (SAI1_RX_DMASx->CR & 0X01);   /* 等待SAI1_RX_DMASx可配置 */
    
    /* 详见<<STM32H7xx参考手册>>16.3.2节,Table 119 */
    DMAMUX1_Channel13->CCR = SAI1_RX_DMASx_Channel; /* DMA2_Stream5的通道选择: 88,即SAI1_B对应的通道 */

    SAI1_RX_DMASx_CLR_TC();             /* 清除一次中断 */ 
    SAI1_RX_DMASx->FCR = 0X0000021;     /* 设置为默认值 */ 

    SAI1_RX_DMASx->PAR = (uint32_t)&SAI1_SAIB->DR;  /* 外设地址为:SAI1_SAI->DR */ 
    SAI1_RX_DMASx->M0AR = (uint32_t)buf0;           /* 内存1地址 */ 
    SAI1_RX_DMASx->M1AR = (uint32_t)buf1;           /* 内存2地址 */ 
    SAI1_RX_DMASx->NDTR = num;          /* 暂时设置长度为1 */ 
    SAI1_RX_DMASx->CR = 0;              /* 先全部复位CR寄存器值 */ 
    SAI1_RX_DMASx->CR |= 0 << 6;        /* 外设到存储器模式 */ 
    SAI1_RX_DMASx->CR |= 1 << 8;        /* 循环模式 */ 
    SAI1_RX_DMASx->CR |= 0 << 9;        /* 外设非增量模式 */ 
    SAI1_RX_DMASx->CR |= 1 << 10;       /* 存储器增量模式 */ 
    SAI1_RX_DMASx->CR |= (uint16_t)width << 11;     /* 外设数据长度:16位/32位 */ 
    SAI1_RX_DMASx->CR |= (uint16_t)width << 13;     /* 存储器数据长度:16位/32位 */ 
    SAI1_RX_DMASx->CR |= 1 << 16;       /* 中等优先级 */ 
    SAI1_RX_DMASx->CR |= 1 << 18;       /* 双缓冲模式 */ 
    SAI1_RX_DMASx->CR |= 0 << 21;       /* 外设突发单次传输 */ 
    SAI1_RX_DMASx->CR |= 0 << 23;       /* 存储器突发单次传输 */ 
    SAI1_RX_DMASx->CR |= 0 << 25;       /* 选择通道0 SAI1_A通道 */ 

    SAI1_RX_DMASx->FCR &= ~(1 << 2);    /* 不使用FIFO模式 */ 
    SAI1_RX_DMASx->FCR &= ~(3 << 0);    /* 无FIFO 设置 */ 

    SAI1_RX_DMASx->CR |= 1 << 4;        /* 开启传输完成中断 */ 
    sys_nvic_init(0, 1, SAI1_RX_DMASx_IRQn, 2);     /* 抢占0，子优先级1，组2 */ 
}

/* SAI1 DMA回调函数指针 */ 
void (*sai1_tx_callback)(void); /* TX回调函数 */ 
void (*sai1_rx_callback)(void); /* RX回调函数 */ 

/**
 * @brief       SAI1 TX DMA 中断服务函数
 * @param       无
 * @retval      无
 */
void SAI1_TX_DMASx_IRQHandler(void)
{
    OSIntEnter();
    if (SAI1_TX_DMASx_IS_TC())      /* SAI1_TX_DMASx,传输完成标志 */ 
    {
        SAI1_TX_DMASx_CLR_TC();     /* 清除传输完成中断 */ 
        sai1_tx_callback();         /* 执行回调函数,读取数据等操作在这里面处理 */
        SCB_CleanInvalidateDCache();/* 清除无效的D-Cache */
    }
    OSIntExit();
}

/**
 * @brief       SAI1 RX DMA 中断服务函数
 * @param       无
 * @retval      无
 */
void SAI1_RX_DMASx_IRQHandler(void)
{
    OSIntEnter();
    if (SAI1_RX_DMASx_IS_TC())      /* SAI1_RX_DMASx,传输完成标志 */ 
    {
        SAI1_RX_DMASx_CLR_TC();     /* 清除传输完成中断 */ 
        sai1_rx_callback();         /* 执行回调函数,读取数据等操作在这里面处理 */ 
        SCB_CleanInvalidateDCache();/* 清除无效的D-Cache */
    }
    OSIntExit();
}

/**
 * @brief       SAI开始播放
 * @param       无
 * @retval      无
 */
void sai1_play_start(void)
{
    SAI1_TX_DMASx->CR |= 1 << 0;    /* 开启DMA TX传输 */ 
}

/**
 * @brief       SAI停止播放
 * @param       无
 * @retval      无
 */
void sai1_play_stop(void)
{
    SAI1_TX_DMASx->CR &= ~(1 << 0); /* 结束播放 */ 
}

/**
 * @brief       SAI开始录音
 * @param       无
 * @retval      无
 */
void sai1_rec_start(void)
{
    SAI1_RX_DMASx->CR |= 1 << 0;    /* 开启DMA RX传输 */ 
}

/**
 * @brief       SAI关闭录音
 * @param       无
 * @retval      无
 */
void sai1_rec_stop(void)
{
    SAI1_RX_DMASx->CR &= ~(1 << 0); /* 结束录音 */ 
}

















