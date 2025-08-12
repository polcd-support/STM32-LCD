/**
 ****************************************************************************************************
 * @file        qspi.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       QSPI ��������
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

#ifndef __QSPI_H
#define __QSPI_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* QSPI ��� ���� ���� */

#define QSPI_BK1_CLK_GPIO_PORT          GPIOB
#define QSPI_BK1_CLK_GPIO_PIN           SYS_GPIO_PIN2
#define QSPI_BK1_CLK_GPIO_AF            9
#define QSPI_BK1_CLK_GPIO_CLK_ENABLE()  do{ RCC->AHB4ENR |= 1 << 1; }while(0)   /* PB��ʱ��ʹ�� */

#define QSPI_BK1_NCS_GPIO_PORT          GPIOB
#define QSPI_BK1_NCS_GPIO_PIN           SYS_GPIO_PIN6
#define QSPI_BK1_NCS_GPIO_AF            10
#define QSPI_BK1_NCS_GPIO_CLK_ENABLE()  do{ RCC->AHB4ENR |= 1 << 1; }while(0)   /* PB��ʱ��ʹ�� */

#define QSPI_BK1_IO0_GPIO_PORT          GPIOF
#define QSPI_BK1_IO0_GPIO_PIN           SYS_GPIO_PIN8
#define QSPI_BK1_IO0_GPIO_AF            10
#define QSPI_BK1_IO0_GPIO_CLK_ENABLE()  do{ RCC->AHB4ENR |= 1 << 5; }while(0)   /* PF��ʱ��ʹ�� */

#define QSPI_BK1_IO1_GPIO_PORT          GPIOF
#define QSPI_BK1_IO1_GPIO_PIN           SYS_GPIO_PIN9
#define QSPI_BK1_IO1_GPIO_AF            10
#define QSPI_BK1_IO1_GPIO_CLK_ENABLE()  do{ RCC->AHB4ENR |= 1 << 5; }while(0)   /* PF��ʱ��ʹ�� */

#define QSPI_BK1_IO2_GPIO_PORT          GPIOF
#define QSPI_BK1_IO2_GPIO_PIN           SYS_GPIO_PIN6
#define QSPI_BK1_IO2_GPIO_AF            9
#define QSPI_BK1_IO2_GPIO_CLK_ENABLE()  do{ RCC->AHB4ENR |= 1 << 5; }while(0)   /* PF��ʱ��ʹ�� */

#define QSPI_BK1_IO3_GPIO_PORT          GPIOF
#define QSPI_BK1_IO3_GPIO_PIN           SYS_GPIO_PIN7
#define QSPI_BK1_IO3_GPIO_AF            9
#define QSPI_BK1_IO3_GPIO_CLK_ENABLE()  do{ RCC->AHB4ENR |= 1 << 5; }while(0)   /* PF��ʱ��ʹ�� */

/******************************************************************************************/


uint8_t qspi_wait_flag(uint32_t flag, uint8_t sta, uint32_t wtime); /* QSPI�ȴ�ĳ��״̬ */
uint8_t qspi_init(void);    /* ��ʼ��QSPI */
void qspi_send_cmd(uint8_t cmd, uint32_t addr, uint8_t mode, uint8_t dmcycle);  /* QSPI�������� */
uint8_t qspi_receive(uint8_t *buf, uint32_t datalen);   /* QSPI�������� */
uint8_t qspi_transmit(uint8_t *buf, uint32_t datalen);  /* QSPI�������� */

#endif



















