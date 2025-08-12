/**
 ****************************************************************************************************
 * @file        mp3play.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       mp3���� ����
 *              1, ֧��16λ������/������MP3�Ľ���
 *              2, ֧��CBR/VBR��ʽMP3����
 *              3, ֧��ID3V1��ID3V2��ǩ����
 *              4, ֧�����б�����(MP3�����320Kbps)����
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

#ifndef __MP3PLAY_H__
#define __MP3PLAY_H__

#include "./SYSTEM/sys/sys.h"
#include <mp3dec.h>


#define MP3_TITSIZE_MAX     40          /* ����������󳤶� */
#define MP3_ARTSIZE_MAX     40          /* ����������󳤶� */
#define MP3_FILE_BUF_SZ     5*1024      /* MP3����ʱ,�ļ�buf��С */

/* ID3V1 ��ǩ */
typedef __packed struct
{
    uint8_t id[3];          /* ID,TAG������ĸ */
    uint8_t title[30];      /* �������� */
    uint8_t artist[30];     /* ���������� */
    uint8_t year[4];        /* ��� */
    uint8_t comment[30];    /* ��ע */
    uint8_t genre;          /* ���� */
} ID3V1_Tag;

/* ID3V2 ��ǩͷ */
typedef __packed struct
{
    uint8_t id[3];          /* ID */
    uint8_t mversion;       /* ���汾�� */
    uint8_t sversion;       /* �Ӱ汾�� */
    uint8_t flags;          /* ��ǩͷ��־ */
    uint8_t size[4];        /* ��ǩ��Ϣ��С(��������ǩͷ10�ֽ�).����,��ǩ��С=size+10 */
} ID3V2_TagHead;

/* ID3V2.3 �汾֡ͷ */
typedef __packed struct
{
    uint8_t id[4];          /* ֡ID */
    uint8_t size[4];        /* ֡��С */
    uint16_t flags;         /* ֡��־ */
} ID3V23_FrameHead;

/* MP3 Xing֡��Ϣ(û��ȫ���г���,���г����õĲ���) */
typedef __packed struct
{
    uint8_t id[4];          /* ֡ID,ΪXing/Info */
    uint8_t flags[4];       /* ��ű�־ */
    uint8_t frames[4];      /* ��֡�� */
    uint8_t fsize[4];       /* �ļ��ܴ�С(������ID3) */
} MP3_FrameXing;

/* MP3 VBRI֡��Ϣ(û��ȫ���г���,���г����õĲ���) */
typedef __packed struct
{
    uint8_t id[4];          /* ֡ID,ΪXing/Info */
    uint8_t version[2];     /* �汾�� */
    uint8_t delay[2];       /* �ӳ� */
    uint8_t quality[2];     /* ��Ƶ����,0~100,Խ������Խ�� */
    uint8_t fsize[4];       /* �ļ��ܴ�С */
    uint8_t frames[4];      /* �ļ���֡�� */
} MP3_FrameVBRI;


/* MP3���ƽṹ�� */
typedef __packed struct
{
    uint8_t title[MP3_TITSIZE_MAX];     /* �������� */
    uint8_t artist[MP3_ARTSIZE_MAX];    /* ���������� */
    uint32_t totsec ;       /* ���׸�ʱ��,��λ:�� */
    uint32_t cursec ;       /* ��ǰ����ʱ�� */

    uint32_t bitrate;       /* ������ */
    uint32_t samplerate;    /* ������ */
    uint16_t outsamples;    /* PCM�����������С(��16λΪ��λ),������MP3,�����ʵ�����*2(����DAC���) */

    uint32_t datastart;     /* ����֡��ʼ��λ��(���ļ������ƫ��) */
} __mp3ctrl;

extern __mp3ctrl *mp3ctrl;


void mp3_sai_dma_tx_callback(void) ;
void mp3_fill_buffer(uint16_t *buf, uint16_t size, uint8_t nch);
uint8_t mp3_id3v1_decode(uint8_t *buf, __mp3ctrl *pctrl);
uint8_t mp3_id3v2_decode(uint8_t *buf, uint32_t size, __mp3ctrl *pctrl);
uint8_t mp3_get_info(uint8_t *pname, __mp3ctrl *pctrl);
uint8_t mp3_play_song(uint8_t *fname);

#endif




























