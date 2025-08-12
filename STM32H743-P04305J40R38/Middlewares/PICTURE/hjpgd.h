/**
 ****************************************************************************************************
 * @file        hjpgd.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-05-23
 * @brief       ��������-jpegӲ�����벿�� ����
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20220523
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __HJPGD_H
#define __HJPGD_H

#include "./BSP/JPEGCODEC/jpegcodec.h"


extern jpeg_codec_typedef hjpgd;  

 
/* �ӿں��� */

void jpeg_dma_in_callback(void);
void jpeg_dma_out_callback(void);
void jpeg_endofcovert_callback(void);
void jpeg_hdrover_callback(void);
uint8_t hjpgd_decode(char* pname);

#endif




























