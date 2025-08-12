/**
 ****************************************************************************************************
 * @file        appplay_remote.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-01
 * @brief       APP-红外遥控器 代码
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
 * 修改说明
 * V1.0 20221101
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "appplay_remote.h"
#include "./BSP/REMOTE/remote.h"


/* 红外遥控条目信息列表 */
uint8_t *const appplay_remote_items_tbl[GUI_LANGUAGE_NUM][3] =
{
    {"  键值:", "  次数:", "  符号:",},
    {"  鍵值:", "  次數:", "  符號:",},
    {"KEYVAL:", "KEYCNT:", "SYMBOL:",},
};

/**
 * @brief       app-红外遥控解码
 * @param       caption         : 窗口名字
 * @retval      未用到
 */
uint8_t appplay_remote(uint8_t *caption)
{
    uint8_t rval = 0;
    uint8_t key;
    uint16_t sx = (lcddev.width - 120) / 2;
    uint16_t sy = (lcddev.height - 100) / 2;

    _window_obj *twin = 0;  /* 窗体 */
    twin = window_creat(sx, sy, 120, 100, 0, 1 << 6 | 1 << 0, 16);  /* 创建窗口 */

    if (twin)
    {
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;   /* 窗体主色 */
        window_draw(twin);                      /* 画窗体 */
        app_draw_smooth_line(sx + 5, sy + 32 + 1 + 22, 110, 1, 0Xb1ffc4, 0X1600b1);         /* 画彩线 */
        app_draw_smooth_line(sx + 5, sy + 32 + 1 + 22 + 22, 110, 1, 0Xb1ffc4, 0X1600b1);    /* 画彩线 */

        g_back_color = APP_WIN_BACK_COLOR;
        /* 显示条目信息 */
        gui_show_ptstr(sx + 8, sy + 32 + 1 + 3, sx + 8 + 56, sy + 32 + 1 + 3 + 16, 0, BLACK, 16, (uint8_t *)appplay_remote_items_tbl[gui_phy.language][0], 1);
        gui_show_ptstr(sx + 8, sy + 32 + 1 + 3 + 22, sx + 8 + 56, sy + 32 + 1 + 3 + 22 + 16, 0, BLACK, 16, (uint8_t *)appplay_remote_items_tbl[gui_phy.language][1], 1);
        gui_show_ptstr(sx + 8, sy + 32 + 1 + 3 + 44, sx + 8 + 56, sy + 32 + 1 + 3 + 44 + 16, 0, BLACK, 16, (uint8_t *)appplay_remote_items_tbl[gui_phy.language][2], 1);
    }
    else rval = 1;

    if (rval == 0)
    {
        g_back_color = APP_WIN_BACK_COLOR;  /* 背景色为窗体主色 */
        remote_init();                      /* 初始化红外接收 */

        while (1)
        {
            key = remote_scan();

            if (system_task_return)
            {
                delay_ms(10);
                if (tpad_scan(1)) 
                {
                    break;  /* TPAD返回,再次确认,排除干扰 */
                }
                else system_task_return = 0;
            }

            if (key)
            {
                lcd_show_num(sx + 8 + 56, sy + 32 + 1 + 3, key, 3, 16, RED);                /* 显示键值 */
                lcd_show_num(sx + 8 + 56, sy + 32 + 1 + 3 + 22, g_remote_cnt, 3, 16, RED);  /* 显示按键次数 */

                switch (key)
                {
                    case 69:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "POWER", RED);
                        break;

                    case 70:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "UP    ", RED);
                        break;

                    case 64:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "PLAY  ", RED);
                        break;

                    case 71:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "ALIEN ", RED);
                        break;

                    case 67:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "RIGHT ", RED);
                        break;

                    case 68:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "LEFT  ", RED);
                        break;

                    case 28:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "8     ", RED);
                        break;

                    case 7:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "VOL-  ", RED);
                        break;

                    case 21:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "DOWN  ", RED);
                        break;

                    case 9:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "VOL+  ", RED);
                        break;

                    case 22:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "1     ", RED);
                        break;

                    case 25:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "2     ", RED);
                        break;

                    case 13:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "3     ", RED);
                        break;

                    case 12:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "4     ", RED);
                        break;

                    case 24:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "5     ", RED);
                        break;

                    case 94:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "6     ", RED);
                        break;

                    case 8:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "7     ", RED);
                        break;

                    case 90:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "9     ", RED);
                        break;

                    case 66:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "0     ", RED);
                        break;

                    case 74:
                        lcd_show_string(sx + 8 + 56, sy + 32 + 1 + 3 + 44, 240, 320, 16, "DELETE", RED);
                        break;
                }
            }

            delay_ms(100);
        }
    }

    REMOTE_IN_TIMX->CR1 &= ~(1 << 0);   /* 关闭定时器 */
    window_delete(twin);
    return rval;
}



