/**
 ****************************************************************************************************
 * @file        calendar.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       APP-���� ����
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
 * V1.1 20220526
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "calendar.h"
#include "stdio.h"
#include "settings.h"
#include "math.h"
#include "camera.h"
#include "./BSP/ADC/adc3.h"
#include "./BSP/DS18B20/ds18b20.h"
#include "./BSP/24CXX/24cxx.h"
#include "./BSP/RTC/rtc.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/DCMI/dcmi.h"
#include "./FATFS/exfuns/exfuns.h"


_alarm_obj alarm;               /* ���ӽṹ�� */
_calendar_obj calendar;         /* �����ṹ�� */

static uint16_t TIME_TOPY;      /* 	120 */
static uint16_t OTHER_TOPY;     /* 	200 */

/* ���� */
uint8_t *const calendar_week_table[GUI_LANGUAGE_NUM][7] =
{
    {"������", "����һ", "���ڶ�", "������", "������", "������", "������"},
    {"������", "����һ", "���ڶ�", "������", "������", "������", "������"},
    {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
};

/* ���ӱ��� */
uint8_t *const calendar_alarm_caption_table[GUI_LANGUAGE_NUM] =
{
    "����", "�[�", "ALARM",
};

/* ���찴ť */
uint8_t *const calendar_alarm_realarm_table[GUI_LANGUAGE_NUM] =
{
    "����", "���", "REALARM",
};

/* ��ʾ��Ϣ */
uint8_t *const calendar_loading_str[GUI_LANGUAGE_NUM][3] =
{
    {
        "���ڼ���,���Ժ�...",
        "δ��⵽DS18B20!",
        "�����ڲ��¶ȴ�����...",
    },
    {
        "���ڼ���,���Ժ�...",
        "δ��⵽DS18B20!",
        "���ÃȲ��¶ȴ�����...",
    },
    {
        "Loading...",
        "DS18B20 Check Failed!",
        "Use STM32H7 Temp Sensor...",
    },
};

/**
 * @brief       ���³�ʼ������
 * @param       alarmx          : ���ӽṹ��
 * @param       calendarx       : �����ṹ��
 * @retval      ��
 */
void calendar_alarm_init(_alarm_obj *alarmx, _calendar_obj *calendarx)
{
    uint8_t temp;
    calendar_get_date(calendarx);       /* ��ȡ��ǰ������Ϣ */

    if (calendarx->week == 7)temp = 1 << 0;
    else temp = 1 << calendarx->week;

    if (alarmx->weekmask & temp)        /* ��Ҫ���� */
    {
        rtc_set_alarma(calendarx->week, alarmx->hour, alarmx->min, 0);  /* ��������ʱ�� */
    }
}

/**
 * @brief       ����������
 * @param       type            : ��������
 *                                0,��.
 *                                1,��.��.
 *                                2,��.��.��
 *                                3,��.��.��.��
 * @retval      ��
 */
void calendar_alarm_ring(uint8_t type)
{
    uint8_t i;

    for (i = 0; i < (type + 1); i++)
    {
        pcf8574_write_bit(BEEP_IO,0);
        delay_ms(50);
        pcf8574_write_bit(BEEP_IO,1);
        delay_ms(70);
    }
}

/**
 * @brief       �õ�ʱ��
 * @param       calendarx       : �����ṹ��
 * @retval      ��
 */
void calendar_get_time(_calendar_obj *calendarx)
{
    uint8_t ampm;
    rtc_get_time(&calendarx->hour, &calendarx->min, &calendarx->sec, &ampm); /* �õ�ʱ�� */
}

/**
 * @brief       �õ�����
 * @param       calendarx       : �����ṹ��
 * @retval      ��
 */
void calendar_get_date(_calendar_obj *calendarx)
{
    uint8_t year;
    rtc_get_date(&year, &calendarx->month, &calendarx->date, &calendarx->week);
    calendar.year = year + 2000; /* ��2000�꿪ʼ���� */
}

/**
 * @brief       ���ݵ�ǰ������,����������
 * @param       ��
 * @retval      ��
 */
void calendar_date_refresh(void)
{
    uint8_t weekn;   /* �ܼĴ� */
    uint16_t offx = (lcddev.width - 240) / 2;

    /* ��ʾ���������� */
    g_back_color = BLACK;
    
    lcd_show_xnum(offx + 5, OTHER_TOPY + 9, (calendar.year / 100) % 100, 2, 16, 0, 0XF81F);     /* ��ʾ��  20/19 */
    lcd_show_xnum(offx + 21, OTHER_TOPY + 9, calendar.year % 100, 2, 16, 0, 0XF81F);            /* ��ʾ�� */
    lcd_show_string(offx + 37, OTHER_TOPY + 9, lcddev.width, lcddev.height, 16, "-", 0XF81F);   /* "-" */
    lcd_show_xnum(offx + 45, OTHER_TOPY + 9, calendar.month, 2, 16, 0X80, 0XF81F);              /* ��ʾ�� */
    lcd_show_string(offx + 61, OTHER_TOPY + 9, lcddev.width, lcddev.height, 16, "-", 0XF81F);   /* "-" */
    lcd_show_xnum(offx + 69, OTHER_TOPY + 9, calendar.date, 2, 16, 0X80, 0XF81F);               /* ��ʾ�� */
    
    /* ��ʾ�ܼ�? */
    weekn = calendar.week;
    text_show_string(5 + offx, OTHER_TOPY + 35, lcddev.width, lcddev.height, (char *)calendar_week_table[gui_phy.language][weekn], 16, 0, RED); /* ��ʾ�ܼ�? */
}

/**
 * @brief       ��ȡ����������Ϣ
 *  @note       �������ݱ�����:SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings)+sizeof(_es8388_obj)
 * @param       alarm           : ������Ϣ
 * @retval      ��
 */
void calendar_read_para(_alarm_obj *alarm)
{
    at24cxx_read(SYSTEM_PARA_SAVE_BASE + sizeof(_system_setings) + sizeof(_es8388_obj), (uint8_t *)alarm, sizeof(_alarm_obj));
}

/**
 * @brief       д������������Ϣ
 * @param       alarm           : ������Ϣ
 * @retval      ��
 */
void calendar_save_para(_alarm_obj *alarm)
{
    OS_CPU_SR cpu_sr = 0;
    alarm->ringsta &= 0X7F;     /* ������λ */
    OS_ENTER_CRITICAL();        /* �����ٽ���(�޷����жϴ��) */
    at24cxx_write(SYSTEM_PARA_SAVE_BASE + sizeof(_system_setings) + sizeof(_es8388_obj), (uint8_t *)alarm, sizeof(_alarm_obj));
    OS_EXIT_CRITICAL();         /* �˳��ٽ���(���Ա��жϴ��) */
}

/**
 * @brief       ���崦��(�ߴ�:200*160)
 * @param       x,y             : ��ʼ����
 * @retval      ������
 */
uint8_t calendar_alarm_msg(uint16_t x, uint16_t y)
{
    uint8_t rval = 0;
    uint8_t res = 0;
    uint32_t abr;
    FIL *falarm = 0;
    uint16_t tempcolor = gui_phy.back_color;    /* ����֮ǰ�ı���ɫ */
    _window_obj *twin = 0;                      /* ���� */
    _btn_obj *rbtn = 0;     /* ȡ����ť */
    _btn_obj *okbtn = 0;    /* ȷ����ť */
    uint8_t freadflag = 0;
    uint8_t dcmiflag = 0;

    if (DCMI->CR & 0X01)    /* ����ͷ���ڹ���? */
    {
        dcmiflag = 1;
        dcmi_stop();        /* �ر�����ͷ */
        sw_sdcard_mode();   /* �л�ΪSD��ģʽ */
    }

    OSTaskSuspend(6);       /* ���������� */
    twin = window_creat(x, y, 200, 160, 0, 1 | 1 << 5 | 1 << 6, 16);/* ��������,��ȡ����ɫ */
    okbtn = btn_creat(x + 20, y + 120, 70, 30, 0, 0x02);            /* ������ť */
    rbtn = btn_creat(x + 20 + 70 + 20, y + 120, 70, 30, 0, 0x02);   /* ������ť */
    falarm = (FIL *)gui_memin_malloc(sizeof(FIL));  /* ����FIL�ֽڵ��ڴ����� */

    if (twin == NULL || rbtn == NULL || okbtn == NULL || falarm == NULL)rval = 1;
    else
    {
        /* ���ڵ����ֺͱ���ɫ */
        twin->caption = (uint8_t *)calendar_alarm_caption_table[gui_phy.language];
        twin->windowbkc = APP_WIN_BACK_COLOR;
        /* ���ذ�ť����ɫ */
        rbtn->bkctbl[0] = 0X8452;   /* �߿���ɫ */
        rbtn->bkctbl[1] = 0XAD97;   /* ��һ�е���ɫ */
        rbtn->bkctbl[2] = 0XAD97;   /* �ϰ벿����ɫ */
        rbtn->bkctbl[3] = 0X8452;   /* �°벿����ɫ */
        okbtn->bkctbl[0] = 0X8452;  /* �߿���ɫ */
        okbtn->bkctbl[1] = 0XAD97;  /* ��һ�е���ɫ */
        okbtn->bkctbl[2] = 0XAD97;  /* �ϰ벿����ɫ */
        okbtn->bkctbl[3] = 0X8452;  /* �°벿����ɫ */

        rbtn->caption = (uint8_t *)GUI_CANCEL_CAPTION_TBL[gui_phy.language];        /* ȡ����ť */
        okbtn->caption = (uint8_t *)calendar_alarm_realarm_table[gui_phy.language]; /* ���찴ť */

        if (asc2_s6030 == 0)        /* ֮ǰ��û���ֿ��? */
        {
            freadflag = 1;          /* ��Ƕ�ȡ���ֿ� */
            res = f_open(falarm, (const TCHAR *)APP_ASCII_S6030, FA_READ);          /* ���ļ� */

            if (res == 0)
            {
                asc2_s6030 = (uint8_t *)gui_memex_malloc(falarm->obj.objsize);      /* Ϊ�����忪�ٻ����ַ */

                if (asc2_s6030 == 0)rval = 1;
                else
                {
                    res = f_read(falarm, asc2_s6030, falarm->obj.objsize, (UINT *)&abr);    /* һ�ζ�ȡ�����ļ� */
                }

                f_close(falarm);
            }
        }

        window_draw(twin);  /* �������� */
        btn_draw(rbtn);     /* ����ť */
        btn_draw(okbtn);    /* ����ť */

        if (res)rval = res;
        else    /* ��ʾ����ʱ�� */
        {
            gui_phy.back_color = APP_WIN_BACK_COLOR;
            gui_show_num(x + 15, y + 32 + 14, 2, BLUE, 60, alarm.hour, 0X80);   /* ��ʾʱ */
            gui_show_ptchar(x + 15 + 60, y + 32 + 14, lcddev.width, lcddev.height, 0, BLUE, 60, ':', 0);    /* ":" */
            gui_show_num(x + 15 + 90, y + 32 + 14, 2, BLUE, 60, alarm.min, 0X80);   /* ��ʾ�� */
        }

        system_task_return = 0;

        while (rval == 0)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* �õ�������ֵ */
            delay_ms(5);

            if (system_task_return)
            {
                rval = 1;   /* ȡ�� */
                break;  /* TPAD���� */
            }

            res = btn_check(rbtn, &in_obj); /* ȡ����ť��� */

            if (res && ((rbtn->sta & 0X80) == 0))   /* ����Ч���� */
            {
                rval = 1;
                break;/* �˳� */
            }

            res = btn_check(okbtn, &in_obj);    /* ���찴ť��� */

            if (res && ((okbtn->sta & 0X80) == 0))  /* ����Ч���� */
            {
                rval = 0XFF;
                break;      /* �˳� */
            }
        }
    }
    
    alarm.ringsta &= ~(1 << 7); /* ȡ������ */

    if (rval == 0XFF)   /* �Ժ����� */
    {
        alarm.min += 5; /* �Ƴ�5���� */

        if (alarm.min > 59)
        {
            alarm.min = alarm.min % 60;
            alarm.hour++;

            if (alarm.hour > 23)alarm.hour = 0;
        }

        calendar_alarm_init((_alarm_obj *)&alarm, &calendar); /* ���³�ʼ������ */
    }

    window_delete(twin);
    btn_delete(rbtn);
    btn_delete(okbtn);
    gui_memin_free(falarm);

    if (freadflag)   /* ��ȡ���ֿ�?�ͷ� */
    {
        gui_memex_free(asc2_s6030);
        asc2_s6030 = 0;
    }

    system_task_return = 0; /* ȡ��TPAD */
    gui_phy.back_color = tempcolor; /* �ָ�����ɫ */
    OSTaskResume(6);    /* �ָ������� */

    if (dcmiflag)
    {
        sw_ov5640_mode();
        dcmi_start();   /* ������������ͷ */
    }

    return rval;
}

/**
 * @brief       ��Բ��ָ�����
 * @param       x,y             : �������ĵ�
 * @param       size            : ���̴�С(ֱ��)
 * @param       d               : ���̷ָ�,���ӵĸ߶�
 * @retval      ��
 */
void calendar_circle_clock_drawpanel(uint16_t x, uint16_t y, uint16_t size, uint16_t d)
{
    uint16_t r = size / 2; /* �õ��뾶 */
    uint16_t sx = x - r;
    uint16_t sy = y - r;
    uint16_t px0, px1;
    uint16_t py0, py1;
    uint16_t i;
    gui_fill_circle(x, y, r, WHITE);        /* ����Ȧ */
    gui_fill_circle(x, y, r - 4, BLACK);    /* ����Ȧ */

    for (i = 0; i < 60; i++)   /* �����Ӹ� */
    {
        px0 = sx + r + (r - 4) * sin((app_pi / 30) * i);
        py0 = sy + r - (r - 4) * cos((app_pi / 30) * i);
        px1 = sx + r + (r - d) * sin((app_pi / 30) * i);
        py1 = sy + r - (r - d) * cos((app_pi / 30) * i);
        gui_draw_bline1(px0, py0, px1, py1, 0, WHITE);
    }

    for (i = 0; i < 12; i++)   /* ��Сʱ�� */
    {
        px0 = sx + r + (r - 5) * sin((app_pi / 6) * i);
        py0 = sy + r - (r - 5) * cos((app_pi / 6) * i);
        px1 = sx + r + (r - d) * sin((app_pi / 6) * i);
        py1 = sy + r - (r - d) * cos((app_pi / 6) * i);
        gui_draw_bline1(px0, py0, px1, py1, 2, YELLOW);
    }

    for (i = 0; i < 4; i++)   /* ��3Сʱ�� */
    {
        px0 = sx + r + (r - 5) * sin((app_pi / 2) * i);
        py0 = sy + r - (r - 5) * cos((app_pi / 2) * i);
        px1 = sx + r + (r - d - 3) * sin((app_pi / 2) * i);
        py1 = sy + r - (r - d - 3) * cos((app_pi / 2) * i);
        gui_draw_bline1(px0, py0, px1, py1, 2, YELLOW);
    }

    gui_fill_circle(x, y, d / 2, WHITE);    /* ������Ȧ */
}

/**
 * @brief       ��Բ��ָ�����
 * @param       x,y             : �������ĵ�
 * @param       size            : ���̴�С(ֱ��)
 * @param       d               : ���̷ָ�,���ӵĸ߶�
 * @param       hour            : ʱ��
 * @param       min             : ����
 * @param       sec             : ����
 * @retval      ��
 */
void calendar_circle_clock_showtime(uint16_t x, uint16_t y, uint16_t size, uint16_t d, uint8_t hour, uint8_t min, uint8_t sec)
{
    static uint8_t oldhour = 0; /* ���һ�ν���ú�����ʱ������Ϣ */
    static uint8_t oldmin = 0;
    static uint8_t oldsec = 0;
    float temp;
    uint16_t r = size / 2;      /* �õ��뾶 */
    uint16_t sx = x - r;
    uint16_t sy = y - r;
    uint16_t px0, px1;
    uint16_t py0, py1;
    uint8_t r1;

    if (hour > 11)hour -= 12;

    /* ���Сʱ */
    r1 = d / 2 + 4;
    /* �����һ�ε����� */
    temp = (float)oldmin / 60;
    temp += oldhour;
    px0 = sx + r + (r - 3 * d - 7) * sin((app_pi / 6) * temp);
    py0 = sy + r - (r - 3 * d - 7) * cos((app_pi / 6) * temp);
    px1 = sx + r + r1 * sin((app_pi / 6) * temp);
    py1 = sy + r - r1 * cos((app_pi / 6) * temp);
    gui_draw_bline1(px0, py0, px1, py1, 2, BLACK);
    
    /* ������� */
    r1 = d / 2 + 3;
    temp = (float)oldsec / 60;
    temp += oldmin;
    /* �����һ�ε����� */
    px0 = sx + r + (r - 2 * d - 7) * sin((app_pi / 30) * temp);
    py0 = sy + r - (r - 2 * d - 7) * cos((app_pi / 30) * temp);
    px1 = sx + r + r1 * sin((app_pi / 30) * temp);
    py1 = sy + r - r1 * cos((app_pi / 30) * temp);
    gui_draw_bline1(px0, py0, px1, py1, 1, BLACK);
    
    /* ������� */
    r1 = d / 2 + 3;
    /* �����һ�ε����� */
    px0 = sx + r + (r - d - 7) * sin((app_pi / 30) * oldsec);
    py0 = sy + r - (r - d - 7) * cos((app_pi / 30) * oldsec);
    px1 = sx + r + r1 * sin((app_pi / 30) * oldsec);
    py1 = sy + r - r1 * cos((app_pi / 30) * oldsec);
    gui_draw_bline1(px0, py0, px1, py1, 0, BLACK);
    
    /* ��ʾСʱ */
    r1 = d / 2 + 4;
    /* ��ʾ�µ�ʱ�� */
    temp = (float)min / 60;
    temp += hour;
    px0 = sx + r + (r - 3 * d - 7) * sin((app_pi / 6) * temp);
    py0 = sy + r - (r - 3 * d - 7) * cos((app_pi / 6) * temp);
    px1 = sx + r + r1 * sin((app_pi / 6) * temp);
    py1 = sy + r - r1 * cos((app_pi / 6) * temp);
    gui_draw_bline1(px0, py0, px1, py1, 2, YELLOW);
    
    /* ��ʾ���� */
    r1 = d / 2 + 3;
    temp = (float)sec / 60;
    temp += min;
    /* ��ʾ�µķ��� */
    px0 = sx + r + (r - 2 * d - 7) * sin((app_pi / 30) * temp);
    py0 = sy + r - (r - 2 * d - 7) * cos((app_pi / 30) * temp);
    px1 = sx + r + r1 * sin((app_pi / 30) * temp);
    py1 = sy + r - r1 * cos((app_pi / 30) * temp);
    gui_draw_bline1(px0, py0, px1, py1, 1, GREEN);
    
    /* ��ʾ���� */
    r1 = d / 2 + 3;
    /* ��ʾ�µ����� */
    px0 = sx + r + (r - d - 7) * sin((app_pi / 30) * sec);
    py0 = sy + r - (r - d - 7) * cos((app_pi / 30) * sec);
    px1 = sx + r + r1 * sin((app_pi / 30) * sec);
    py1 = sy + r - r1 * cos((app_pi / 30) * sec);
    gui_draw_bline1(px0, py0, px1, py1, 0, RED);
    oldhour = hour;	/* ����ʱ */
    oldmin = min;   /* ����� */
    oldsec = sec;   /* ������ */
}

/**
 * @brief       ʱ����ʾģʽ
 * @param       ��
 * @retval      0, �����˳�; ����, �������;
 */
uint8_t calendar_play(void)
{
    uint8_t second = 0;
    short temperate = 0;    /* �¶�ֵ */
    uint8_t t = 0;
    uint8_t tempdate = 0;
    uint8_t rval = 0;       /* ����ֵ */
    uint8_t res;
    uint16_t xoff = 0;
    uint16_t yoff = 0;      /* ����yƫ���� */
    uint16_t r = 0;         /* ���̰뾶 */
    uint8_t d = 0;          /* ָ�볤�� */
    uint16_t color = 0X07FF;/* ��ɫ */
    uint32_t br = 0;

    uint8_t TEMP_SEN_TYPE = 0;  /* Ĭ��ʹ��DS18B20 */
    FIL *f_calendar = 0;

    f_calendar = (FIL *)gui_memin_malloc(sizeof(FIL)); /* ����FIL�ֽڵ��ڴ����� */

    if (f_calendar == NULL)rval = 1;    /* ����ʧ�� */
    else
    {
        res = f_open(f_calendar, (const TCHAR *)APP_ASCII_S6030, FA_READ); /* ���ļ� */

        if (res == FR_OK)
        {
            asc2_s6030 = (uint8_t *)gui_memex_malloc(f_calendar->obj.objsize);  /* Ϊ�����忪�ٻ����ַ */

            if (asc2_s6030 == 0)rval = 1;
            else
            {
                res = f_read(f_calendar, asc2_s6030, f_calendar->obj.objsize, (UINT *)&br); /* һ�ζ�ȡ�����ļ� */
            }

            f_close(f_calendar);
        }

        if (res)rval = res;
    }

    if (rval == 0)   /* �޴��� */
    {
        adc3_init(); /* ���³�ʼ���¶ȴ����� */
        lcd_clear(BLACK);       /* ����� */ 
        second = calendar.sec;  /* �õ��˿̵����� */
        text_show_string(48, 60, lcddev.width, lcddev.height, (char *)calendar_loading_str[gui_phy.language][0], 16, 0x01, color); /* ��ʾ������Ϣ */

        if (ds18b20_init())
        {
            text_show_string(48, 76, lcddev.width, lcddev.height, (char *)calendar_loading_str[gui_phy.language][1], 16, 0x01, color);
            delay_ms(500);
            text_show_string(48, 92, lcddev.width, lcddev.height, (char *)calendar_loading_str[gui_phy.language][2], 16, 0x01, color);
            TEMP_SEN_TYPE = 1;
        }

        delay_ms(1100);     /* �ȴ�1.1s */
        g_back_color = BLACK;
        lcd_clear(BLACK);   /* ����� */
        r = lcddev.width / 3;
        d = lcddev.width / 40;
        yoff = (lcddev.height - r * 2 - 140) / 2;
        TIME_TOPY = yoff + r * 2 + 10;
        OTHER_TOPY = TIME_TOPY + 60 + 10;
        xoff = (lcddev.width - 240) / 2;
        calendar_circle_clock_drawpanel(lcddev.width / 2, yoff + r, r * 2, d); /* ��ʾָ��ʱ�ӱ��� */
        calendar_date_refresh();    /* �������� */
        tempdate = calendar.date;   /* �����ݴ��� */
        gui_phy.back_color = BLACK;
        gui_show_ptchar(xoff + 70 - 4, TIME_TOPY, lcddev.width, lcddev.height, 0, color, 60, ':', 0);	/* ":" */
        gui_show_ptchar(xoff + 150 - 4, TIME_TOPY, lcddev.width, lcddev.height, 0, color, 60, ':', 0);	/* ":" */
    }

    while (rval == 0)
    {
        calendar_get_time(&calendar);   /* ����ʱ�� */

        if (system_task_return)
        {
            delay_ms(10);
            
            if (tpad_scan(1)) 
            {
                break;  /* TPAD����,�ٴ�ȷ��,�ų����� */
            }
            else system_task_return = 0;
        }

        if (second != calendar.sec)     /* ���Ӹı��� */
        {
            second = calendar.sec;
            calendar_circle_clock_showtime(lcddev.width / 2, yoff + r, r * 2, d, calendar.hour, calendar.min, calendar.sec); /* ָ��ʱ����ʾʱ�� */
            gui_phy.back_color = BLACK;
            gui_show_num(xoff + 10, TIME_TOPY, 2, color, 60, calendar.hour, 0X80);  /* ��ʾʱ */
            gui_show_num(xoff + 90, TIME_TOPY, 2, color, 60, calendar.min, 0X80);   /* ��ʾ�� */
            gui_show_num(xoff + 170, TIME_TOPY, 2, color, 60, calendar.sec, 0X80);  /* ��ʾ�� */

            if (t % 2 == 0)   /* �ȴ�2���� */
            {
                if (TEMP_SEN_TYPE)temperate = adc3_get_temperature() / 10;   /* �õ��ڲ��¶ȴ������ɼ������¶�,0.1�� */
                else temperate = ds18b20_get_temperature();                 /* �õ�18b20�¶� */

                if (temperate < 0)  /* �¶�Ϊ������ʱ�򣬺�ɫ��ʾ */
                {
                    color = RED;
                    temperate = -temperate; /* ��Ϊ���¶� */
                }
                else color = 0X07FF;        /* ����Ϊ�غ�ɫ������ʾ */

                gui_show_num(xoff + 90, OTHER_TOPY, 2, color, 60, temperate / 10, 0X80);    /* XX */
                gui_show_ptchar(xoff + 150, OTHER_TOPY, lcddev.width, lcddev.height, 0, color, 60, '.', 0); /* "." */
                gui_show_ptchar(xoff + 180 - 15, OTHER_TOPY, lcddev.width, lcddev.height, 0, color, 60, temperate % 10 + '0', 0); /* ��ʾС�� */
                gui_show_ptchar(xoff + 210 - 10, OTHER_TOPY, lcddev.width, lcddev.height, 0, color, 60, 95 + ' ', 0);   /* ��ʾ�� */

                if (t > 0)t = 0;
            }

            calendar_get_date(&calendar);   /* �������� */

            if (calendar.date != tempdate)
            {
                calendar_date_refresh();    /* �����仯��,�������� */
                tempdate = calendar.date;   /* �޸�tempdate����ֹ�ظ����� */
            }

            t++;
        }

        delay_ms(20);
    };

    while (tp_dev.sta & TP_PRES_DOWN)tp_dev.scan(0);    /* �ȴ�TP�ɿ� */

    gui_memex_free(asc2_s6030); /* ɾ��������ڴ� */
    asc2_s6030 = 0;             /* ���� */
    gui_memin_free(f_calendar); /* ɾ��������ڴ� */
    g_point_color = BLUE;
    g_back_color = WHITE ;
    return rval;
}




















