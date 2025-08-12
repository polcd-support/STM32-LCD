/**
 ****************************************************************************************************
 * @file        exti.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-16
 * @brief       �ⲿ�ж� ��������
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
 * V1.0 20230316
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __EXTI_H
#define __EXTI_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/
/* ���� �� �жϱ�� & �жϷ����� ���� */ 

#define KEY0_INT_GPIO_PORT              GPIOH
#define KEY0_INT_GPIO_PIN               SYS_GPIO_PIN3
#define KEY0_INT_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 7; }while(0)   /* PH��ʱ��ʹ�� */
#define KEY0_INT_IRQn                   EXTI3_IRQn
#define KEY0_INT_IRQHandler             EXTI3_IRQHandler

#define KEY1_INT_GPIO_PORT              GPIOH
#define KEY1_INT_GPIO_PIN               SYS_GPIO_PIN2
#define KEY1_INT_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 7; }while(0)   /* PH��ʱ��ʹ�� */
#define KEY1_INT_IRQn                   EXTI2_IRQn
#define KEY1_INT_IRQHandler             EXTI2_IRQHandler

#define KEY2_INT_GPIO_PORT              GPIOC
#define KEY2_INT_GPIO_PIN               SYS_GPIO_PIN13
#define KEY2_INT_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 2; }while(0)   /* PC��ʱ��ʹ�� */
#define KEY2_INT_IRQn                   EXTI15_10_IRQn
#define KEY2_INT_IRQHandler             EXTI15_10_IRQHandler

#define WKUP_INT_GPIO_PORT              GPIOA
#define WKUP_INT_GPIO_PIN               SYS_GPIO_PIN0
#define WKUP_INT_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA��ʱ��ʹ�� */
#define WKUP_INT_IRQn                   EXTI0_IRQn
#define WKUP_INT_IRQHandler             EXTI0_IRQHandler

/******************************************************************************************/


void extix_init(void);  /* �ⲿ�жϳ�ʼ�� */

#endif

























