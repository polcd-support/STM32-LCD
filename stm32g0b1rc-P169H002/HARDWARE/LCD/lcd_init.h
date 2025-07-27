#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "main.h"

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 240
#define LCD_H 280

#else
#define LCD_W 280
#define LCD_H 240
#endif

// 清屏按钮区域（方屏右下角）
#define CLEAR_BTN_X1 (LCD_WIDTH - 60)
#define CLEAR_BTN_Y1 (LCD_HEIGHT - 40)
#define CLEAR_BTN_X2 (LCD_WIDTH - 10)
#define CLEAR_BTN_Y2 (LCD_HEIGHT - 10)



//-----------------LCD端口定义---------------- 

#define LCD_RES_Clr()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)  // RES
#define LCD_RES_Set()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)

#define LCD_DC_Clr()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)  // DC
#define LCD_DC_Set()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)
 		     
#define LCD_CS_Clr()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET)  // CS
#define LCD_CS_Set()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET)

#define LCD_BLK_Clr()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)  // BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)

void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(uint8_t dat);//模拟SPI时序
void LCD_WR_DATA8(uint8_t dat);//写入一个字节
void LCD_WR_DATA(uint16_t dat);//写入两个字节
void LCD_WR_REG(uint8_t dat);//写入一个指令
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif




