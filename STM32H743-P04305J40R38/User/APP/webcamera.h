/**
 ****************************************************************************************************
 * @file        webcamera.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       APP-��������ͷ����
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
 * V1.1 20220526
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�uint8_t/uint16_t/uint32_tΪuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#ifndef __WEBCAMERA_H
#define __WEBCAMERA_H 

#include "netplay.h" 


#define WEBCAM_FIFO_NUM         100     //����FIFO����
#define WEBCAM_LINE_SIZE        4096    //�����д�С(*4�ֽ�)

 
uint8_t webcam_play(void);
#endif

