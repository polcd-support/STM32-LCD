/**
 ****************************************************************************************************
 * @file        memlcd.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.2
 * @date        2022-05-26
 * @brief       GUI代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#ifndef __MEMLCD_H
#define __MEMLCD_H
#include "guix.h"
#include "stdlib.h" 

#define MEMLCD_MAX_LAYER    1               /* MEM LCD所支持的显存层数 */

/* LCD重要参数集 */
typedef struct  
{
    uint16_t width;                         /* MEM LCD 宽度 */
    uint16_t height;                        /* MEM LCD 高度 */
    uint8_t  dir;                           /* 横屏还是竖屏控制：0，竖屏；1，横屏。 */
    uint16_t *grambuf[MEMLCD_MAX_LAYER];    /* MEM LCD GRAM */
    uint8_t layer;                          /* 操作层,0,第一层;1,第二层;2,第三层,以此类推. */
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
