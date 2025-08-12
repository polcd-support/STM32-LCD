/**
 ****************************************************************************************************
 * @file        memlcd.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.2
 * @date        2022-05-26
 * @brief       GUI����
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
 ****************************************************************************************************
 */

#ifndef __MEMLCD_H
#define __MEMLCD_H
#include "guix.h"
#include "stdlib.h" 

#define MEMLCD_MAX_LAYER    1               /* MEM LCD��֧�ֵ��Դ���� */

/* LCD��Ҫ������ */
typedef struct  
{
    uint16_t width;                         /* MEM LCD ��� */
    uint16_t height;                        /* MEM LCD �߶� */
    uint8_t  dir;                           /* ���������������ƣ�0��������1�������� */
    uint16_t *grambuf[MEMLCD_MAX_LAYER];    /* MEM LCD GRAM */
    uint8_t layer;                          /* ������,0,��һ��;1,�ڶ���;2,������,�Դ�����. */
}_mlcd_dev;

extern _mlcd_dev mlcddev;



uint8_t mlcd_init(uint16_t width,uint16_t height,uint8_t lx);
void mlcd_delete(void);
void mlcd_draw_point(uint16_t x,uint16_t y,uint16_t color);
uint16_t mlcd_read_point(uint16_t x,uint16_t y);
void mlcd_clear(uint16_t color);
void mlcd_fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
void mlcd_display_layer(uint8_t lx);

#endif
