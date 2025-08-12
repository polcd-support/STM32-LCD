/**
 ****************************************************************************************************
 * @file        sai.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2023-03-25
 * @brief       SAI 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230325
 * 第一次发布
 * V1.1 20230325
 * 添加sai1_saib_init/sai1_rx_dma_init/sai1_rec_start/sai1_rec_stop等函数,支持录音
 *
 ****************************************************************************************************
 */

#ifndef __SAI_H
#define __SAI_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* SAI1 引脚 定义 */

#define SAI1_MCLK_GPIO_PORT             GPIOE
#define SAI1_MCLK_GPIO_PIN              SYS_GPIO_PIN2
#define SAI1_MCLK_GPIO_AF               6                                           /* AF功能选择 */
#define SAI1_MCLK_GPIO_CLK_ENABLE()     do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE口时钟使能 */

#define SAI1_SCK_GPIO_PORT              GPIOE
#define SAI1_SCK_GPIO_PIN               SYS_GPIO_PIN5
#define SAI1_SCK_GPIO_AF                6                                           /* AF功能选择 */
#define SAI1_SCK_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE口时钟使能 */

#define SAI1_FS_GPIO_PORT               GPIOE
#define SAI1_FS_GPIO_PIN                SYS_GPIO_PIN4
#define SAI1_FS_GPIO_AF                 6                                           /* AF功能选择 */
#define SAI1_FS_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE口时钟使能 */

#define SAI1_SDA_GPIO_PORT              GPIOE
#define SAI1_SDA_GPIO_PIN               SYS_GPIO_PIN6
#define SAI1_SDA_GPIO_AF                6                                           /* AF功能选择 */
#define SAI1_SDA_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE口时钟使能 */

#define SAI1_SDB_GPIO_PORT              GPIOE
#define SAI1_SDB_GPIO_PIN               SYS_GPIO_PIN3
#define SAI1_SDB_GPIO_AF                6                                           /* AF功能选择 */
#define SAI1_SDB_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 4; }while(0)       /* PE口时钟使能 */

/* SAI1 相关定义 */
#define SAI1_SAIA                       SAI1_Block_A
#define SAI1_SAIB                       SAI1_Block_B
#define SAI1_SAI_CLK_ENABLE()           do{ RCC->APB2ENR |= 1 << 22; }while(0)      /* SAI1钟使能 */


/* SAI1 DMA相关定义 */
#define SAI1_TX_DMASx                   DMA2_Stream3
#define SAI1_TX_DMASx_Channel           87
#define SAI1_TX_DMASx_IRQHandler        DMA2_Stream3_IRQHandler
#define SAI1_TX_DMASx_IRQn              DMA2_Stream3_IRQn
#define SAI1_TX_DMA_CLK_ENABLE()        do{ RCC->AHB1ENR |= 1 << 1; }while(0)       /* DMA2 钟使能 */

#define SAI1_TX_DMASx_IS_TC()           ( DMA2->LISR & (1 << 27) )                  /* 判断 DMA2_Stream3 传输完成标志, 这是一个假函数形式,
                                                                                     * 不能当函数使用, 只能用在if等语句里面 
                                                                                     */
#define SAI1_TX_DMASx_CLR_TC()          do{ DMA2->LIFCR |= 1 << 27; }while(0)       /* 清除 DMA2_Stream3 传输完成标志 */

/* SAI1 RX DMA相关定义 */
#define SAI1_RX_DMASx                   DMA2_Stream5
#define SAI1_RX_DMASx_Channel           88
#define SAI1_RX_DMASx_IRQHandler        DMA2_Stream5_IRQHandler
#define SAI1_RX_DMASx_IRQn              DMA2_Stream5_IRQn
#define SAI1_RX_DMA_CLK_ENABLE()        do{ RCC->AHB1ENR |= 1 << 1; }while(0)       /* DMA2 钟使能 */

#define SAI1_RX_DMASx_IS_TC()           ( DMA2->HISR & (1 << 11) )                  /* 判断 DMA2_Stream5 传输完成标志, 这是一个假函数形式,
                                                                                     * 不能当函数使用, 只能用在if等语句里面 
                                                                                     */
#define SAI1_RX_DMASx_CLR_TC()          do{ DMA2->HIFCR |= 1 << 11; }while(0)       /* 清除 DMA2_Stream5 传输完成标志 */

/******************************************************************************************/

extern void (*sai1_tx_callback)(void);          /* sai1 tx回调函数指针 */
extern void (*sai1_rx_callback)(void);          /* sai1 rx回调函数指针 */

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





















