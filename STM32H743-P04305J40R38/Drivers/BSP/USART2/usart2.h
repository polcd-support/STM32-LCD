/**
 ****************************************************************************************************
 * @file        USART22.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       ����2 ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230324
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __USART22_H
#define __USART22_H

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* ����2 ���� ���� */
#define USART2_TX_GPIO_PORT                 GPIOA
#define USART2_TX_GPIO_PIN                  SYS_GPIO_PIN2
#define USART2_TX_GPIO_AF                   7                                           /* AF����ѡ�� */
#define USART2_TX_GPIO_CLK_ENABLE()         do{ RCC->AHB4ENR |= 1 << 0; }while(0)       /* PA��ʱ��ʹ�� */

#define USART2_RX_GPIO_PORT                 GPIOA
#define USART2_RX_GPIO_PIN                  SYS_GPIO_PIN3
#define USART2_RX_GPIO_AF                   7                                           /* AF����ѡ�� */
#define USART2_RX_GPIO_CLK_ENABLE()         do{ RCC->AHB4ENR |= 1 << 0; }while(0)       /* PA��ʱ��ʹ�� */

#define USART2_UX                           USART2
#define USART2_UX_CLK_ENABLE()              do{ RCC->APB1LENR |= 1 << 17; }while(0)     /* USART2 ʱ��ʹ�� */

/******************************************************************************************/



void usart2_init(uint32_t sclk, uint32_t baudrate); /* ����2��ʼ�� */

#endif































