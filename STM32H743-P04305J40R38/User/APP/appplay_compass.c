/**
 ****************************************************************************************************
 * @file        appplay_compass.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-06
 * @brief       APP-指南针 代码
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
 * V1.0 20221206
 * 第一次发布
 ****************************************************************************************************
 */

#include "appplay_compass.h"
#include "./BSP/ST480MC/st480mc.h"
#include "./BSP/24CXX/24cxx.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LED/led.h"


/* 磁力计提示 */
uint8_t *const appplay_compass_remind_tbl[2][GUI_LANGUAGE_NUM] =
{
    {"如需校准,请按KEY_UP按键", "如需校準,請按KEY_UP按鍵", "Press KEY_UP to adjust",},
    {"初始化错误,请检查...", "初始化錯誤,請檢查...", "Init Error,Please check...",},
};

/* 磁力计提示 */
uint8_t *const appplay_compass_position_tbl[8][GUI_LANGUAGE_NUM] =
{
    {"北", "北", "N",},
    {"东北", "東北", "EN",},
    {"东", "東", "E",},
    {"东南", "東南", "ES",},
    {"南", "南", "S",},
    {"西南", "西南", "WS",},
    {"西", "西", "W",},
    {"西北", "西北", "WN",},
};

/* 磁力计提示 */
uint8_t *const appplay_compass_direction_tbl[4][GUI_LANGUAGE_NUM] =
{
    {"南", "南", "S",},
    {"西", "西", "W",},
    {"北", "北", "N",},
    {"东", "東", "E",},
};

/**
 * 磁力计平面校准参数
 * 校准原理请参考(方法2):http://blog.sina.com.cn/s/blog_402c071e0102v8ie.html
 */
int16_t g_magx_offset = 0;      /* x轴补偿值 */
int16_t g_magy_offset = 0;      /* y轴补偿值 */

/**
 * @brief       罗盘(磁力计)校准函数
 *   @note      这里我们使用最简单的平面校准方法.
 *              进入此函数后,请水平转动开发板至少一周(360°),转动完成后, 按WKUP键退出!
 * @param       无
 * @retval      无
 */
void appplay_compass_calibration(void)
{
    int16_t x_min = 0;
    int16_t x_max = 0;
    int16_t y_min = 0;
    int16_t y_max = 0;

    int16_t magx, magy, magz;
    uint8_t res;
    uint8_t key = 0;
    
    lcd_clear(BLACK);
    lcd_show_string(10, 90, 240, 16, 16, "Compass Calibration", WHITE);
    lcd_show_string(10, 110, 240, 16, 16, "Pls rotate horiz one cycle!", WHITE);
    lcd_show_string(10, 130, 240, 16, 16, "If done, press KEY_UP key!", WHITE);
    
    while (1)
    {
        key = key_scan(0);
        
        if (key == WKUP_PRES)   /* 结束校准 */
        {
            break;
        }
        
        res = st480mc_read_magdata(&magx, &magy, &magz);    /* 读取数据 */
        
        if (res == 0)
        {
            x_max = x_max < magx ? magx : x_max;            /* 记录x最大值 */
            x_min = x_min > magx ? magx : x_min;            /* 记录x最小值 */
            y_max = y_max < magy ? magy : y_max;            /* 记录y最大值 */
            y_min = y_min > magy ? magy : y_min;            /* 记录y最小值 */
        }
        
        LED0_TOGGLE();          /* LED0闪烁,提示程序运行 */
    }
    
    g_magx_offset = (x_max + x_min) / 2;    /* X轴偏移量 */
    g_magy_offset = (y_max + y_min) / 2;    /* Y轴偏移量 */
   
    /* 串口打印水平校准相关参数 */
    printf("x_min:%d\r\n", x_min);
    printf("x_max:%d\r\n", x_max);
    printf("y_min:%d\r\n", y_min);
    printf("y_max:%d\r\n", y_max);
    
    printf("g_magx_offset:%d\r\n", g_magx_offset);
    printf("g_magy_offset:%d\r\n", g_magy_offset);
}

/**
 * @brief       罗盘获取角度
 *   @note      获取当前罗盘的角度(地磁角度)
 * @param       无
 * @retval      角度
 */
float appplay_compass_get_angle(void)
{
    float angle;
    int16_t magx, magy, magz;
    
    st480mc_read_magdata_average(&magx, &magy, &magz, 15);  /* 读取原始数据, 15次取平均 */
    
    magx = (magx - g_magx_offset) ; /* 根据校准参数, 计算新的输出 */
    magy = (magy - g_magy_offset) ; /* 根据校准参数, 计算新的输出 */

    /* 根据不同的象限情况, 进行方位角换算 */
    if ((magx > 0) && (magy > 0))
    {
        angle = (atan((double)magy / magx) * 180) / 3.14159f;
    }
    else if ((magx > 0) && (magy < 0))
    {
        angle = 360 + (atan((double)magy / magx) * 180) / 3.14159f;
    }
    else if ((magx == 0) && (magy > 0))
    {
        angle = 90;
    }
    else if ((magx == 0) && (magy < 0))
    {
        angle = 270;
    }
    else if (magx < 0)
    {
        angle = 180 + (atan((double)magy / magx) * 180) / 3.14159f;
    }
    
    if (angle > 360) angle = 360;   /* 限定方位角范围 */
    if (angle < 0) angle = 0;       /* 限定方位角范围 */

    return angle;             /* 角度变换一下 */
}

/**
 * @brief       画指针
 * @param       x,y             : 坐标中心点
 * @param       width           : 最宽处宽度
 * @param       length          : 指针长度
 * @param       coloru          : 上部分颜色
 * @param       colord          : 下部分颜色
 * @retval      无
 */
void appplay_compass_cross_show(uint16_t x, uint16_t y, uint16_t width, uint16_t length, uint16_t coloru, uint16_t colord)
{
    uint16_t x0, x1, x2, y0, y1, y2;
    
    x0 = x - width / 2;
    x1 = x + width / 2;
    x2 = x;
    y0 = y1 = y;
    y2 = y - length;
    gui_fill_triangle(x0, y0, x1, y1, x2, y2, coloru);
    
    y2 = y + length;
    gui_fill_triangle(x0, y0, x1, y1, x2, y2, colord);
    
}

/**
 * @brief       画圆形指针表盘
 * @param       x,y             : 坐标中心点
 * @param       size            : 表盘大小(直径)
 * @param       d               : 表盘分割,分针的高度
 * @param       fsize           : 字体大小
 * @param       arg             : 角度
 * @param       mode            : 模式: 0, 清除上一次显示
 *                                      1, 显示新的值
 * @retval      无
 */
void appplay_compass_circle_panel(uint16_t x, uint16_t y, uint16_t size, uint16_t d, uint8_t fsize, int16_t arg, uint8_t mode)
{
    uint16_t r = size / 2; /* 得到半径 */
    uint16_t sx = x - r;
    uint16_t sy = y - r;
    uint16_t px0, px1;
    uint16_t py0, py1;
    uint16_t cx, cy;
    uint16_t i;
    uint16_t color;
    
    double roate = arg;

    roate = (roate / 360) * 2 * app_pi;

    if(mode == 0)color = BLACK;
    else color = WHITE;
    
    for (i = 0; i < 60; i++) /* 画6°格 */
    {
        px0 = sx + r + (r - 4) * sin((app_pi / 30) * i + roate);
        py0 = sy + r - (r - 4) * cos((app_pi / 30) * i + roate);
        px1 = sx + r + (r - d) * sin((app_pi / 30) * i + roate);
        py1 = sy + r - (r - d) * cos((app_pi / 30) * i + roate);
        
        
        gui_draw_bline1(px0, py0, px1, py1, 0, color);
    }

    for (i = 0; i < 12; i++) /* 画30°格 */
    {
        px0 = sx + r + (r - 5) * sin((app_pi / 6) * i + roate);
        py0 = sy + r - (r - 5) * cos((app_pi / 6) * i + roate);
        px1 = sx + r + (r - d) * sin((app_pi / 6) * i + roate);
        py1 = sy + r - (r - d) * cos((app_pi / 6) * i + roate);
        gui_draw_bline1(px0, py0, px1, py1, 2, color);
    }

    for (i = 0; i < 4; i++) /* 画90°格 */
    {
        px0 = sx + r + (r - 5) * sin((app_pi / 2) * i + roate);
        py0 = sy + r - (r - 5) * cos((app_pi / 2) * i + roate);
        px1 = sx + r + (r - d - 3) * sin((app_pi / 2) * i + roate);
        py1 = sy + r - (r - d - 3) * cos((app_pi / 2) * i + roate);
        gui_draw_bline1(px0, py0, px1, py1, 2, color);
        
        cx = sx + r + (r - d - 3 - fsize) * sin((app_pi / 2) * i + roate);
        cy = sy + r - (r - d - 3 - fsize) * cos((app_pi / 2) * i + roate);

        if(gui_phy.language == 2)   /* 英文/4 */
        {
            cx -= fsize/4;
        }
        else    /* 中文/2 */
        {
            cx -= fsize/2;
        }
        
        cy -= fsize/2;
        
        if(mode && i == 2)  /* 显示状态 ,且是北方, 则显示红色 */
        {
            gui_show_string(appplay_compass_direction_tbl[i][gui_phy.language],cx, cy, fsize, fsize, fsize, RED);
        }
        else
        {
            gui_show_string(appplay_compass_direction_tbl[i][gui_phy.language],cx, cy, fsize, fsize, fsize, color);
        }
     
    }
}

/**
 * @brief       指南针UI加载
 * @param       x,y             : 坐标中心点
 * @param       pw,ph           : 指针宽度,高度
 * @param       r               : 表盘半径
 * @param       caption         : 标题
 * @retval      无
 */
void appplay_compass_ui_load(uint16_t x, uint16_t y, uint16_t r, uint16_t pw, uint16_t ph, uint8_t * caption)
{
    lcd_clear(BLACK);
    app_gui_tcbar(0, 0, lcddev.width, gui_phy.tbheight, 0x02);                                          /* 下分界线 */
    gui_show_strmid(0, 0, lcddev.width, gui_phy.tbheight, WHITE, gui_phy.tbfsize, (uint8_t *)caption);  /* 显示标题 */

    gui_fill_circle(x, y, r, WHITE);                        /* 画外圈 */
    gui_fill_circle(x, y, r - 4, BLACK);                    /* 画内圈 */
    appplay_compass_cross_show(x, y, pw, ph, BLUE, RED);    /* 画指针 */
}

/**
 * @brief       显示角度（指北针盘）
 * @param       x,y             : 中心坐标(居中显示)
 * @param       fsize           : 角度值字体16/24/32
 * @param       angle           : 角度 0 ~ 360,单位:1°
 * @retval      无
 */
void appplay_compass_show_angle(uint16_t x, uint16_t y, uint16_t fsize, uint16_t angle)
{
    uint8_t *buf; 
    uint8_t pos = 0;
    
    x -= fsize * 3;

    buf = gui_memin_malloc(100);
    
    if(angle >  338 || angle <= 22) pos = 0;
    else if(angle >  22 && angle <= 68) pos = 1;
    else if(angle >  68 && angle <= 112) pos = 2;
    else if(angle >  112 && angle <= 158) pos = 3;
    else if(angle >  158 && angle <= 202) pos = 4;
    else if(angle >  202 && angle <= 248) pos = 5;
    else if(angle >  248 && angle <= 292) pos = 6;
    else if(angle >  292 && angle <= 338) pos = 7;
    
    sprintf((char *)buf, "%d°%s", angle, appplay_compass_position_tbl[pos][gui_phy.language]); /* 百分比 */
    
    gui_fill_rectangle(x, y, 6 * fsize, fsize, BLACK);            /* 填充底色 */
    
    gui_show_strmid(x, y, 6 * fsize, fsize, YELLOW, fsize, buf);  /* 显示角度 */
    
    gui_memin_free(buf);
}

/**
 * @brief       罗盘演示
 * @param       caption         : 标题
 * @retval      无
 */
uint8_t appplay_compass_play(uint8_t *caption)
{
    uint16_t angle;
    uint8_t rval = 0;
    uint8_t ry;             /* 表盘半径 */
    uint8_t dy;             /* 表盘刻度高度 */
    uint16_t pcx,pcy;       /* 表盘中心坐标 */
    uint8_t fsize;
    uint8_t key = 0;
    
    uint16_t old_angle = 0;
    
    /* 提示想校准的话，按KEY_UP按键 */
    window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)appplay_compass_remind_tbl[0][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 16, 0, 0, 0);
    delay_ms(2000);

    if (st480mc_init()) /* 初始化ST480MC */
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)appplay_compass_remind_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
        delay_ms(500);
        rval = 1;
    }
    else                /* 初始化OK */
    {
        pcx = lcddev.width / 2;                         /* 横坐标居中 */
        pcy = (lcddev.height - gui_phy.tbheight) / 2;   /* 纵坐标居中 */
        
        if (lcddev.width == 240)
        {
            ry = 80;
            dy = 8;
            fsize = 12;
        }
        else if (lcddev.width == 320)
        {
            ry = 140;
            dy = 9;
            fsize = 16;
        }
        else if (lcddev.width == 480)
        {
            ry = 200;
            dy = 12;
            fsize = 24;
        }

        appplay_compass_ui_load(pcx, pcy, ry, dy, ry*2/3, caption); /* 加载 UI界面 */

        while (1)
        {
            if (system_task_return)
            {
                delay_ms(10);
                if (tpad_scan(1)) 
                {
                    break;  /* TPAD返回,再次确认,排除干扰 */
                }
                else system_task_return = 0;
            }

            key = key_scan(0);

            if(key == WKUP_PRES)
            {
                appplay_compass_calibration(); 
                appplay_compass_ui_load(pcx, pcy, ry, dy, ry*2/3, caption); /* 加载 UI界面 */
           }                

            angle = appplay_compass_get_angle();    /* 获取磁力计角度 */

            
            if(old_angle != angle)
            {
                appplay_compass_show_angle(pcx, pcy + ry + fsize, fsize, 360 - angle);      /* 显示角度&方位 */
                
                appplay_compass_circle_panel(pcx, pcy, 2 * ry, dy, fsize, old_angle, 0);    /* 画航向角表盘 */
                
                appplay_compass_circle_panel(pcx, pcy, 2 * ry, dy, fsize, angle , 1);       /* 画航向角表盘 */
                
                old_angle = angle;
            }
            
            delay_ms(10);
        }
    }

    return rval;
}





















