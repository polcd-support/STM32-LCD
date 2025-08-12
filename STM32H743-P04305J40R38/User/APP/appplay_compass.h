/**
 ****************************************************************************************************
 * @file        appplay_compass.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-06
 * @brief       APP-ָ���� ����
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
 * V1.0 20221206
 * ��һ�η���
 ****************************************************************************************************
 */

#ifndef __APPPLAY_COMPASS_H
#define __APPPLAY_COMPASS_H

#include "common.h"


void appplay_compass_calibration(void);
float appplay_compass_get_angle(void);
void appplay_compass_circle_panel(uint16_t x, uint16_t y, uint16_t size, uint16_t d, uint8_t fsize, int16_t arg, uint8_t mode);
void appplay_compass_ui_load(uint16_t x, uint16_t y, uint16_t r, uint16_t pw, uint16_t ph, uint8_t * caption);
void appplay_compass_show_angle(uint16_t x, uint16_t y, uint16_t fsize, uint16_t angle);

uint8_t appplay_compass_play(uint8_t *caption);

#endif



