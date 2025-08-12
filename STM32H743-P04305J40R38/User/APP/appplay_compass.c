/**
 ****************************************************************************************************
 * @file        appplay_compass.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-06
 * @brief       APP-ָ���� ����
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221206
 * ��һ�η���
 ****************************************************************************************************
 */

#include "appplay_compass.h"
#include "./BSP/ST480MC/st480mc.h"
#include "./BSP/24CXX/24cxx.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LED/led.h"


/* ��������ʾ */
uint8_t *const appplay_compass_remind_tbl[2][GUI_LANGUAGE_NUM] =
{
    {"����У׼,�밴KEY_UP����", "����У��,Ո��KEY_UP���I", "Press KEY_UP to adjust",},
    {"��ʼ������,����...", "��ʼ���e�`,Ո�z��...", "Init Error,Please check...",},
};

/* ��������ʾ */
uint8_t *const appplay_compass_position_tbl[8][GUI_LANGUAGE_NUM] =
{
    {"��", "��", "N",},
    {"����", "�|��", "EN",},
    {"��", "�|", "E",},
    {"����", "�|��", "ES",},
    {"��", "��", "S",},
    {"����", "����", "WS",},
    {"��", "��", "W",},
    {"����", "����", "WN",},
};

/* ��������ʾ */
uint8_t *const appplay_compass_direction_tbl[4][GUI_LANGUAGE_NUM] =
{
    {"��", "��", "S",},
    {"��", "��", "W",},
    {"��", "��", "N",},
    {"��", "�|", "E",},
};

/**
 * ������ƽ��У׼����
 * У׼ԭ����ο�(����2):http://blog.sina.com.cn/s/blog_402c071e0102v8ie.html
 */
int16_t g_magx_offset = 0;      /* x�Ჹ��ֵ */
int16_t g_magy_offset = 0;      /* y�Ჹ��ֵ */

/**
 * @brief       ����(������)У׼����
 *   @note      ��������ʹ����򵥵�ƽ��У׼����.
 *              ����˺�����,��ˮƽת������������һ��(360��),ת����ɺ�, ��WKUP���˳�!
 * @param       ��
 * @retval      ��
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
        
        if (key == WKUP_PRES)   /* ����У׼ */
        {
            break;
        }
        
        res = st480mc_read_magdata(&magx, &magy, &magz);    /* ��ȡ���� */
        
        if (res == 0)
        {
            x_max = x_max < magx ? magx : x_max;            /* ��¼x���ֵ */
            x_min = x_min > magx ? magx : x_min;            /* ��¼x��Сֵ */
            y_max = y_max < magy ? magy : y_max;            /* ��¼y���ֵ */
            y_min = y_min > magy ? magy : y_min;            /* ��¼y��Сֵ */
        }
        
        LED0_TOGGLE();          /* LED0��˸,��ʾ�������� */
    }
    
    g_magx_offset = (x_max + x_min) / 2;    /* X��ƫ���� */
    g_magy_offset = (y_max + y_min) / 2;    /* Y��ƫ���� */
   
    /* ���ڴ�ӡˮƽУ׼��ز��� */
    printf("x_min:%d\r\n", x_min);
    printf("x_max:%d\r\n", x_max);
    printf("y_min:%d\r\n", y_min);
    printf("y_max:%d\r\n", y_max);
    
    printf("g_magx_offset:%d\r\n", g_magx_offset);
    printf("g_magy_offset:%d\r\n", g_magy_offset);
}

/**
 * @brief       ���̻�ȡ�Ƕ�
 *   @note      ��ȡ��ǰ���̵ĽǶ�(�شŽǶ�)
 * @param       ��
 * @retval      �Ƕ�
 */
float appplay_compass_get_angle(void)
{
    float angle;
    int16_t magx, magy, magz;
    
    st480mc_read_magdata_average(&magx, &magy, &magz, 15);  /* ��ȡԭʼ����, 15��ȡƽ�� */
    
    magx = (magx - g_magx_offset) ; /* ����У׼����, �����µ���� */
    magy = (magy - g_magy_offset) ; /* ����У׼����, �����µ���� */

    /* ���ݲ�ͬ���������, ���з�λ�ǻ��� */
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
    
    if (angle > 360) angle = 360;   /* �޶���λ�Ƿ�Χ */
    if (angle < 0) angle = 0;       /* �޶���λ�Ƿ�Χ */

    return angle;             /* �Ƕȱ任һ�� */
}

/**
 * @brief       ��ָ��
 * @param       x,y             : �������ĵ�
 * @param       width           : ������
 * @param       length          : ָ�볤��
 * @param       coloru          : �ϲ�����ɫ
 * @param       colord          : �²�����ɫ
 * @retval      ��
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
 * @brief       ��Բ��ָ�����
 * @param       x,y             : �������ĵ�
 * @param       size            : ���̴�С(ֱ��)
 * @param       d               : ���̷ָ�,����ĸ߶�
 * @param       fsize           : �����С
 * @param       arg             : �Ƕ�
 * @param       mode            : ģʽ: 0, �����һ����ʾ
 *                                      1, ��ʾ�µ�ֵ
 * @retval      ��
 */
void appplay_compass_circle_panel(uint16_t x, uint16_t y, uint16_t size, uint16_t d, uint8_t fsize, int16_t arg, uint8_t mode)
{
    uint16_t r = size / 2; /* �õ��뾶 */
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
    
    for (i = 0; i < 60; i++) /* ��6��� */
    {
        px0 = sx + r + (r - 4) * sin((app_pi / 30) * i + roate);
        py0 = sy + r - (r - 4) * cos((app_pi / 30) * i + roate);
        px1 = sx + r + (r - d) * sin((app_pi / 30) * i + roate);
        py1 = sy + r - (r - d) * cos((app_pi / 30) * i + roate);
        
        
        gui_draw_bline1(px0, py0, px1, py1, 0, color);
    }

    for (i = 0; i < 12; i++) /* ��30��� */
    {
        px0 = sx + r + (r - 5) * sin((app_pi / 6) * i + roate);
        py0 = sy + r - (r - 5) * cos((app_pi / 6) * i + roate);
        px1 = sx + r + (r - d) * sin((app_pi / 6) * i + roate);
        py1 = sy + r - (r - d) * cos((app_pi / 6) * i + roate);
        gui_draw_bline1(px0, py0, px1, py1, 2, color);
    }

    for (i = 0; i < 4; i++) /* ��90��� */
    {
        px0 = sx + r + (r - 5) * sin((app_pi / 2) * i + roate);
        py0 = sy + r - (r - 5) * cos((app_pi / 2) * i + roate);
        px1 = sx + r + (r - d - 3) * sin((app_pi / 2) * i + roate);
        py1 = sy + r - (r - d - 3) * cos((app_pi / 2) * i + roate);
        gui_draw_bline1(px0, py0, px1, py1, 2, color);
        
        cx = sx + r + (r - d - 3 - fsize) * sin((app_pi / 2) * i + roate);
        cy = sy + r - (r - d - 3 - fsize) * cos((app_pi / 2) * i + roate);

        if(gui_phy.language == 2)   /* Ӣ��/4 */
        {
            cx -= fsize/4;
        }
        else    /* ����/2 */
        {
            cx -= fsize/2;
        }
        
        cy -= fsize/2;
        
        if(mode && i == 2)  /* ��ʾ״̬ ,���Ǳ���, ����ʾ��ɫ */
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
 * @brief       ָ����UI����
 * @param       x,y             : �������ĵ�
 * @param       pw,ph           : ָ����,�߶�
 * @param       r               : ���̰뾶
 * @param       caption         : ����
 * @retval      ��
 */
void appplay_compass_ui_load(uint16_t x, uint16_t y, uint16_t r, uint16_t pw, uint16_t ph, uint8_t * caption)
{
    lcd_clear(BLACK);
    app_gui_tcbar(0, 0, lcddev.width, gui_phy.tbheight, 0x02);                                          /* �·ֽ��� */
    gui_show_strmid(0, 0, lcddev.width, gui_phy.tbheight, WHITE, gui_phy.tbfsize, (uint8_t *)caption);  /* ��ʾ���� */

    gui_fill_circle(x, y, r, WHITE);                        /* ����Ȧ */
    gui_fill_circle(x, y, r - 4, BLACK);                    /* ����Ȧ */
    appplay_compass_cross_show(x, y, pw, ph, BLUE, RED);    /* ��ָ�� */
}

/**
 * @brief       ��ʾ�Ƕȣ�ָ�����̣�
 * @param       x,y             : ��������(������ʾ)
 * @param       fsize           : �Ƕ�ֵ����16/24/32
 * @param       angle           : �Ƕ� 0 ~ 360,��λ:1��
 * @retval      ��
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
    
    sprintf((char *)buf, "%d��%s", angle, appplay_compass_position_tbl[pos][gui_phy.language]); /* �ٷֱ� */
    
    gui_fill_rectangle(x, y, 6 * fsize, fsize, BLACK);            /* ����ɫ */
    
    gui_show_strmid(x, y, 6 * fsize, fsize, YELLOW, fsize, buf);  /* ��ʾ�Ƕ� */
    
    gui_memin_free(buf);
}

/**
 * @brief       ������ʾ
 * @param       caption         : ����
 * @retval      ��
 */
uint8_t appplay_compass_play(uint8_t *caption)
{
    uint16_t angle;
    uint8_t rval = 0;
    uint8_t ry;             /* ���̰뾶 */
    uint8_t dy;             /* ���̶̿ȸ߶� */
    uint16_t pcx,pcy;       /* ������������ */
    uint8_t fsize;
    uint8_t key = 0;
    
    uint16_t old_angle = 0;
    
    /* ��ʾ��У׼�Ļ�����KEY_UP���� */
    window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)appplay_compass_remind_tbl[0][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 16, 0, 0, 0);
    delay_ms(2000);

    if (st480mc_init()) /* ��ʼ��ST480MC */
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)appplay_compass_remind_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
        delay_ms(500);
        rval = 1;
    }
    else                /* ��ʼ��OK */
    {
        pcx = lcddev.width / 2;                         /* ��������� */
        pcy = (lcddev.height - gui_phy.tbheight) / 2;   /* ��������� */
        
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

        appplay_compass_ui_load(pcx, pcy, ry, dy, ry*2/3, caption); /* ���� UI���� */

        while (1)
        {
            if (system_task_return)
            {
                delay_ms(10);
                if (tpad_scan(1)) 
                {
                    break;  /* TPAD����,�ٴ�ȷ��,�ų����� */
                }
                else system_task_return = 0;
            }

            key = key_scan(0);

            if(key == WKUP_PRES)
            {
                appplay_compass_calibration(); 
                appplay_compass_ui_load(pcx, pcy, ry, dy, ry*2/3, caption); /* ���� UI���� */
           }                

            angle = appplay_compass_get_angle();    /* ��ȡ�����ƽǶ� */

            
            if(old_angle != angle)
            {
                appplay_compass_show_angle(pcx, pcy + ry + fsize, fsize, 360 - angle);      /* ��ʾ�Ƕ�&��λ */
                
                appplay_compass_circle_panel(pcx, pcy, 2 * ry, dy, fsize, old_angle, 0);    /* ������Ǳ��� */
                
                appplay_compass_circle_panel(pcx, pcy, 2 * ry, dy, fsize, angle , 1);       /* ������Ǳ��� */
                
                old_angle = angle;
            }
            
            delay_ms(10);
        }
    }

    return rval;
}





















