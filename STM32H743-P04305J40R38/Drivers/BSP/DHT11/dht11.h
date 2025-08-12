/**
 ****************************************************************************************************
 * @file        dht11.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-23
 * @brief       DHT11������ʪ�ȴ����� ��������
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
 * V1.0 20230323
 * ��һ�η���
 *
 ****************************************************************************************************
 */
 
#ifndef __DHT11_H
#define __DHT11_H 

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* DHT11 ���� ���� */

#define DHT11_DQ_GPIO_PORT                  GPIOB
#define DHT11_DQ_GPIO_PIN                   SYS_GPIO_PIN12
#define DHT11_DQ_GPIO_CLK_ENABLE()          do{ RCC->AHB4ENR |= 1 << 1; }while(0)   /* PB��ʱ��ʹ�� */

/******************************************************************************************/

/* IO�������� */
#define DHT11_DQ_OUT(x)         sys_gpio_pin_set(DHT11_DQ_GPIO_PORT, DHT11_DQ_GPIO_PIN, x)  /* ���ݶ˿���� */
#define DHT11_DQ_IN             sys_gpio_pin_get(DHT11_DQ_GPIO_PORT, DHT11_DQ_GPIO_PIN)     /* ���ݶ˿����� */


uint8_t dht11_init(void);   /* ��ʼ��DHT11 */
uint8_t dht11_check(void);  /* ����Ƿ����DHT11 */
uint8_t dht11_read_data(uint8_t *temp,uint8_t *humi);   /* ��ȡ��ʪ�� */

#endif















