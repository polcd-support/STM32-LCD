/**
 ****************************************************************************************************
 * @file        spblcd.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-26
 * @brief       SPB效果实现 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F429开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20221126
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __SPBLCD_H
#define	__SPBLCD_H

#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/delay/delay.h"


#define SLCD_DMA_MAX_TRANS  60*1024     /* DMA一次最多传输60K字节 */
extern uint16_t *g_sramlcdbuf;          /* SRAMLCD缓存,先在SRAM 里面将图片解码,并加入图标以及文字等信息 */


void slcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
uint16_t slcd_read_point(uint16_t x, uint16_t y);
void slcd_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *color);
void slcd_frame_sram2spi(uint8_t frame);
void slcd_dma_init(void);
void slcd_dma_enable(uint32_t x);
void slcd_frame_show(uint32_t x);

#endif

























