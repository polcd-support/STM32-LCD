/**
 ****************************************************************************************************
 * @file        camera.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-1
 * @brief       APP-¼���� ����
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221201
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */
 
#ifndef __RECORDER_H
#define __RECORDER_H

#include "common.h"
#include "wavplay.h"


#define SAI_RX_DMA_BUF_SIZE     4096        /* ����RX DMA �����С */
#define SAI_RX_FIFO_SIZE        10          /* �������FIFO��С */

/* ��ͼ��/ͼƬ·�� */
extern uint8_t *const RECORDER_DEMO_PIC;        /* demoͼƬ·�� */
extern uint8_t *const RECORDER_RECR_PIC;        /* ¼�� �ɿ� */
extern uint8_t *const RECORDER_RECP_PIC;        /* ¼�� ���� */
extern uint8_t *const RECORDER_PAUSER_PIC;      /* ��ͣ �ɿ� */
extern uint8_t *const RECORDER_PAUSEP_PIC;      /* ��ͣ ���� */
extern uint8_t *const RECORDER_STOPR_PIC;       /* ֹͣ �ɿ� */
extern uint8_t *const RECORDER_STOPP_PIC;       /* ֹͣ ���� */


uint8_t recoder_sai_fifo_read(uint8_t **buf);
uint8_t recoder_sia_fifo_write(uint8_t *buf);
uint8_t recorder_play(void);

#endif























