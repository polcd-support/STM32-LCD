/**
 ****************************************************************************************************
 * @file        usart3.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-10-29
 * @brief       ����3 ��������
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
 * V1.0 20221029
 * ��һ�η���
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

/* ���� */
__align(8) uint8_t g_usart3_tx_buf[USART3_MAX_SEND_LEN];         /* ���ջ���,���USART3_MAX_RECV_LEN���ֽ� */
/* ���ڽ��ջ����� */
uint8_t g_usart3_rx_buf[USART3_MAX_RECV_LEN];         /* ���ջ���,���USART3_MAX_RECV_LEN���ֽ� */

/**
 * ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
 * ���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
 * �κ�����,���ʾ�˴ν������.
 * ���յ�������״̬
 * [15]:0,û�н��յ�����;1,���յ���һ������.
 * [14:0]:���յ������ݳ���
 */
volatile uint16_t g_usart3_rx_sta = 0;

/**
 * @brief       ����3�жϷ�����
 * @param       ��
 * @retval      ��
 */
void USART3_IRQHandler(void)
{
    uint8_t res;
    OSIntEnter();
    
    if (USART3->ISR & (1 << 5))                         /* ���յ����� */
    {
        res = USART3->RDR;
        
        if ((g_usart3_rx_sta & (1 << 15)) == 0)         /* �������һ������,��û�б�����,���ٽ����������� */
        { 
            if (g_usart3_rx_sta < USART3_MAX_RECV_LEN)    /* �����Խ������� */
            {
                TIM7->CNT = 0;                          /* ��������� */
                
                if (g_usart3_rx_sta == 0)               /* ʹ�ܶ�ʱ��7���ж� */
                {
                    TIM7->CR1 |= 1 << 0;                /* ʹ�ܶ�ʱ��7 */
                }
                
                g_usart3_rx_buf[g_usart3_rx_sta++] = res; /* ��¼���յ���ֵ */
            }
            else 
            {
                g_usart3_rx_sta |= 1 << 15;             /* ǿ�Ʊ�ǽ������ */
            } 
        }
    }
    
    OSIntExit();
}

/**
 * @brief       ��ʼ��IO ����3
 * @param       pclk1:PCLK1ʱ��Ƶ��(Mhz)
 * @param       bound:������
 * @retval      ��
 */
void usart3_init(uint32_t pclk1,uint32_t bound)
{
    volatile uint32_t temp;   
    temp = (pclk1 * 1000000 + bound / 2) / bound;   /* �õ�USARTDIV@OVER8=0,��������������� */

    RCC->AHB4ENR |= 1 << 1;             /* ʹ��PORTB��ʱ�� */
    RCC->APB1LENR |= 1 << 18;           /* ʹ�ܴ���3ʱ�� */
    sys_gpio_set(GPIOB,SYS_GPIO_PIN10 | SYS_GPIO_PIN11,SYS_GPIO_MODE_AF,SYS_GPIO_OTYPE_PP,SYS_GPIO_SPEED_MID,SYS_GPIO_PUPD_PU); /* PB10,PB11,���ù���,������� */
    sys_gpio_af_set(GPIOB,SYS_GPIO_PIN10,7);        /* PB10,AF7 */
    sys_gpio_af_set(GPIOB,SYS_GPIO_PIN11,7);        /* PB11,AF7 */
    USART3->BRR = temp;                 /* ���������� */
    USART3->CR3 |= 1 << 12;             /* ��ֹOVERRUN���� */
    USART3->CR1 = 0;                    /* ����CR1�Ĵ��� */
    USART3->CR1 |= 0 << 28;             /* ����M1=0 */
    USART3->CR1 |= 0 << 12;             /* ����M0=0&M1=0,ѡ��8λ�ֳ� */
    USART3->CR1 |= 0 << 15;             /* ����OVER8=0,16�������� */
    USART3->CR1 |= 1 << 3;              /* ���ڷ���ʹ�� */
    USART3->CR1 |= 1 << 2;              /* ���ڽ���ʹ�� */
    USART3->CR1 |= 1 << 5;              /* ���ջ������ǿ��ж�ʹ�� */
    sys_nvic_init(1,2,USART3_IRQn,2);   /* ��2�����ȼ�0,2,������ȼ� */
    tim7_int_init(100 - 1,20000 - 1);   /* 10ms�ж�һ�� */
    TIM7->CR1 &= ~(1 << 0);             /* �رն�ʱ��7 */
    USART3->CR1 |= 1 << 0;              /* ����ʹ�� */
    g_usart3_rx_sta = 0;                /* ���� */
}

/**
 * @brief       ����3,printf ����
 * @param       ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
 * @retval      ��
 */
void u3_printf(char* fmt,...)  
{  
    uint16_t i, j;
    uint8_t *pbuf;
    va_list ap;
    
    va_start(ap,fmt);
    vsprintf((char*)g_usart3_tx_buf,fmt,ap);
    va_end(ap);
    i=strlen((const char*)g_usart3_tx_buf); /* �˴η������ݵĳ��� */
    
    for (j = 0;j < i;j++)                   /* ѭ���������� */
    {
        while ((USART3->ISR & 0X40) == 0);  /* ѭ������,ֱ��������� */
        
        USART3->TDR = g_usart3_tx_buf[j];  
    }
}
