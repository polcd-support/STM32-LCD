#include "lcd_demo.h"

#include "lcd.h"
#include "pic.h"
#include "delay.h"

typedef enum {
    STATE_LOGO,
    STATE_TEXT,
	STATE_COLOR_FULL,
    STATE_COLOR_BAR,
    STATE_GRAYSCALE
} AppState;

AppState g_state = STATE_LOGO;
uint32_t g_state_timer = 0;
uint8_t color_full_index = 0;
extern const uint8_t gImage_logo[];

// 新屏 P0273B02-IPS（ST7365P / 320x320 / 无触摸）
// 循环播放：开机 LOGO -> 文字 -> 纯色 -> 彩条 -> 灰度 -> 回到 LOGO
void LCD_DEMO(void)
{

	LCD_Init();

	// clear
	LCD_Clear(BLACK);
	delay_ms(100);
	LCD_BLK_Set();//打开背光

	while (1)
	{
		switch (g_state) {
			case STATE_LOGO:
				// LOGO 为 320x293，垂直居中显示
				LCD_ShowPicture(0, (SCREEN_HEIGHT - 293) / 2, 320, 293, gImage_logo);

				if (HAL_GetTick() - g_state_timer > LOGO_DURATION) {
					LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
					g_state = STATE_TEXT;
					g_state_timer = HAL_GetTick();
				}
				break;

			case STATE_TEXT:
				LCD_ShowString(20, 50, "STM32 Display", WHITE, BLACK, 24, 0);
				LCD_ShowString(30, 100, "Multi-Size Text", BLUE, BLACK, 16, 0);
				LCD_ShowChinese(80, 150, "浦洋液晶", RED, BLACK, 32, 0);

				if (HAL_GetTick() - g_state_timer > TEXT_DURATION) {
					LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
					g_state = STATE_COLOR_FULL;
					g_state_timer = HAL_GetTick();
				}
				break;

			case STATE_COLOR_FULL:
				switch (color_full_index) {
					case 0: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, RED);   break;
					case 1: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, GREEN); break;
					case 2: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLUE);  break;
					case 3: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, WHITE); break;
					case 4: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK); break;
				}

				if (HAL_GetTick() - g_state_timer > COLOR_FULL_INTERVAL) {
					if (++color_full_index > 4) {
						g_state = STATE_COLOR_BAR;
						color_full_index = 0;
					}
					g_state_timer = HAL_GetTick();
				}
				break;

			case STATE_COLOR_BAR:
				DrawColorBars();
				if (HAL_GetTick() - g_state_timer > EFFECT_DURATION) {
					g_state = STATE_GRAYSCALE;
					g_state_timer = HAL_GetTick();
				}
				break;

			case STATE_GRAYSCALE:
				DrawGrayscale();
				if (HAL_GetTick() - g_state_timer > EFFECT_DURATION) {
					LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
					g_state = STATE_LOGO;
					g_state_timer = HAL_GetTick();
				}
				break;
		}
	}
}
