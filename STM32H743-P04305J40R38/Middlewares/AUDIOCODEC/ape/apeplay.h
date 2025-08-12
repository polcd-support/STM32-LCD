/**
 ****************************************************************************************************
 * @file        apeplay.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       ape���� ����
 *              1, ֧��16λ������/������ape�Ľ���
 *              2, ���֧��96K��APE��ʽ(LV1��LV2)
 *              3, LV1~LV3,��48K��������������,LV4,LV5��
 *              4, ��ĳЩape�ļ�,���ܲ�֧��,����Monkey's Audio�������ת��һ��,������������
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

#ifndef __APEPLAY_H__
#define __APEPLAY_H__

#include "apedecoder.h"
#include "parser.h"
#include "./SYSTEM/sys/sys.h"


#define APE_FILE_BUF_SZ         48*1024     /* APE����ʱ,�ļ�buf��С */
#define APE_BLOCKS_PER_LOOP     2*1024      /* APE����ʱ,ÿ��ѭ������block�ĸ��� */

/* APE���ƽṹ�� */
typedef __packed struct
{
    uint32_t totsec ;       /* ���׸�ʱ��,��λ:�� */
    uint32_t cursec ;       /* ��ǰ����ʱ�� */

    uint32_t bitrate;       /* ������ */
    uint32_t samplerate;    /* ������ */
    uint16_t outsamples;    /* PCM�����������С */
    uint16_t bps;           /* λ��,����16bit,24bit,32bit */

    uint32_t datastart;     /* ����֡��ʼ��λ��(���ļ������ƫ��) */
} __apectrl;

extern __apectrl *apectrl;


void ape_fill_buffer(uint32_t *buf, uint16_t size);
void ape_i2s_dma_tx_callback(void);
void ape_get_curtime(FIL *fx, __apectrl *apectrl);
uint8_t ape_play_song(uint8_t *fname);

#endif
