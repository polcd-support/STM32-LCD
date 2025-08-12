/**
 ****************************************************************************************************
 * @file        timer.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-25
 * @brief       ͨ�ö�ʱ��(�����ۺϲ���ʵ��) ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F103������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221125
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/TIMER/timer.h"
#include "./BSP/USART3/usart3.h"
#include "./BSP/LED/led.h"
#include "ucos_ii.h"
#include "os.h"

volatile uint8_t g_framecnt;    /* ֡������ */
volatile uint8_t g_framecntout; /* ͳһ��֡������������� */

extern void usbapp_pulling(void);

/**
 * @brief       ��ʱ��8�жϷ������
 * @param       ��
 * @retval      ��
 */
void TIM8_UP_TIM13_IRQHandler(void)
{
    OSIntEnter();

    if (TIM8->SR & 0X0001)        /* �Ǹ����ж� */
    {
        
        if (OSRunning != OS_TRUE)  /* OS��û����,��TIM3���ж�,10msһ��,��ɨ��USB */
        {
            usbapp_pulling();
        }
        else
        {
            g_framecntout = g_framecnt;
            printf("frame:%d\r\n", g_framecnt); /* ��ӡ֡�� */
            g_framecnt = 0;
        }
    }

    TIM8->SR &= ~(1 << 0);      /* ����жϱ�־λ */
    
    OSIntExit();
}

/**
 * @brief       ������ʱ��8�жϳ�ʼ��
 * @param       arr: �Զ���װֵ
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void tim8_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB2ENR |= 1 << 1;     /* TIM8ʱ��ʹ�� */
    while ((RCC->APB2ENR & (1 << 1)) == 0); /* �ȴ�ʱ������OK */
    TIM8->ARR = arr;            /* �趨�������Զ���װֵ */
    TIM8->PSC = psc;            /* Ԥ��Ƶ�� */
    TIM8->DIER |= 1 << 0;       /* ��������ж� */
    TIM8->CR1 |= 0x01;          /* ʹ�ܶ�ʱ�� */
    
    sys_nvic_init(1, 3, TIM8_UP_TIM13_IRQn, 2);  /* ��ռ1�������ȼ�3����2 */
}

/* ��Ƶ����֡�ʿ���ȫ�ֱ��� */
extern volatile uint8_t g_avi_frameup;  /* ��Ƶ����ʱ϶���Ʊ���,������1��ʱ��,���Ը�����һ֡��Ƶ */
extern volatile uint8_t webcam_oensec;
extern uint16_t g_reg_time;
/**
 * @brief       ��ʱ��6�жϷ������
 * @param       ��
 * @retval      ��
 */
void TIM6_DAC_IRQHandler(void)
{
    OSIntEnter();

    if (TIM6->SR & 0X01)    /* �Ǹ����ж� */
    {
        g_avi_frameup = 1;  /* ��� */
        webcam_oensec = 1;
        g_reg_time ++;
    }

    TIM6->SR &= ~(1 << 0);  /* ����жϱ�־λ */
    OSIntExit();
}

/**
 * @brief       ������ʱ��6�жϳ�ʼ��
 * @param       arr: �Զ���װֵ
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void tim6_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 4;     /* TIM6ʱ��ʹ�� */
    
    while ((RCC->APB1LENR & (1 << 4)) == 0); /* �ȴ�ʱ������OK */
    
    TIM6->ARR = arr;            /* �趨�������Զ���װֵ */
    TIM6->PSC = psc;            /* Ԥ��Ƶ�� */
    TIM6->CNT = 0;              /* ���������� */
    TIM6->DIER |= 1 << 0;       /* ��������ж� */
    TIM6->CR1 |= 0x01;          /* ʹ�ܶ�ʱ�� */
    
    sys_nvic_init(3, 0, TIM6_DAC_IRQn, 2);  /* ��ռ3�������ȼ�0����2 */
}

/**
 * @brief       ��ʱ��7�жϷ������
 * @param       ��
 * @retval      ��
 */
void TIM7_IRQHandler(void)
{
    OSIntEnter();

    if (TIM7->SR & 0X01) /* �Ǹ����ж� */
    {
        g_usart3_rx_sta |= 1 << 15; /* ��ǽ������ */
        TIM7->SR &= ~(1 << 0);      /* ����жϱ�־λ */
        TIM7->CR1 &= ~(1 << 0);     /* �رն�ʱ��7 */
    }

    OSIntExit();
}

/**
 * @brief       ������ʱ��7�жϳ�ʼ��
 * @param       arr: �Զ���װֵ
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void tim7_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 5;    /* TIM7ʱ��ʹ�� */
    
    while ((RCC->APB1LENR & (1 << 5)) == 0); /* �ȴ�ʱ������OK */
    
    TIM7->ARR = arr;            /* �趨�������Զ���װֵ */
    TIM7->PSC = psc;            /* Ԥ��Ƶ�� */
    TIM7->CNT = 0;              /* ���������� */
    TIM7->DIER |= 1 << 0;       /* ��������ж� */
    TIM7->CR1 |= 0x01;          /* ʹ�ܶ�ʱ�� */
    
    sys_nvic_init(1, 0, TIM7_IRQn, 2);  /* ��ռ1�������ȼ�0����2 */
}


/**
 * @brief       ��ʱ��3 CH2 PWM��� ��ʼ��������ʹ��PWMģʽ1��
 * @param       arr: �Զ���װֵ
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void tim3_ch2_pwm_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 1;    /* TIM3ʱ��ʹ�� */
    RCC->AHB4ENR |= 1 << 1;     /* ʹ��PORTBʱ�� */

    sys_gpio_set(GPIOB, SYS_GPIO_PIN5,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);    /* PB15 ����ģʽ���� */

    sys_gpio_af_set(GPIOB, SYS_GPIO_PIN5, 2);      /* PB5,AF2 */

    TIM3->ARR = arr;        /* �趨�������Զ���װֵ */
    TIM3->PSC = psc;        /* ����Ԥ��Ƶ��  */
 
    TIM3->CCMR1 |= 6 << 12; /* CH2 PWM1ģʽ */
    TIM3->CCMR1 |= 1 << 11; /* CH2 Ԥװ��ʹ�� */
    
    TIM3->CCER |= 1 << 4;   /* OC1 ���ʹ�� */
    TIM3->CCER |= 0 << 5;   /* OC1 �ߵ�ƽ��Ч */
    TIM3->CR1 |= 1 << 7;    /* ARPEʹ�� */
    TIM3->CR1 |= 1 << 0;    /* ʹ�ܶ�ʱ��TIMX */
}
