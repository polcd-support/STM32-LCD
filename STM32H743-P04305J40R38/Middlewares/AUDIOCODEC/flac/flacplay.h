/**
 ****************************************************************************************************
 * @file        flacplay.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       flac���� ����
 *              1, ֧��16/24λ������/������flac�Ľ���
 *              2, ���֧��192K/16bit��96K/24bit��flac����
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221127
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __FLACPLAY_H__
#define __FLACPLAY_H__

#include <inttypes.h>
#include <string.h>
#include "flacdecoder.h"
#include "./SYSTEM/sys/sys.h"
#include "./FATFS/source/ff.h"


/* flaC ��ǩ */
typedef __packed struct
{
    uint8_t id[3];      /* ID,���ļ���ʼλ��,������flaC 4����ĸ */
} FLAC_Tag;

/* metadata ���ݿ�ͷ��Ϣ�ṹ�� */
typedef __packed struct
{
    uint8_t head;       /* metadata blockͷ */
    uint8_t size[3];    /* metadata block���ݳ��� */
} MD_Block_Head;

/* FLAC���ƽṹ�� */
typedef __packed struct
{
    uint32_t totsec ;   /* ���׸�ʱ��,��λ:�� */
    uint32_t cursec ;   /* ��ǰ����ʱ�� */

    uint32_t bitrate;   /* ������ */
    uint32_t samplerate;/* ������ */
    uint16_t outsamples;/* PCM�����������С */
    uint16_t bps;       /* λ��,����16bit,24bit,32bit */

    uint32_t datastart; /* ����֡��ʼ��λ��(���ļ������ƫ��) */
} __flacctrl;

extern __flacctrl *flacctrl;


uint8_t flac_init(FIL *fx, __flacctrl *fctrl, FLACContext *fc);
void flac_i2s_dma_tx_callback(void);
void flac_get_curtime(FIL *fx, __flacctrl *flacx);
uint8_t flac_play_song(uint8_t *fname);

#endif




























