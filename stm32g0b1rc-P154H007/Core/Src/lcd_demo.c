#include "lcd_demo.h"

#include "lcd.h"
#include "delay.h"
#include "pic.h"

/* gImage_logo 为 240x220 的 RGB565 图片, 数据前 8 字节是取模工具生成的图片头 */
#define LOGO_W           240
#define LOGO_H           220
#define LOGO_DATA_OFFSET 8

static void LCD_Demo_Loop(void);

/* 说明: 新屏(ST7789P3 240x240)无触摸, 演示为纯显示循环 */
void LCD_DEMO(void)
{
	LCD_Init();
	LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
	delay_ms(100);
	LCD_BLK_Set();//打开背光(LEDA/LEDK 直接供电时该引脚可不接)

	while (1)
	{
		LCD_Demo_Loop();
	}
}

static void LCD_Demo_Loop(void)
{
	static const uint16_t full_colors[5] = {RED, GREEN, BLUE, WHITE, BLACK};
	uint8_t i;

	/* 1. 开机 LOGO: 240x220 居中显示 */
	LCD_ShowPicture(0, (SCREEN_HEIGHT-LOGO_H)/2, LOGO_W, LOGO_H, gImage_logo + LOGO_DATA_OFFSET);
	delay_ms(LOGO_DURATION);
	LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);

	/* 2. 字符与汉字显示 */
	LCD_ShowString(42, 52, (const uint8_t *)"STM32 Display", WHITE, BLACK, 24, 0);
	LCD_ShowString(56, 100, (const uint8_t *)"240x240 IPS", BLUE, BLACK, 16, 0);
	LCD_ShowString(56, 124, (const uint8_t *)"ST7789P3 SPI", BLUE, BLACK, 16, 0);
	LCD_ShowChinese(56, 168, (uint8_t *)"浦洋液晶", RED, BLACK, 32, 0);
	delay_ms(TEXT_DURATION);
	LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);

	/* 3. 全屏纯色: 红/绿/蓝/白/黑 */
	for (i = 0; i < 5; i++) {
		LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, full_colors[i]);
		delay_ms(COLOR_FULL_INTERVAL);
	}
	LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);

	/* 4. 彩条 */
	DrawColorBars();
	delay_ms(EFFECT_DURATION);
	LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);

	/* 5. 灰度渐变 */
	DrawGrayscale();
	delay_ms(EFFECT_DURATION);
	LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
}
