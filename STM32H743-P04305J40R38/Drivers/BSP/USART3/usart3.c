/**
 ****************************************************************************************************
 * @file        usart3.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-10-29
 * @brief       串口3 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20221029
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/USART3/usart3.h"
#include "./BSP/TIMER/timer.h"
#include "./MALLOC/malloc.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ucos_ii.h"

/* 发送 */
__align(8) uint8_t g_usart3_tx_buf[USART3_MAX_SEND_LEN];         /* 接收缓冲,最大USART3_MAX_RECV_LEN个字节 */
/* 串口接收缓存区 */
uint8_t g_usart3_rx_buf[USART3_MAX_RECV_LEN];         /* 接收缓冲,最大USART3_MAX_RECV_LEN个字节 */

/**
 * 通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
 * 如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
 * 任何数据,则表示此次接收完毕.
 * 接收到的数据状态
 * [15]:0,没有接收到数据;1,接收到了一批数据.
 * [14:0]:接收到的数据长度
 */
volatile uint16_t g_usart3_rx_sta = 0;

/**
 * @brief       串口3中断服务函数
 * @param       无
 * @retval      无
 */
void USART3_IRQHandler(void)
{
    uint8_t res;
    OSIntEnter();
    
    if (USART3->ISR & (1 << 5))                         /* 接收到数据 */
    {
        res = USART3->RDR;
        
        if ((g_usart3_rx_sta & (1 << 15)) == 0)         /* 接收完的一批数据,还没有被处理,则不再接收其他数据 */
        { 
            if (g_usart3_rx_sta < USART3_MAX_RECV_LEN)    /* 还可以接收数据 */
            {
                TIM7->CNT = 0;                          /* 计数器清空 */
                
                if (g_usart3_rx_sta == 0)               /* 使能定时器7的中断 */
                {
                    TIM7->CR1 |= 1 << 0;                /* 使能定时器7 */
                }
                
                g_usart3_rx_buf[g_usart3_rx_sta++] = res; /* 记录接收到的值 */
            }
            else 
            {
                g_usart3_rx_sta |= 1 << 15;             /* 强制标记接收完成 */
            } 
        }
    }
    
    OSIntExit();
}

/**
 * @brief       初始化IO 串口3
 * @param       pclk1:PCLK1时钟频率(Mhz)
 * @param       bound:波特率
 * @retval      无
 */
void usart3_init(uint32_t pclk1,uint32_t bound)
{
    volatile uint32_t temp;   
    temp = (pclk1 * 1000000 + bound / 2) / bound;   /* 得到USARTDIV@OVER8=0,采用四舍五入计算 */

    RCC->AHB4ENR |= 1 << 1;             /* 使能PORTB口时钟 */
    RCC->APB1LENR |= 1 << 18;           /* 使能串口3时钟 */
    sys_gpio_set(GPIOB,SYS_GPIO_PIN10 | SYS_GPIO_PIN11,SYS_GPIO_MODE_AF,SYS_GPIO_OTYPE_PP,SYS_GPIO_SPEED_MID,SYS_GPIO_PUPD_PU); /* PB10,PB11,复用功能,上拉输出 */
    sys_gpio_af_set(GPIOB,SYS_GPIO_PIN10,7);        /* PB10,AF7 */
    sys_gpio_af_set(GPIOB,SYS_GPIO_PIN11,7);        /* PB11,AF7 */
    USART3->BRR = temp;                 /* 波特率设置 */
    USART3->CR3 |= 1 << 12;             /* 禁止OVERRUN功能 */
    USART3->CR1 = 0;                    /* 清零CR1寄存器 */
    USART3->CR1 |= 0 << 28;             /* 设置M1=0 */
    USART3->CR1 |= 0 << 12;             /* 设置M0=0&M1=0,选择8位字长 */
    USART3->CR1 |= 0 << 15;             /* 设置OVER8=0,16倍过采样 */
    USART3->CR1 |= 1 << 3;              /* 串口发送使能 */
    USART3->CR1 |= 1 << 2;              /* 串口接收使能 */
    USART3->CR1 |= 1 << 5;              /* 接收缓冲区非空中断使能 */
    sys_nvic_init(1,2,USART3_IRQn,2);   /* 组2，优先级0,2,最高优先级 */
    tim7_int_init(100 - 1,20000 - 1);   /* 10ms中断一次 */
    TIM7->CR1 &= ~(1 << 0);             /* 关闭定时器7 */
    USART3->CR1 |= 1 << 0;              /* 串口使能 */
    g_usart3_rx_sta = 0;                /* 清零 */
}

/**
 * @brief       串口3,printf 函数
 * @param       确保一次发送数据不超过USART3_MAX_SEND_LEN字节
 * @retval      无
 */
void u3_printf(char* fmt,...)  
{  
    uint16_t i, j;
    uint8_t *pbuf;
    va_list ap;
    
    va_start(ap,fmt);
    vsprintf((char*)g_usart3_tx_buf,fmt,ap);
    va_end(ap);
    i=strlen((const char*)g_usart3_tx_buf); /* 此次发送数据的长度 */
    
    for (j = 0;j < i;j++)                   /* 循环发送数据 */
    {
        while ((USART3->ISR & 0X40) == 0);  /* 循环发送,直到发送完毕 */
        
        USART3->TDR = g_usart3_tx_buf[j];  
    }
}
