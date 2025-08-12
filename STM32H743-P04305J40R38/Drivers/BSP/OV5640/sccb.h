/**
 ****************************************************************************************************
 * @file        sccb.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       SCCB ��������
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

#ifndef __SCCB_H
#define __SCCB_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* ���� ���� */

#define SCCB_SCL_GPIO_PORT               GPIOB
#define SCCB_SCL_GPIO_PIN                SYS_GPIO_PIN4
#define SCCB_SCL_GPIO_CLK_ENABLE()       do{ RCC->AHB1ENR |= 1 << 1; }while(0)   /* PB��ʱ��ʹ�� */

#define SCCB_SDA_GPIO_PORT               GPIOB
#define SCCB_SDA_GPIO_PIN                SYS_GPIO_PIN3
#define SCCB_SDA_GPIO_CLK_ENABLE()       do{ RCC->AHB1ENR |= 1 << 1; }while(0)   /* PB��ʱ��ʹ�� */

/******************************************************************************************/

/* IO�������� */
#define SCCB_SCL(x)         sys_gpio_pin_set(SCCB_SCL_GPIO_PORT, SCCB_SCL_GPIO_PIN, x)  /* SCL */
#define SCCB_SDA(x)         sys_gpio_pin_set(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN, x)  /* SDA */
#define SCCB_READ_SDA       sys_gpio_pin_get(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN)     /* ��ȡSDA */


/* ����ӿں��� */
void sccb_init(void);
void sccb_stop(void);
void sccb_start(void);

void sccb_nack(void);
uint8_t sccb_read_byte(void);
uint8_t sccb_send_byte(uint8_t data);

#endif













