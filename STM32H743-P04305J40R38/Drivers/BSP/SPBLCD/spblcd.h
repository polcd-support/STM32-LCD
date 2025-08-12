/**
 ****************************************************************************************************
 * @file        spblcd.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-26
 * @brief       SPBЧ��ʵ�� ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F429������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221126
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __SPBLCD_H
#define	__SPBLCD_H

#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/delay/delay.h"


#define SLCD_DMA_MAX_TRANS  60*1024     /* DMAһ����ഫ��60K�ֽ� */
extern uint16_t *g_sramlcdbuf;          /* SRAMLCD����,����SRAM ���潫ͼƬ����,������ͼ���Լ����ֵ���Ϣ */


void slcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
uint16_t slcd_read_point(uint16_t x, uint16_t y);
void slcd_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *color);
void slcd_frame_sram2spi(uint8_t frame);
void slcd_dma_init(void);
void slcd_dma_enable(uint32_t x);
void slcd_frame_show(uint32_t x);

#endif

























