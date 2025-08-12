/**
 ****************************************************************************************************
 * @file        keyplay.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-01
 * @brief       APP-按键测试 代码
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

#include "keyplay.h"
#include "gradienter.h"
#include "./BSP/KEY/key.h"


/**
 * @brief       显示圆形提示信息
 * @param       x,y             : 要显示的圆中心坐标
 * @param       r               : 半径
 * @param       fsize           : 字体大小
 * @param       color           : 圆的颜色
 * @param       str             : 显示的字符串
 * @retval      无
 */
void key_show_circle(uint16_t x, uint16_t y, uint16_t r, uint8_t fsize, uint16_t color, uint8_t *str)
{
    gui_fill_circle(x, y, r, color);
    gui_show_strmid(x - r, y - fsize / 2, 2 * r, fsize, BLUE, fsize, str); /* 显示标题 */
}

/**
 * @brief       按键测试
 * @param       caption         : 窗口名字
 * @retval      未用到
 */
uint8_t key_play(void)
{
    uint8_t key;
    uint16_t k0x, k1x, k2x, kux;
    uint16_t kuy, koy;
    uint16_t kcr;
    uint8_t fsize = 0;      /* key字体大小 */

    uint8_t keyold = 0XFF;  /* 按键和之前的按键值 */

    kcr = lcddev.width / 10;
    k2x = 2 * kcr;
    k1x = k2x + 3 * kcr;
    k0x = k1x + 3 * kcr;
    kux = k1x;
    kuy = (lcddev.height - 5 * kcr) / 2 + kcr;
    koy = kuy + 3 * kcr;

    if (lcddev.width <= 272)
    {
        fsize = 12;
    }
    else if (lcddev.width == 320)
    {
        fsize = 16;
    }
    else if (lcddev.width >= 480)
    {
        fsize = 24;
    }

    lcd_clear(LGRAY);
    app_gui_tcbar(0, 0, lcddev.width, gui_phy.tbheight, 0x02);  /* 下分界线 */
    gui_show_strmid(0, 0, lcddev.width, gui_phy.tbheight, WHITE, gui_phy.tbfsize, (uint8_t*)APP_MFUNS_CAPTION_TBL[22][gui_phy.language]); /* 显示标题 */
    system_task_return = 0;

    while (1)
    {
        key = key_scan(1);

        if (key != keyold)
        {
            keyold = key;

            if (key == KEY0_PRES)key_show_circle(k0x, koy, kcr, fsize, RED, (uint8_t *)"KEY0");
            else key_show_circle(k0x, koy, kcr, fsize, YELLOW, (uint8_t *)"KEY0");

            if (key == KEY1_PRES)key_show_circle(k1x, koy, kcr, fsize, RED, (uint8_t *)"KEY1");
            else key_show_circle(k1x, koy, kcr, fsize, YELLOW, (uint8_t *)"KEY1");

            if (key == KEY2_PRES)key_show_circle(k2x, koy, kcr, fsize, RED, (uint8_t *)"KEY2");
            else key_show_circle(k2x, koy, kcr, fsize, YELLOW, (uint8_t *)"KEY2");

            if (key == WKUP_PRES)key_show_circle(kux, kuy, kcr, fsize, RED, (uint8_t *)"KEYUP");
            else key_show_circle(kux, kuy, kcr, fsize, YELLOW, (uint8_t *)"KEYUP");
        }

        if (system_task_return)
        {
            delay_ms(10);
            if (tpad_scan(1)) 
            {
                break;  /* TPAD返回,再次确认,排除干扰 */
            }
            else system_task_return = 0;
        }

        delay_ms(10);
    }

    return 0;
}







































