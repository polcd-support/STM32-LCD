/**
 ****************************************************************************************************
 * @file        spdif.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-25
 * @brief       SPDIF ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230325
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __SPDIF_H
#define __SPDIF_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* SPDIF RX ���� ���� */

#define SPDIF_RX_GPIO_PORT              GPIOG
#define SPDIF_RX_GPIO_PIN               SYS_GPIO_PIN12
#define SPDIF_RX_GPIO_AF                8                                           /* AF����ѡ�� */
#define SPDIF_RX_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 6; }while(0)       /* PG��ʱ��ʹ�� */

/******************************************************************************************/


/* SPDIF RX������ģʽ���� */
#define SPDIF_RX_IDLE       0       /* IDLEģʽ */
#define SPDIF_RX_SYNC       1       /* ͬ��ģʽ */
#define SPDIF_RX_RCV        3       /* ��������ģʽ */


/* SPDIF���ƽṹ�� */
typedef struct
{
    uint8_t consta;                 /* ����״̬,0,δ����;1,�������� */
    uint32_t samplerate;            /* SPDIF������ */
    uint32_t clock;                 /* SPDIFʱ��Ƶ�� */
}spdif_rx_dev;

extern spdif_rx_dev spdif_dev;
extern void (*spdif_rx_stop_callback)(void);    /* SPDIF RXֹͣʱ�Ļص����� */


void spdif_rx_init(void);
void spdif_rx_mode(uint8_t mode);
void spdif_rx_dma_init(uint32_t *buf0, uint32_t *buf1, uint16_t num, uint8_t width);
uint8_t spdif_rx_wait_sync(void);
uint32_t spdif_rx_get_samplerate(void);
void spdif_rx_start(void);
void spdif_rx_stop(void);

#endif



















