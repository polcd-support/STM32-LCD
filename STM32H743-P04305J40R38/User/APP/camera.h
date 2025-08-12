/**
 ****************************************************************************************************
 * @file        camera.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-30
 * @brief       APP-����� ����
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
 * V1.0 20221130
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#ifndef __CAMERA_H
#define __CAMERA_H
#include "common.h"


#define OV2640_RGB565_MODE      0       /* rgb565ģʽ */
#define OV2640_JPEG_MODE        1       /* jpegģʽ */

extern volatile uint8_t hsync_int;      /* ֡�жϱ�־ */
extern volatile uint8_t jpeg_size;      /* jpegͼƬ�ֱ��� */
extern volatile uint8_t ov2640_mode;    /* ����ģʽ:0,RGB565ģʽ;1,JPEGģʽ */

extern volatile uint32_t jpeg_buf_size; /* ����JPEG���ݻ���jpeg_buf�Ĵ�С(*4�ֽ�) */
extern volatile uint32_t jpeg_data_len; /* buf�е�JPEG��Ч���ݳ��� */
extern volatile uint8_t jpeg_data_ok;   /* JPEG���ݲɼ���ɱ�־
                                         * 0,����û�вɼ���;
                                         * 1,���ݲɼ�����,���ǻ�û����;
                                         * 2,�����Ѿ����������,���Կ�ʼ��һ֡����
                                         */
                                         
extern uint32_t *jpeg_buf;              /* JPEG���ݻ���buf,ͨ��malloc�����ڴ� */


void jpeg_data_process(void);
void sw_sdcard_mode(void);
void camera_new_pathname(uint8_t *pname, uint8_t mode);
uint8_t ov2640_jpg_photo(uint8_t *pname);
uint8_t camera_play(void);
#endif























