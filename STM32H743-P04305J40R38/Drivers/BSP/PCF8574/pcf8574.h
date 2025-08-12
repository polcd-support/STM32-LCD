/**
 ****************************************************************************************************
 * @file        pcf8574.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       PCF8574 ��������
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
 * V1.0 20230322
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __PCF8574_H
#define __PCF8574_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/IIC/myiic.h"


/******************************************************************************************/
/* ���� ���� */

#define PCF8574_INT_GPIO_PORT               GPIOB
#define PCF8574_INT_GPIO_PIN                SYS_GPIO_PIN12
#define PCF8574_INT_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 1; }while(0)       /* IO��ʱ��ʹ�� */

/******************************************************************************************/

#define PCF8574_INT     sys_gpio_pin_get(PCF8574_INT_GPIO_PORT, PCF8574_INT_GPIO_PIN)   /* PCF8574 INT�� */

#define PCF8574_ADDR    0X40    /* PCF8574��ַ(������һλ) */

/* PCF8574����IO�Ĺ��� */
#define BEEP_IO         0       /* ��������������        P0 */
#define AP_INT_IO       1       /* AP3216C�ж�����       P1 */
#define DCMI_PWDN_IO    2       /* DCMI�ĵ�Դ��������    P2 */
#define USB_PWR_IO      3       /* USB��Դ��������       P3 */
#define EX_IO           4       /* ��չIO,�Զ���ʹ��     P4 */
#define MPU_INT_IO      5       /* MPU9250�ж�����       P5 */
#define RS485_RE_IO     6       /* RS485_RE����          P6 */
#define ETH_RESET_IO    7       /* ��̫����λ����         P7 */


uint8_t pcf8574_init(void); 
uint8_t pcf8574_read_byte(void);
void pcf8574_write_byte(uint8_t data);
void pcf8574_write_bit(uint8_t bit, uint8_t sta);
uint8_t pcf8574_read_bit(uint8_t bit);

#endif

