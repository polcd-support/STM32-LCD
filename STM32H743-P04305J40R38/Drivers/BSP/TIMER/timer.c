/**
 ****************************************************************************************************
 * @file        timer.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-25
 * @brief       通用定时器(服务综合测试实验) 驱动代码
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
 * V1.0 20221125
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/TIMER/timer.h"
#include "./BSP/USART3/usart3.h"
#include "./BSP/LED/led.h"
#include "ucos_ii.h"
#include "os.h"

volatile uint8_t g_framecnt;    /* 帧计数器 */
volatile uint8_t g_framecntout; /* 统一的帧计数器输出变量 */

extern void usbapp_pulling(void);

/**
 * @brief       定时器8中断服务程序
 * @param       无
 * @retval      无
 */
void TIM8_UP_TIM13_IRQHandler(void)
{
    OSIntEnter();

    if (TIM8->SR & 0X0001)        /* 是更新中断 */
    {
        
        if (OSRunning != OS_TRUE)  /* OS还没运行,借TIM3的中断,10ms一次,来扫描USB */
        {
            usbapp_pulling();
        }
        else
        {
            g_framecntout = g_framecnt;
            printf("frame:%d\r\n", g_framecnt); /* 打印帧率 */
            g_framecnt = 0;
        }
    }

    TIM8->SR &= ~(1 << 0);      /* 清除中断标志位 */
    
    OSIntExit();
}

/**
 * @brief       基本定时器8中断初始化
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void tim8_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB2ENR |= 1 << 1;     /* TIM8时钟使能 */
    while ((RCC->APB2ENR & (1 << 1)) == 0); /* 等待时钟设置OK */
    TIM8->ARR = arr;            /* 设定计数器自动重装值 */
    TIM8->PSC = psc;            /* 预分频器 */
    TIM8->DIER |= 1 << 0;       /* 允许更新中断 */
    TIM8->CR1 |= 0x01;          /* 使能定时器 */
    
    sys_nvic_init(1, 3, TIM8_UP_TIM13_IRQn, 2);  /* 抢占1，子优先级3，组2 */
}

/* 视频播放帧率控制全局变量 */
extern volatile uint8_t g_avi_frameup;  /* 视频播放时隙控制变量,当等于1的时候,可以更新下一帧视频 */
extern volatile uint8_t webcam_oensec;
extern uint16_t g_reg_time;
/**
 * @brief       定时器6中断服务程序
 * @param       无
 * @retval      无
 */
void TIM6_DAC_IRQHandler(void)
{
    OSIntEnter();

    if (TIM6->SR & 0X01)    /* 是更新中断 */
    {
        g_avi_frameup = 1;  /* 标记 */
        webcam_oensec = 1;
        g_reg_time ++;
    }

    TIM6->SR &= ~(1 << 0);  /* 清除中断标志位 */
    OSIntExit();
}

/**
 * @brief       基本定时器6中断初始化
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void tim6_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 4;     /* TIM6时钟使能 */
    
    while ((RCC->APB1LENR & (1 << 4)) == 0); /* 等待时钟设置OK */
    
    TIM6->ARR = arr;            /* 设定计数器自动重装值 */
    TIM6->PSC = psc;            /* 预分频器 */
    TIM6->CNT = 0;              /* 计数器清零 */
    TIM6->DIER |= 1 << 0;       /* 允许更新中断 */
    TIM6->CR1 |= 0x01;          /* 使能定时器 */
    
    sys_nvic_init(3, 0, TIM6_DAC_IRQn, 2);  /* 抢占3，子优先级0，组2 */
}

/**
 * @brief       定时器7中断服务程序
 * @param       无
 * @retval      无
 */
void TIM7_IRQHandler(void)
{
    OSIntEnter();

    if (TIM7->SR & 0X01) /* 是更新中断 */
    {
        g_usart3_rx_sta |= 1 << 15; /* 标记接收完成 */
        TIM7->SR &= ~(1 << 0);      /* 清除中断标志位 */
        TIM7->CR1 &= ~(1 << 0);     /* 关闭定时器7 */
    }

    OSIntExit();
}

/**
 * @brief       基本定时器7中断初始化
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void tim7_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 5;    /* TIM7时钟使能 */
    
    while ((RCC->APB1LENR & (1 << 5)) == 0); /* 等待时钟设置OK */
    
    TIM7->ARR = arr;            /* 设定计数器自动重装值 */
    TIM7->PSC = psc;            /* 预分频器 */
    TIM7->CNT = 0;              /* 计数器清零 */
    TIM7->DIER |= 1 << 0;       /* 允许更新中断 */
    TIM7->CR1 |= 0x01;          /* 使能定时器 */
    
    sys_nvic_init(1, 0, TIM7_IRQn, 2);  /* 抢占1，子优先级0，组2 */
}


/**
 * @brief       定时器3 CH2 PWM输出 初始化函数（使用PWM模式1）
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void tim3_ch2_pwm_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 1;    /* TIM3时钟使能 */
    RCC->AHB4ENR |= 1 << 1;     /* 使能PORTB时钟 */

    sys_gpio_set(GPIOB, SYS_GPIO_PIN5,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);    /* PB15 引脚模式设置 */

    sys_gpio_af_set(GPIOB, SYS_GPIO_PIN5, 2);      /* PB5,AF2 */

    TIM3->ARR = arr;        /* 设定计数器自动重装值 */
    TIM3->PSC = psc;        /* 设置预分频器  */
 
    TIM3->CCMR1 |= 6 << 12; /* CH2 PWM1模式 */
    TIM3->CCMR1 |= 1 << 11; /* CH2 预装载使能 */
    
    TIM3->CCER |= 1 << 4;   /* OC1 输出使能 */
    TIM3->CCER |= 0 << 5;   /* OC1 高电平有效 */
    TIM3->CR1 |= 1 << 7;    /* ARPE使能 */
    TIM3->CR1 |= 1 << 0;    /* 使能定时器TIMX */
}
