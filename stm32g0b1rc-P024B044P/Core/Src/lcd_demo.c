#include "lcd_demo.h"

#include "lcd.h"
#include "ft6236.h"
#include "stdlib.h"
#include "pic.h"


typedef enum {
    STATE_LOGO,
    STATE_TEXT,
    STATE_IMAGE,
	STATE_COLOR_FULL,
    STATE_COLOR_BAR,
    STATE_GRAYSCALE,
    STATE_COUNTDOWN,
    STATE_HANDWRITING,
    STATE_PHONE_DIALER
} AppState;

AppState g_state = STATE_LOGO;
uint32_t g_state_timer = 0;
uint8_t g_img_index = 0;
uint8_t color_full_index = 0;
uint8_t g_countdown = 3;
#define DIAL_KEY_X0      12
#define DIAL_KEY_Y0      76
#define DIAL_KEY_W       64
#define DIAL_KEY_H       42
#define DIAL_KEY_X_GAP   12
#define DIAL_KEY_Y_GAP   6
#define DIAL_FUNC_Y      274
#define DIAL_HIT_MARGIN  8

static char g_dial_number[16] = {0};
static uint8_t g_dial_len = 0;
static uint8_t g_dial_touch_lock = 0;
extern const uint8_t gImage_logo[];
//extern const unsigned char gImage_img1[],gImage_img2[],gImage_img3[];

uint8_t IsTouchInButton(uint16_t x, uint16_t y) ;
static uint8_t IsTouchInDialButton(uint16_t x, uint16_t y);
static void DrawHandwritingButtons(void);
static void DrawDialerButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *text, uint16_t fill, uint16_t text_color);
static void DrawPhoneDialer(void);
static void DrawDialNumber(void);
static void HandlePhoneDialerTouch(uint16_t x, uint16_t y);
static void DrawTouchDebug(uint16_t x, uint16_t y);
static uint8_t IsPointInRect(uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint16_t rw, uint16_t rh, uint16_t margin);


void LCD_DEMO(void)
{

	LCD_Init();
	FT6236_Init();
	LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
	delay_ms(100);
	LCD_BLK_Set();//打开背光
	
	static uint16_t lastX = 0xFFFF, lastY = 0xFFFF;
	uint16_t currX,currY;
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        	FT6236_Get_Touch_Data(); // 更新触摸坐标
        switch (g_state) {
            case STATE_LOGO:
				LCD_ShowPicture(0, 29, 239, 219, gImage_logo);
                
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
                
            case STATE_IMAGE:
                switch (g_img_index) {
//					case 0: LCD_ShowPicture(0, 0, 239, 279, gImage_img1); break;
//					case 1: LCD_ShowPicture(0, 0, 239, 279, gImage_img2); break;
//					case 2: LCD_ShowPicture(0, 0, 239, 171, gImage_img3); break;

                }
                
                if (HAL_GetTick() - g_state_timer > IMAGE_INTERVAL) {
                    if (++g_img_index > 2) {
                        g_state = STATE_COLOR_BAR;
                        g_img_index = 0;
                    }
                    LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
                    g_state_timer = HAL_GetTick();
                }
                break;
            case STATE_COLOR_FULL:
				switch (color_full_index) {
					case 0: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, RED); break;
					case 1: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GREEN); break;
					case 2: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLUE); break;
					case 3: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); break;
					case 4: LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK); break;
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
                    LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
                    g_state = STATE_GRAYSCALE;
                    g_state_timer = HAL_GetTick();
                }
                break;
                
            case STATE_GRAYSCALE:
                DrawGrayscale();
                if (HAL_GetTick() - g_state_timer > EFFECT_DURATION) {
                    LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
                    g_state = STATE_HANDWRITING;
                    g_state_timer = HAL_GetTick();
					DrawHandwritingButtons();
                }
                break;
                
            case STATE_COUNTDOWN:
                LCD_ShowIntNum(100, 120, g_countdown, 1, RED, BLACK, 32);
                if (HAL_GetTick() - g_state_timer > 1000) {
                    if (--g_countdown == 0) {
                        LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
                        DrawHandwritingButtons();
                        g_state = STATE_HANDWRITING;
                    }
                    g_state_timer = HAL_GetTick();
                }
                break;
                
            case STATE_HANDWRITING:
                // 触摸绘图
                
                if (FT6236_Instance.Touch_Count > 0) {
                    currX = FT6236_Instance.X_Pos;
                    currY = FT6236_Instance.Y_Pos;

                    if (IsTouchInButton(currX, currY)) {
                        LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
                        DrawHandwritingButtons();
                        lastX = lastY = 0xFFFF;
                    } else if (IsTouchInDialButton(currX, currY)) {
                        LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
                        g_dial_len = 0;
                        g_dial_number[0] = '\0';
                        g_dial_touch_lock = 1;
                        DrawPhoneDialer();
                        g_state = STATE_PHONE_DIALER;
                        lastX = lastY = 0xFFFF;
                    } else {
                        if(lastX != 0xFFFF && lastY != 0xFFFF) {
                            LCD_DrawThickLine(lastX, lastY, currX, currY, WHITE,2);
                        }
                        lastX = currX;
                        lastY = currY;
                    }
                } else {
                    lastX = lastY = 0xFFFF; // 手指抬起时重置
                    g_dial_touch_lock = 0;
                }
                break;

            case STATE_PHONE_DIALER:
                if (FT6236_Instance.Touch_Count > 0) {
                    if (g_dial_touch_lock == 0) {
                        HandlePhoneDialerTouch(FT6236_Instance.X_Pos, FT6236_Instance.Y_Pos);
                        g_dial_touch_lock = 1;
                    }
                } else {
                    g_dial_touch_lock = 0;
                }
                break;
        }
    }
}

uint8_t IsTouchInButton(uint16_t x, uint16_t y) {
    return (x >= SCREEN_WIDTH-BTN_WIDTH) && 
           (y >= SCREEN_HEIGHT-BTN_HEIGHT);
}

static uint8_t IsTouchInDialButton(uint16_t x, uint16_t y) {
    return (x < BTN_WIDTH + 18) && (y >= SCREEN_HEIGHT-BTN_HEIGHT-18);
}

static void DrawHandwritingButtons(void) {
    DrawClearButton();
    LCD_FillRect_FastStatic(0, SCREEN_HEIGHT-BTN_HEIGHT-10, BTN_WIDTH+10, SCREEN_HEIGHT, DARKBLUE);
    LCD_ShowString(12, SCREEN_HEIGHT-BTN_HEIGHT+3, "Dial", WHITE, DARKBLUE, 16, 0);
}

static void DrawDialerButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *text, uint16_t fill, uint16_t text_color) {
    LCD_FillRect_FastStatic(x, y, x+w-1, y+h-1, fill);
    LCD_DrawRectangle(x, y, x+w-1, y+h-1, WHITE);
    LCD_ShowString(x + (w > 32 ? (w-16)/2 : 4), y + (h-16)/2, text, text_color, fill, 16, 0);
}

static void DrawDialNumber(void) {
    LCD_FillRect_FastStatic(8, 32, SCREEN_WIDTH-9, 84, BLACK);
    LCD_DrawRectangle(8, 32, SCREEN_WIDTH-9, 70, GRAY);
    LCD_ShowString(14, 44, (uint8_t *)g_dial_number, WHITE, BLACK, 24, 0);
}

static void DrawPhoneDialer(void) {
    const uint8_t *keys[12] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};
    uint8_t i;
    uint16_t row;
    uint16_t col;

    LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    LCD_ShowString(60, 8, "Phone Dial", WHITE, BLACK, 16, 0);
    DrawDialNumber();

    for (i = 0; i < 12; i++) {
        row = i / 3;
        col = i % 3;
        DrawDialerButton(DIAL_KEY_X0 + col * (DIAL_KEY_W + DIAL_KEY_X_GAP), DIAL_KEY_Y0 + row * (DIAL_KEY_H + DIAL_KEY_Y_GAP), DIAL_KEY_W, DIAL_KEY_H, keys[i], LGRAYBLUE, BLACK);
    }

    DrawDialerButton(12, DIAL_FUNC_Y, 64, 34, "Back", GRAY, BLACK);
    DrawDialerButton(88, DIAL_FUNC_Y, 64, 34, "Call", GREEN, BLACK);
    DrawDialerButton(164, DIAL_FUNC_Y, 64, 34, "Del", BRRED, WHITE);
}

static uint8_t IsPointInRect(uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint16_t rw, uint16_t rh, uint16_t margin) {
    uint16_t x0 = (rx > margin) ? (rx - margin) : 0;
    uint16_t y0 = (ry > margin) ? (ry - margin) : 0;
    uint16_t x1 = rx + rw + margin;
    uint16_t y1 = ry + rh + margin;

    return (x >= x0) && (x < x1) && (y >= y0) && (y < y1);
}

static void DrawTouchDebug(uint16_t x, uint16_t y) {
    LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, 24, BLACK);
    LCD_ShowString(6, 4, "X", WHITE, BLACK, 16, 0);
    LCD_ShowIntNum(22, 4, x, 3, WHITE, BLACK, 16);
    LCD_ShowString(82, 4, "Y", WHITE, BLACK, 16, 0);
    LCD_ShowIntNum(98, 4, y, 3, WHITE, BLACK, 16);
}
static void HandlePhoneDialerTouch(uint16_t x, uint16_t y) {
    DrawTouchDebug(x, y);
    const char key_chars[12] = {'1','2','3','4','5','6','7','8','9','*','0','#'};
    uint8_t row;
    uint8_t col;
    uint8_t index;
    uint16_t key_x;
    uint16_t key_y;

    for (row = 0; row < 4; row++) {
        for (col = 0; col < 3; col++) {
            key_x = DIAL_KEY_X0 + col * (DIAL_KEY_W + DIAL_KEY_X_GAP);
            key_y = DIAL_KEY_Y0 + row * (DIAL_KEY_H + DIAL_KEY_Y_GAP);
            if (IsPointInRect(x, y, key_x, key_y, DIAL_KEY_W, DIAL_KEY_H, DIAL_HIT_MARGIN)) {
                index = row * 3 + col;
                if (g_dial_len < sizeof(g_dial_number) - 1) {
                    g_dial_number[g_dial_len++] = key_chars[index];
                    g_dial_number[g_dial_len] = '\0';
                    DrawDialNumber();
                }
                return;
            }
        }
    }

    if (IsPointInRect(x, y, 12, DIAL_FUNC_Y, 64, 34, DIAL_HIT_MARGIN)) {
        LCD_FillRect_FastStatic(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
        DrawHandwritingButtons();
        g_state = STATE_HANDWRITING;
    } else if (IsPointInRect(x, y, 88, DIAL_FUNC_Y, 64, 34, DIAL_HIT_MARGIN)) {
        LCD_FillRect_FastStatic(80, 70, 160, 92, BLACK);
        LCD_ShowString(88, 74, "Calling", GREEN, BLACK, 16, 0);
    } else if (IsPointInRect(x, y, 164, DIAL_FUNC_Y, 64, 34, DIAL_HIT_MARGIN)) {
        if (g_dial_len > 0) {
            g_dial_number[--g_dial_len] = '\0';
            DrawDialNumber();
        }
    }
}