/**
 ****************************************************************************************************
 * @file        es8388.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-14
 * @brief       ES8388 ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20220114
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __ES8388_H
#define __ES8388_H

#include "./SYSTEM/SYS/sys.h"


__packed typedef struct
{
    uint8_t mvol;       /* ����(����������һ���),��Χ:0~63 */
    uint8_t cfreq[5];   /* ����Ƶ��ѡ������,ÿ��Ԫ�ؿ�ѡ��ΧΪ0~3,�Ҵ����Ƶ�ʸ�����ͬ
                         * [0]��ӦƵ��:80,105,135,175
                         * [1]��ӦƵ��:230,300,385,500
                         * [2]��ӦƵ��:650,850,1100,1400
                         * [3]��ӦƵ��:1800,2400,3200,4100
                         * [4]��ӦƵ��:5300,6900,9000,11700 
                         */
    
    uint8_t freqval[5]; /* ����Ƶ������� */
    uint8_t d3;         /* 3d���� */
    uint8_t speakersw;  /* �������ȿ���,0,�ر�;1,�� */
    uint8_t saveflag;   /* �����־,0X0A,�������;����,����δ���� */
} _es8388_obj;

extern _es8388_obj es8388set;/* ES8388������ */

#define ES8388_ADDR     0x10                        /* ES8388��������ַ,�̶�Ϊ0x10 */

uint8_t es8388_init(void);                          /* ES8388��ʼ�� */
uint8_t es8388_write_reg(uint8_t reg, uint8_t val); /* ES8388д�Ĵ��� */
uint8_t es8388_read_reg(uint8_t reg);               /* ES8388���Ĵ��� */
void es8388_sai_cfg(uint8_t fmt, uint8_t len);      /* ����SAI����ģʽ */
void es8388_hpvol_set(uint8_t volume);              /* ���ö������� */
void es8388_spkvol_set(uint8_t volume);             /* ������������ */
void es8388_3d_set(uint8_t depth);                  /* ����3D������ */
void es8388_adda_cfg(uint8_t dacen, uint8_t adcen); /* ES8388 DAC/ADC���� */
void es8388_output_cfg(uint8_t o1en, uint8_t o2en); /* ES8388 DAC���ͨ������ */
void es8388_mic_gain(uint8_t gain);                 /* ES8388 MIC��������(MIC PGA����) */
void es8388_alc_ctrl(uint8_t sel, uint8_t maxgain, uint8_t mingain);    /* ES8388 ALC���� */
void es8388_input_cfg(uint8_t in);                  /* ES8388 ADC���ͨ������ */
void es8388_eq1_eet(uint8_t cfreq,uint8_t gain);
void es8388_eq2_eet(uint8_t cfreq,uint8_t gain);
void es8388_eq3_eet(uint8_t cfreq,uint8_t gain);
void es8388_eq4_eet(uint8_t cfreq,uint8_t gain);
void es8388_eq5_eet(uint8_t cfreq,uint8_t gain);
#endif


