#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "main.h"

#define USE_HORIZONTAL 0

/* ST7789P3 panel: 240 x 320 active pixels, 完整显存无需偏移 */
#if USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1
#define LCD_W 240
#define LCD_H 320
#define LCD_X_OFFSET 0
#define LCD_Y_OFFSET 0
#else
#define LCD_W 320
#define LCD_H 240
#define LCD_X_OFFSET 0
#define LCD_Y_OFFSET 0
#endif



//-----------------LCD端口定义---------------- 

#define LCD_RES_Clr()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)  // RES
#define LCD_RES_Set()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)

#define LCD_CS_Clr()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET)  // CS
#define LCD_CS_Set()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET)

#define LCD_BLK_Clr()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)  // BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)

/* 面板为三线九位串行: 命令/数据由第9位区分, 不使用DC引脚
   发送由硬件SPI1(PA1=SCK, PA7=MOSI, 9bit帧) + DMA1_Channel1(寄存器级)完成 */
typedef uint16_t lcd_frame_t;
#define LCD_FRAME_DATA(b)  ((lcd_frame_t)(0x100u | (uint8_t)(b)))  // 第9位=1: 数据

extern uint8_t lcd_dma_status;        // DMA通路状态: 0=正常 1=寄存器写入未生效 2=超时(已退回阻塞发送)

void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(uint8_t dat);//写入一个数据字节
void LCD_WR_DATA8(uint8_t dat);//写入一个字节
void LCD_WR_DATA(uint16_t dat);//写入两个字节
void LCD_WR_DATA_Bulk(const lcd_frame_t *frames, uint32_t count);//连续写入像素数据帧
void LCD_WR_REG(uint8_t dat);//写入一个指令
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif




