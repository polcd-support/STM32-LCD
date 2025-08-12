/**
 ****************************************************************************************************
 * @file        dma.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-16
 * @brief       DMA ��������
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
 * V1.0 20230316
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __DMA_H
#define	__DMA_H

#include "./SYSTEM/sys/sys.h"


void dma_usart_tx_config(DMA_Stream_TypeDef *dma_streamx, uint8_t ch, uint32_t par, uint32_t mar);  /* ����DMAx_CHx */
void dma_mux_init(DMA_Stream_TypeDef *dma_streamx, uint8_t ch);     /* DMA����������ʼ�� */
void dma_enable(DMA_Stream_TypeDef *dma_streamx, uint16_t ndtr);    /* ʹ��һ��DMA���� */
#endif






























