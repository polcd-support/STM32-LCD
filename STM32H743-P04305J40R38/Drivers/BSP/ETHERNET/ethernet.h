/**
 ****************************************************************************************************
 * @file        ethernet.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-08-01
 * @brief       ETHERNET ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20211202
 * ��һ�η���
 *
 ****************************************************************************************************
 */
 
#ifndef __ETHERNET_H
#define __ETHERNET_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/ETHERNET/stm32h7xx_hal_eth.h"

/******************************************************************************************/
/* ���� ���� */

#define ETH_CLK_GPIO_PORT               GPIOA
#define ETH_CLK_GPIO_PIN                GPIO_PIN_1
#define ETH_CLK_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 0;}while(0)                    /* ����IO��ʱ��ʹ�� */

#define ETH_MDIO_GPIO_PORT              GPIOA
#define ETH_MDIO_GPIO_PIN               GPIO_PIN_2
#define ETH_MDIO_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 0;}while(0)                   /* ����IO��ʱ��ʹ�� */

#define ETH_CRS_GPIO_PORT               GPIOA
#define ETH_CRS_GPIO_PIN                GPIO_PIN_7
#define ETH_CRS_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 0;}while(0)                    /* ����IO��ʱ��ʹ�� */

#define ETH_MDC_GPIO_PORT               GPIOC
#define ETH_MDC_GPIO_PIN                GPIO_PIN_1
#define ETH_MDC_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 2;}while(0)                    /* ����IO��ʱ��ʹ�� */

#define ETH_RXD0_GPIO_PORT              GPIOC
#define ETH_RXD0_GPIO_PIN               GPIO_PIN_4
#define ETH_RXD0_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 2;}while(0)                    /* ����IO��ʱ��ʹ�� */

#define ETH_RXD1_GPIO_PORT              GPIOC
#define ETH_RXD1_GPIO_PIN               GPIO_PIN_5
#define ETH_RXD1_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 2;}while(0)                    /* ����IO��ʱ��ʹ�� */

#define ETH_TX_EN_GPIO_PORT             GPIOB
#define ETH_TX_EN_GPIO_PIN              GPIO_PIN_11
#define ETH_TX_EN_GPIO_CLK_ENABLE()     do{ RCC->AHB4ENR |= 1 << 1;;}while(0)                   /* ����IO��ʱ��ʹ�� */

#define ETH_TXD0_GPIO_PORT              GPIOG
#define ETH_TXD0_GPIO_PIN               GPIO_PIN_13
#define ETH_TXD0_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 6;}while(0)                    /* ����IO��ʱ��ʹ�� */

#define ETH_TXD1_GPIO_PORT              GPIOG
#define ETH_TXD1_GPIO_PIN               GPIO_PIN_14
#define ETH_TXD1_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 6;}while(0)                    /* ����IO��ʱ��ʹ�� */


/******************************************************************************************/

extern ETH_HandleTypeDef    g_eth_handler;                                      /* ��̫����� */
extern __attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT];                  /* ��̫��Rx DMA������,96�ֽ�,�������ڴ汣��,��ֹCACHE */
extern __attribute__((at(0x30040060))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT];                  /* ��̫��Tx DMA������,96�ֽ�,�������ڴ汣��,��ֹCACHE */
extern __attribute__((at(0x30040200))) uint8_t ETH_Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE];          /* ��̫�����ջ����� */

uint8_t     ethernet_init(void);                                                /* ��̫��оƬ��ʼ�� */
uint32_t    ethernet_read_phy(uint16_t reg);                                    /* ��ȡ��̫��оƬ�Ĵ���ֵ */
void        ethernet_write_phy(uint16_t reg, uint16_t value);                   /* ����̫��оƬָ����ַд��Ĵ���ֵ */
uint8_t     ethernet_chip_get_speed(void);                                      /* �����̫��оƬ���ٶ�ģʽ */
void        NETMPU_Config(void);
#endif

