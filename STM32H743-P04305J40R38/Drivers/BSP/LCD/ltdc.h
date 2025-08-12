/**
 ****************************************************************************************************
 * @file        ltdc.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       LTDC 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230322
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __LTDC_H
#define __LTDC_H

#include "./SYSTEM/sys/sys.h"


/* LCD LTDC重要参数集 */
typedef struct
{
    uint32_t pwidth;        /* LCD面板的宽度,固定参数,不随显示方向改变,如果为0,说明没有任何RGB屏接入 */
    uint32_t pheight;       /* LCD面板的高度,固定参数,不随显示方向改变 */
    uint16_t hsw;           /* 水平同步宽度 */
    uint16_t vsw;           /* 垂直同步宽度 */
    uint16_t hbp;           /* 水平后廊 */
    uint16_t vbp;           /* 垂直后廊 */
    uint16_t hfp;           /* 水平前廊 */
    uint16_t vfp;           /* 垂直前廊 */
    uint8_t activelayer;    /* 当前层编号:0/1 */
    uint8_t dir;            /* 0,竖屏;1,横屏; */
    uint16_t width;         /* LCD宽度 */
    uint16_t height;        /* LCD高度 */
    uint32_t pixsize;       /* 每个像素所占字节数 */
} _ltdc_dev;

extern _ltdc_dev lcdltdc;   /* 管理LCD LTDC参数 */



#define LTDC_PIXFORMAT_ARGB8888      0X00    /* ARGB8888格式 */
#define LTDC_PIXFORMAT_RGB888        0X01    /* RGB888格式 */
#define LTDC_PIXFORMAT_RGB565        0X02    /* RGB565格式 */
#define LTDC_PIXFORMAT_ARGB1555      0X03    /* ARGB1555格式 */
#define LTDC_PIXFORMAT_ARGB4444      0X04    /* ARGB4444格式 */
#define LTDC_PIXFORMAT_L8            0X05    /* L8格式 */
#define LTDC_PIXFORMAT_AL44          0X06    /* AL44格式 */
#define LTDC_PIXFORMAT_AL88          0X07    /* AL88格式 */

/******************************************************************************************/
/* LTDC_BL/DE/VSYNC/HSYNC/CLK 引脚 定义 
 * LTDC_R3~R7, G2~G7, B3~B7,由于引脚太多,就不在这里定义了,直接在ltcd_init里面修改.所以在移植的时候,
 * 除了改这5个IO口, 还得改ltcd_init里面的R3~R7, G2~G7, B3~B7所在的IO口.
 */

/* BL和MCU屏背光脚共用 所以这里不用定义 BL引脚 */
#define LTDC_BL_GPIO_PORT               GPIOB
#define LTDC_BL_GPIO_PIN                SYS_GPIO_PIN5
#define LTDC_BL_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 1; }while(0)   /* 所在IO口时钟使能 */

#define LTDC_DE_GPIO_PORT               GPIOF
#define LTDC_DE_GPIO_PIN                SYS_GPIO_PIN10
#define LTDC_DE_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 5; }while(0)   /* 所在IO口时钟使能 */

#define LTDC_VSYNC_GPIO_PORT            GPIOI
#define LTDC_VSYNC_GPIO_PIN             SYS_GPIO_PIN9
#define LTDC_VSYNC_GPIO_CLK_ENABLE()    do{ RCC->AHB4ENR |= 1 << 8; }while(0)   /* 所在IO口时钟使能 */

#define LTDC_HSYNC_GPIO_PORT            GPIOI
#define LTDC_HSYNC_GPIO_PIN             SYS_GPIO_PIN10
#define LTDC_HSYNC_GPIO_CLK_ENABLE()    do{ RCC->AHB4ENR |= 1 << 8; }while(0)   /* 所在IO口时钟使能 */

#define LTDC_CLK_GPIO_PORT              GPIOG
#define LTDC_CLK_GPIO_PIN               SYS_GPIO_PIN7
#define LTDC_CLK_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 6; }while(0)   /* 所在IO口时钟使能 */



/* 定义颜色像素格式,一般用RGB565 */
#define LTDC_PIXFORMAT              LTDC_PIXFORMAT_RGB565

/* 定义默认背景层颜色 */
#define LTDC_BACKLAYERCOLOR         0X00000000

/* LCD帧缓冲区首地址,这里定义在SDRAM里面 */
#define LTDC_FRAME_BUF_ADDR         0XC0000000

/* LTDC背光控制 */
#define LTDC_BL(x)                  sys_gpio_pin_set(LTDC_BL_GPIO_PORT, LTDC_BL_GPIO_PIN, x)

/******************************************************************************************/



void ltdc_switch(uint8_t sw);
void ltdc_layer_switch(uint8_t layerx, uint8_t sw);
void ltdc_select_layer(uint8_t layerx);
void ltdc_display_dir(uint8_t dir);
void ltdc_draw_point(uint16_t x, uint16_t y, uint32_t color);
uint32_t ltdc_read_point(uint16_t x, uint16_t y);
void ltdc_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color);
void ltdc_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);
void ltdc_clear(uint32_t color);
uint8_t ltdc_clk_set(uint32_t pllsain, uint32_t pllsair, uint32_t pllsaidivr);
void ltdc_layer_window_config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);
void ltdc_layer_parameter_config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor);
uint16_t ltdc_panelid_read(void);
void ltdc_init(void);

#endif







