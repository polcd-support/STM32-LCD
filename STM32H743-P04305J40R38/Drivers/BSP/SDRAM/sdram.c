/**
 ****************************************************************************************************
 * @file        sdram.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       SDRAM 驱动代码
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
 * V1.0 20230321
 * 第一次发布
 *
 ****************************************************************************************************
 */
 
#include "./SYSTEM/delay/delay.h"
#include "./BSP/SDRAM/sdram.h"


/**
 * @brief       向SDRAM发送命令
 * @param       bankx       : 0,向BANK5上面的SDRAM发送指令
 * @param                     1,向BANK6上面的SDRAM发送指令
 * @param       cmd         : 指令(0,正常模式/1,时钟配置使能/2,预充电所有存储区/3,自动刷新/4,加载模式寄存器/5,自刷新/6,掉电)
 * @param       refresh     : 自刷新次数(cmd=3时有效)
 * @param       regval      : 模式寄存器值
 * @retval      返回值       :0,正常; 1,失败.
 */
uint8_t sdram_send_cmd(uint8_t bankx, uint8_t cmd, uint8_t refresh, uint16_t regval)
{
    uint32_t retry = 0;
    uint32_t tempreg = 0;
    
    tempreg |= cmd << 0;            /* 设置指令 */
    tempreg |= 1 << (4 - bankx);    /* 设置发送指令到bank5还是6 */
    tempreg |= refresh << 5;        /* 设置自刷新次数 */
    tempreg |= regval << 9;         /* 设置模式寄存器的值 */
    FMC_Bank5_6_R->SDCMR = tempreg; /* 配置寄存器 */

    while ((FMC_Bank5_6_R->SDSR & (1 << 5)))    /* 等待指令发送完成 */
    {
        retry++;

        if (retry > 0X1FFFFF)return 1;
    }

    return 0;
}

/**
 * @brief       初始化SDRAM
 * @param       无
 * @retval      无
 */
void sdram_init(void)
{
    uint32_t sdctrlreg = 0, sdtimereg = 0;
    uint16_t mregval = 0;
    
    RCC->AHB4ENR |= 0X1F << 2;  /* 使能PC/PD/PE/PF/PG时钟 */
    RCC->AHB3ENR |= 1 << 12;    /* 使能FMC时钟 */

    
    sys_gpio_set(GPIOC, SYS_GPIO_PIN0 | SYS_GPIO_PIN2 | SYS_GPIO_PIN3,                          /* PC0/2/3 复用功能输出 */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 
                 
    sys_gpio_set(GPIOD, 3 << 0 | 7 << 8 | 3 << 14,                                              /* PD0/1/8/9/10/14/15 复用功能输出 */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 

    sys_gpio_set(GPIOE, 3 << 0 | 0X1FF << 7,                                                    /* PE0/1/7~15 复用功能输出 */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 

    sys_gpio_set(GPIOF, 0X3F << 0 | 0X1F << 11,                                                 /* PF0~5/11~15 复用功能输出 */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 

    sys_gpio_set(GPIOG, 7 << 0 | 3 << 4 | SYS_GPIO_PIN8 | SYS_GPIO_PIN15,                       /* PG0~2/4/5/8/15 复用功能输出 */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 

    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN0 | SYS_GPIO_PIN2 | SYS_GPIO_PIN3, 12);      /* PC0,2,3              AF12 */
    sys_gpio_af_set(GPIOD, 3 << 0 | 7 << 8 | 3 << 14, 12);                          /* PD0/1/8/9/10/14/15   AF12 */
    sys_gpio_af_set(GPIOE, 3 << 0 | 0X1FF << 7, 12);                                /* PE0/1/7~15           AF12 */
    sys_gpio_af_set(GPIOF, 0X3F << 0 | 0X1F << 11, 12);                             /* PF0~5/11~15          AF12 */
    sys_gpio_af_set(GPIOG, 7 << 0 | 3 << 4 | SYS_GPIO_PIN8 | SYS_GPIO_PIN15, 12);   /* PG0~2/4/5/8/15       AF12 */


    sdctrlreg |= 1 << 0;                /* 9位列地址 */
    sdctrlreg |= 2 << 2;                /* 13位行地址 */
    sdctrlreg |= 1 << 4;                /* 16位数据位宽 */
    sdctrlreg |= 1 << 6;                /* 4个内部存区(4 BANKS) */
    sdctrlreg |= 2 << 7;                /* 2个CAS延迟 */
    sdctrlreg |= 0 << 9;                /* 允许写访问 */
    sdctrlreg |= 2 << 10;               /* SDRAM时钟=pll2_r_ck/2=220M/2=110M=9.1ns */
    sdctrlreg |= 1 << 12;               /* 使能突发访问 */
    sdctrlreg |= 0 << 13;               /* 读通道延迟0个pll2_r_ck */
    FMC_Bank5_6_R->SDCR[0] = sdctrlreg; /* 设置FMC BANK5 SDRAM控制寄存器(BANK5和6用于管理SDRAM) */

    sdtimereg |= 1 << 0;                /* 加载模式寄存器到激活时间的延迟为2个时钟周期 */
    sdtimereg |= 7 << 4;                /* 退出自刷新延迟为8个时钟周期 */
    sdtimereg |= 4 << 8;                /* 自刷新时间为7个时钟周期 */
    sdtimereg |= 6 << 12;               /* 行循环延迟为7个时钟周期 */
    sdtimereg |= 1 << 16;               /* 恢复延迟为2个时钟周期 */
    sdtimereg |= 1 << 20;               /* 行预充电延迟为2个时钟周期 */
    sdtimereg |= 1 << 24;               /* 行到列延迟为2个时钟周期 */
    FMC_Bank5_6_R->SDTR[0] = sdtimereg; /* 设置FMC BANK5 SDRAM时序寄存器 */
    FMC_Bank1_R->BTCR[0] |= (uint32_t)1 << 31;  /* 使能FMC */

    sdram_send_cmd(0, 1, 0, 0);         /* 时钟配置使能 */
    delay_us(500);                      /* 至少延迟200us. */
    sdram_send_cmd(0, 2, 0, 0);         /* 对所有存储区预充电 */
    sdram_send_cmd(0, 3, 8, 0);         /* 设置自刷新次数 */
    mregval |= 1 << 0;                  /* 设置突发长度:1(可以是1/2/4/8) */
    mregval |= 0 << 3;                  /* 设置突发类型:连续(可以是连续/交错) */
    mregval |= 2 << 4;                  /* 设置CAS值:2(可以是2/3) */
    mregval |= 0 << 7;                  /* 设置操作模式:0,标准模式 */
    mregval |= 1 << 9;                  /* 设置突发写模式:1,单点访问 */
    sdram_send_cmd(0, 4, 0, mregval);   /* 设置SDRAM的模式寄存器 */

    /**
     * 刷新频率计数器(以SDCLK频率计数),计算方法:
     * COUNT=SDRAM刷新周期/行数-20=SDRAM刷新周期(us)*SDCLK频率(Mhz)/行数
     * 我们使用的SDRAM刷新周期为64ms,SDCLK=220/2=110Mhz,行数为8192(2^13).
     * 所以,COUNT=64*1000*110/8192-20=839
     */
    FMC_Bank5_6_R->SDRTR = 839 << 1;    /* 设置刷新频率计数器 */
}

/**
 * @brief       在指定地址(writeaddr + Bank5_SDRAM_ADDR)开始,连续写入n个字节
 * @param       pbuf        : 字节指针
 * @param       writeaddr   : 要写入的地址
 * @param       n           : 要写入的字节数
 * @retval      无
*/
void fmc_sdram_write_buffer(uint8_t *pbuf, uint32_t writeaddr, uint32_t n)
{
    for (; n != 0; n--)
    {
        *(volatile uint8_t *)(BANK5_SDRAM_ADDR + writeaddr) = *pbuf;
        writeaddr++;
        pbuf++;
    }
}

/**
 * @brief       在指定地址(readaddr + Bank5_SDRAM_ADDR)开始,连续读取n个字节
 * @param       pbuf        : 字节指针
 * @param       readaddr    : 要读取的地址
 * @param       n           : 要读取的字节数
 * @retval      无
*/
void fmc_sdram_read_buffer(uint8_t *pbuf, uint32_t readaddr, uint32_t n)
{
    for (; n != 0; n--)
    {
        *pbuf++ = *(volatile uint8_t *)(BANK5_SDRAM_ADDR + readaddr);
        readaddr++;
    }
}






























