/**
 ****************************************************************************************************
 * @file        dcmi.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       DCMI ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20230324
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef _DCMI_H
#define _DCMI_H

#include "./SYSTEM/sys/sys.h"


/* DCMI DMA���ջص�����,��Ҫ�û�ʵ�ָú��� */
extern void (*dcmi_rx_callback)(void);

/* �ӿں��� */
void dcmi_init(void);   /* ��ʼ��DCMI */
void dcmi_stop(void);   /* ֹͣDCMI */
void dcmi_start(void);  /* ����DCMI */
void dcmi_dma_init(uint32_t mem0addr, uint32_t mem1addr, uint16_t memsize, uint8_t memblen, uint8_t meminc);    /* DCMI DMA���� */

/* �����ú��� */
void dcmi_cr_set(uint8_t pclk,uint8_t hsync,uint8_t vsync);
void dcmi_set_window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);

#endif





















