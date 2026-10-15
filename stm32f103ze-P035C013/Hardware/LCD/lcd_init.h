#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "main.h"

#define LCD_W 320
#define LCD_H 480

typedef struct
{
	__IO uint16_t LCD_REG;
	__IO uint16_t LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE        ((uint32_t)(0x6C000000 | 0x000007FE))  //FSMC_NE4(LCD_CS)+FSMC_A10(LCD_RS), 16bit: (1<<10)*2-2
#define LCD             ((LCD_TypeDef *) LCD_BASE)

//-----------------LCD端口定义---------------- 

#define LCD_BLK_Clr()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)  // BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)

void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//设置坐标函数
void LCD_Init(void);//LCD初始化

void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_WriteRAM_Prepare(void);
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint32_t color);

void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram);
#endif




