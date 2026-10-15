#include "lcd_demo.h"

#include "lcd.h"

void LCD_Display_Init(void)
{
    LCD_Init();
    LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BLACK);
    LCD_BLK_Set();
}

void LCD_Display_RunTest(void)
{
    LCD_Display_Init();

    while (1) {
        DrawColorBars();
        HAL_Delay(3000);

        DrawGrayscale();
        HAL_Delay(3000);
    }
}
