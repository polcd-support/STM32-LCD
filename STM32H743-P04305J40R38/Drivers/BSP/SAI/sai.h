/**
 ****************************************************************************************************
 * @file        sai.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2023-03-25
 * @brief       SAI ��������
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
 * V1.0 20230325
 * ��һ�η���
 * V1.1 20230325
 * ���sai1_saib_init/sai1_rx_dma_init/sai1_rec_start/sai1_rec_stop�Ⱥ���,֧��¼��
 *
 ****************************************************************************************************
 */

#ifndef __SAI_H
#define __SAI_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* SAI1 ���� ���� */

#define SAI1_MCLK_GPIO_PORT             GPIOE
#define SAI1_MCLK_GPIO_PIN              SYS_GPIO_PIN2
#define SAI1_MCLK_GPIO_AF               6                                           /* AF����ѡ�� */
#define SAI1_MCLK_GPIO_CLK_ENABLE()     do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE��ʱ��ʹ�� */

#define SAI1_SCK_GPIO_PORT              GPIOE
#define SAI1_SCK_GPIO_PIN               SYS_GPIO_PIN5
#define SAI1_SCK_GPIO_AF                6                                           /* AF����ѡ�� */
#define SAI1_SCK_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE��ʱ��ʹ�� */

#define SAI1_FS_GPIO_PORT               GPIOE
#define SAI1_FS_GPIO_PIN                SYS_GPIO_PIN4
#define SAI1_FS_GPIO_AF                 6                                           /* AF����ѡ�� */
#define SAI1_FS_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE��ʱ��ʹ�� */

#define SAI1_SDA_GPIO_PORT              GPIOE
#define SAI1_SDA_GPIO_PIN               SYS_GPIO_PIN6
#define SAI1_SDA_GPIO_AF                6                                           /* AF����ѡ�� */
#define SAI1_SDA_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE��ʱ��ʹ�� */

#define SAI1_SDB_GPIO_PORT              GPIOE
#define SAI1_SDB_GPIO_PIN               SYS_GPIO_PIN3
#define SAI1_SDB_GPIO_AF                6                                           /* AF����ѡ�� */
#define SAI1_SDB_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE��ʱ��ʹ�� */

/* SAI1 ��ض��� */
#define SAI1_SAIA                       SAI1_Block_A
#define SAI1_SAIB                       SAI1_Block_B
#define SAI1_SAI_CLK_ENABLE()           do{ RCC->APB2ENR |= 1 << 22; }while(0)      /* SAI1��ʹ�� */


/* SAI1 DMA��ض��� */
#define SAI1_TX_DMASx                   DMA2_Stream3
#define SAI1_TX_DMASx_Channel           87
#define SAI1_TX_DMASx_IRQHandler        DMA2_Stream3_IRQHandler
#define SAI1_TX_DMASx_IRQn              DMA2_Stream3_IRQn
#define SAI1_TX_DMA_CLK_ENABLE()        do{ RCC->AHB1ENR |= 1 << 1; }while(0)       /* DMA2 ��ʹ�� */

#define SAI1_TX_DMASx_IS_TC()           ( DMA2->LISR & (1 << 27) )                  /* �ж� DMA2_Stream3 ������ɱ�־, ����һ���ٺ�����ʽ,
                                                                                     * ���ܵ�����ʹ��, ֻ������if��������� 
                                                                                     */
#define SAI1_TX_DMASx_CLR_TC()          do{ DMA2->LIFCR |= 1 << 27; }while(0)       /* ��� DMA2_Stream3 ������ɱ�־ */

/* SAI1 RX DMA��ض��� */
#define SAI1_RX_DMASx                   DMA2_Stream5
#define SAI1_RX_DMASx_Channel           88
#define SAI1_RX_DMASx_IRQHandler        DMA2_Stream5_IRQHandler
#define SAI1_RX_DMASx_IRQn              DMA2_Stream5_IRQn
#define SAI1_RX_DMA_CLK_ENABLE()        do{ RCC->AHB1ENR |= 1 << 1; }while(0)       /* DMA2 ��ʹ�� */

#define SAI1_RX_DMASx_IS_TC()           ( DMA2->HISR & (1 << 11) )                  /* �ж� DMA2_Stream5 ������ɱ�־, ����һ���ٺ�����ʽ,
                                                                                     * ���ܵ�����ʹ��, ֻ������if��������� 
                                                                                     */
#define SAI1_RX_DMASx_CLR_TC()          do{ DMA2->HIFCR |= 1 << 11; }while(0)       /* ��� DMA2_Stream5 ������ɱ�־ */

/******************************************************************************************/

extern void (*sai1_tx_callback)(void);          /* sai1 tx�ص�����ָ�� */
extern void (*sai1_rx_callback)(void);          /* sai1 rx�ص�����ָ�� */

void sai1_saia_init(uint8_t mode, uint8_t cpol, uint8_t datalen);
void sai1_saib_init(uint8_t mode, uint8_t cpol, uint8_t datalen);
uint8_t sai1_samplerate_set(uint32_t samplerate);
void sai1_tx_dma_init(uint8_t* buf0,uint8_t *buf1,uint16_t num,uint8_t width);
void sai1_rx_dma_init(uint8_t* buf0,uint8_t *buf1,uint16_t num,uint8_t width);
void sai1_play_start(void); 
void sai1_play_stop(void);
void sai1_rec_start(void);
void sai1_rec_stop(void);

#endif





















