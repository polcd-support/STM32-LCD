/**
 ****************************************************************************************************
 * @file        spb.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       SPBЧ��ʵ�� ����
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

#include "./PICTURE/piclib.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./MALLOC/malloc.h"
#include "./TEXT/text.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/SPBLCD/spblcd.h"
#include "./BSP/GSM/gsm.h"
#include "./BSP/LCD/ltdc.h"
#include "spb.h"
#include "string.h"
#include "common.h"
#include "calendar.h"
 
 
/* SPB ������ */
m_spb_dev spbdev;

uint8_t *const SPB_REMIND_MSG = "SPB Updating... Please Wait...";

/* ����ͼ·��,���ݲ�ͬ��lcdѡ��ͬ��·�� */
uint8_t*const spb_bkpic_path_tbl[6]=
{ 
    "1:/SYSTEM/SPB/BACKPIC/840224.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/952384.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/1120364.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/1680610.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/2100834.jpg", 
    "1:/SYSTEM/SPB/BACKPIC/28001090.jpg", 
};

/* ����ͼ���·���� */
uint8_t*const spb_icos_path_tbl[3][SPB_ICOS_NUM]=
{
    { 
        "1:/SYSTEM/SPB/ICOS/ebook_56.bmp",
        "1:/SYSTEM/SPB/ICOS/picture_56.bmp",
        "1:/SYSTEM/SPB/ICOS/music_56.bmp",
        "1:/SYSTEM/SPB/ICOS/video_56.bmp",
        "1:/SYSTEM/SPB/ICOS/clock_56.bmp",
        "1:/SYSTEM/SPB/ICOS/set_56.bmp",
        "1:/SYSTEM/SPB/ICOS/game_56.bmp",
        "1:/SYSTEM/SPB/ICOS/notepad_56.bmp",
        "1:/SYSTEM/SPB/ICOS/setup_56.bmp",
        "1:/SYSTEM/SPB/ICOS/paint_56.bmp",
        "1:/SYSTEM/SPB/ICOS/camera_56.bmp",
        "1:/SYSTEM/SPB/ICOS/recorder_56.bmp",
        "1:/SYSTEM/SPB/ICOS/usb_56.bmp",
        "1:/SYSTEM/SPB/ICOS/earthnet_56.bmp",
        "1:/SYSTEM/SPB/ICOS/wireless_56.bmp",
        "1:/SYSTEM/SPB/ICOS/calc_56.bmp",
        "1:/SYSTEM/SPB/ICOS/qrcode_56.bmp",
        "1:/SYSTEM/SPB/ICOS/webcam_56.bmp",
        "1:/SYSTEM/SPB/ICOS/facerec_56.bmp", 
        "1:/SYSTEM/SPB/ICOS/9dtest_56.bmp",	
        "1:/SYSTEM/SPB/ICOS/grad_56.bmp",
        "1:/SYSTEM/SPB/ICOS/buzzer_56.bmp",	
        "1:/SYSTEM/SPB/ICOS/keytest_56.bmp",
        "1:/SYSTEM/SPB/ICOS/ledtest_56.bmp",
    },
    { 
        "1:/SYSTEM/SPB/ICOS/ebook_70.bmp",
        "1:/SYSTEM/SPB/ICOS/picture_70.bmp",
        "1:/SYSTEM/SPB/ICOS/music_70.bmp",
        "1:/SYSTEM/SPB/ICOS/video_70.bmp",
        "1:/SYSTEM/SPB/ICOS/clock_70.bmp",
        "1:/SYSTEM/SPB/ICOS/set_70.bmp",
        "1:/SYSTEM/SPB/ICOS/game_70.bmp",
        "1:/SYSTEM/SPB/ICOS/notepad_70.bmp",
        "1:/SYSTEM/SPB/ICOS/setup_70.bmp",
        "1:/SYSTEM/SPB/ICOS/paint_70.bmp",
        "1:/SYSTEM/SPB/ICOS/camera_70.bmp",
        "1:/SYSTEM/SPB/ICOS/recorder_70.bmp",
        "1:/SYSTEM/SPB/ICOS/usb_70.bmp",
        "1:/SYSTEM/SPB/ICOS/earthnet_70.bmp",
        "1:/SYSTEM/SPB/ICOS/wireless_70.bmp",
        "1:/SYSTEM/SPB/ICOS/calc_70.bmp",
        "1:/SYSTEM/SPB/ICOS/qrcode_70.bmp",
        "1:/SYSTEM/SPB/ICOS/webcam_70.bmp",
        "1:/SYSTEM/SPB/ICOS/facerec_70.bmp", 
        "1:/SYSTEM/SPB/ICOS/9dtest_70.bmp",	
        "1:/SYSTEM/SPB/ICOS/grad_70.bmp",
        "1:/SYSTEM/SPB/ICOS/buzzer_70.bmp",	
        "1:/SYSTEM/SPB/ICOS/keytest_70.bmp",
        "1:/SYSTEM/SPB/ICOS/ledtest_70.bmp",
    },
    { 
        "1:/SYSTEM/SPB/ICOS/ebook_110.bmp",
        "1:/SYSTEM/SPB/ICOS/picture_110.bmp",
        "1:/SYSTEM/SPB/ICOS/music_110.bmp",
        "1:/SYSTEM/SPB/ICOS/video_110.bmp",
        "1:/SYSTEM/SPB/ICOS/clock_110.bmp",
        "1:/SYSTEM/SPB/ICOS/set_110.bmp",
        "1:/SYSTEM/SPB/ICOS/game_110.bmp",
        "1:/SYSTEM/SPB/ICOS/notepad_110.bmp",
        "1:/SYSTEM/SPB/ICOS/setup_110.bmp",
        "1:/SYSTEM/SPB/ICOS/paint_110.bmp",
        "1:/SYSTEM/SPB/ICOS/camera_110.bmp",
        "1:/SYSTEM/SPB/ICOS/recorder_110.bmp",
        "1:/SYSTEM/SPB/ICOS/usb_110.bmp",
        "1:/SYSTEM/SPB/ICOS/earthnet_110.bmp",
        "1:/SYSTEM/SPB/ICOS/wireless_110.bmp",
        "1:/SYSTEM/SPB/ICOS/calc_110.bmp",
        "1:/SYSTEM/SPB/ICOS/qrcode_110.bmp",
        "1:/SYSTEM/SPB/ICOS/webcam_110.bmp",
        "1:/SYSTEM/SPB/ICOS/facerec_110.bmp", 
        "1:/SYSTEM/SPB/ICOS/9dtest_110.bmp",
        "1:/SYSTEM/SPB/ICOS/grad_110.bmp",
        "1:/SYSTEM/SPB/ICOS/buzzer_110.bmp",
        "1:/SYSTEM/SPB/ICOS/keytest_110.bmp",
        "1:/SYSTEM/SPB/ICOS/ledtest_110.bmp",
    },
};

/* ������ͼ���·���� */
uint8_t*const spb_micos_path_tbl[3][3]=
{ 
    {
        "1:/SYSTEM/SPB/icos/phone_56.bmp",
        "1:/SYSTEM/SPB/icos/app_56.bmp",
        "1:/SYSTEM/SPB/icos/v meter_56.bmp",
    },
    { 
        "1:/SYSTEM/SPB/icos/phone_70.bmp",
        "1:/SYSTEM/SPB/icos/app_70.bmp",
        "1:/SYSTEM/SPB/icos/v meter_70.bmp",
    },
    { 
        "1:/SYSTEM/SPB/icos/phone_110.bmp",
        "1:/SYSTEM/SPB/icos/app_110.bmp",
        "1:/SYSTEM/SPB/icos/v meter_110.bmp",
    },
};

/* ����ͼ��icos�Ķ�Ӧ�������� */
uint8_t*const icos_name_tbl[GUI_LANGUAGE_NUM][SPB_ICOS_NUM]=
{
    { 
        "����ͼ��","�������","���ֲ���","��Ƶ����",
        "ʱ��","ϵͳ����","��Ϸ��","���±�",
        "������","��д����","�����","¼����",
        "USB ����","����ͨ��","���ߴ���","������", 
        "��ά��","IP����ͷ","����ʶ��","9D����",
        "ˮƽ��","������","��������","LED����",
    },
    { 
        "��ӈD��","���a���","��������","ҕ�l����",
        "�r�","ϵ�y�O��","�[��C","ӛ�±�",	 
        "�\����","�֌����P","�����","����C ",
        "USB �B��","�W�jͨ��","�o������","Ӌ����", 
        "���S�a","IP�z���^","��Ę�R�e","9D�yԇ",
        "ˮƽ�x","���Q��","���I�yԇ","LED�yԇ",
    },
    { 
        "EBOOK","PHOTOS","MUSIC","VIDEO",
        "TIME","SETTINGS","GAMES","NOTEPAD",
        "EXE","PAINT","CAMERA","RECODER",
        "USB","ETHERNET","WIRELESS","CALC", 
        "QR CODE","WEB CAM","FACE REC","9D TEST",
        "LEVEL","BUZZER","KEY TEST","LED TEST",
    },
};

/* ��ͼ���Ӧ������ */
uint8_t*const micos_name_tbl[GUI_LANGUAGE_NUM][3]=
{ 
    {
        "����","Ӧ������","��ѹ��",    
    },
    {
        "��̖","��������","늉���",
    },
    {
        "PHONE","APPS","V Meter",
    },
};

/**
 * @brief       ��ʼ��spb��������
 * @param       mode            : 0, ������Ҫ�Զ�ѡ���Ƿ����; 1,ǿ�����¸���;
 * @retval      ��
 */
uint8_t spb_init(uint8_t mode)
{

    uint16_t i;
    uint8_t res = 0;
    uint8_t lcdtype = 0;    /* LCD���� */
    uint8_t icoindex = 0;   /* IOC������� */
    uint16_t icowidth;      /* ͼ��Ŀ�� */
    uint16_t icoxpit;       /* x����ͼ��֮��ļ�� */
    uint16_t micoyoff;

    memset((void *)&spbdev, 0, sizeof(spbdev));
    spbdev.selico = 0xff;

    if (lcddev.width == 240)        /* ����240*320��LCD��Ļ */
    {
        icowidth = 56;
        micoyoff = 4;
        lcdtype = 0;
        icoindex = 0;
        spbdev.stabarheight = 20;
        spbdev.spbheight = 224;
        spbdev.spbwidth = 240;
        spbdev.spbfontsize = 12;
    }
    else if(lcddev.width == 272)    /* ����272*480��LCD��Ļ */
    { 
        icowidth = 56; 
        micoyoff = 4;
        lcdtype = 1;
        icoindex = 0;
        spbdev.stabarheight = 20;
        spbdev.spbheight = 384;
        spbdev.spbwidth =272; 
        spbdev.spbfontsize = 12; 
    }
    else if (lcddev.width == 320)   /* ����320*480��LCD��Ļ */
    {
        icowidth = 70;
        micoyoff = 6;
        lcdtype = 2;
        icoindex = 1;
        spbdev.stabarheight = 24;
        spbdev.spbheight = 364;
        spbdev.spbwidth = 320;
        spbdev.spbfontsize = 12;
    }
    else if (lcddev.width == 480)   /* ����480*800��LCD��Ļ */
    {
        icowidth = 110;
        micoyoff = 18;
        lcdtype = 3;
        icoindex = 2;
        spbdev.stabarheight = 30;
        spbdev.spbheight = 610;
        spbdev.spbwidth = 480;
        spbdev.spbfontsize = 16;
    }
    else if(lcddev.width == 600)      /* ����600*1024��LCD��Ļ */
    { 
        icowidth = 110; 
        micoyoff = 18;
        lcdtype = 4;
        icoindex = 2;
        spbdev.stabarheight = 30;
        spbdev.spbheight = 834;
        spbdev.spbwidth = 600; 
        spbdev.spbfontsize = 16; 
    }
    else if(lcddev.width == 800)     /* ����800*1280��LCD��Ļ */
    { 
        icowidth = 110; 
        micoyoff = 18;
        lcdtype = 5;
        icoindex = 2;
        spbdev.stabarheight = 30;
        spbdev.spbheight = 1090;
        spbdev.spbwidth = 800; 
        spbdev.spbfontsize = 16; 
    }

    icoxpit = (lcddev.width - icowidth * 4) / 4;
    spbdev.spbahwidth = spbdev.spbwidth / 4;

    for (i = 0; i < SPB_ICOS_NUM; i++)
    {
        spbdev.icos[i].width = icowidth;  /* ���� ����ͼƬ�Ŀ�ȳߴ� */
        spbdev.icos[i].height = icowidth + spbdev.spbfontsize + spbdev.spbfontsize / 4;
        spbdev.icos[i].x = icoxpit / 2 + (i % 4) * (icowidth + icoxpit) + (i / 8) * spbdev.spbwidth;
        spbdev.icos[i].y = spbdev.stabarheight + 4 + ((i % 8) / 4) * (spbdev.icos[i].height + icoxpit);
        spbdev.icos[i].path = (uint8_t*)spb_icos_path_tbl[icoindex][i];
        spbdev.icos[i].name = (uint8_t*)icos_name_tbl[gui_phy.language][i];
    }

    for (i = 0; i < 3; i++)
    {
        spbdev.micos[i].x = (spbdev.spbwidth - 3 * icowidth - 2 * icoxpit) / 2 + i * (icowidth + icoxpit);
        spbdev.micos[i].y = spbdev.stabarheight + spbdev.spbheight + micoyoff;
        spbdev.micos[i].width = icowidth;   /* ���� ����ͼƬ�Ŀ�ȳߴ� */
        spbdev.micos[i].height = icowidth + spbdev.spbfontsize + spbdev.spbfontsize / 4;
        spbdev.micos[i].path = (uint8_t *)spb_micos_path_tbl[icoindex][i];
        spbdev.micos[i].name = (uint8_t *)micos_name_tbl[gui_phy.language][i];
    }

    /* ָ��EX SRAM LCD BUF */
    pic_phy.read_point = (uint32_t(*)(uint16_t,uint16_t))slcd_read_point;
    pic_phy.draw_point = (void(*)(uint16_t,uint16_t,uint32_t))slcd_draw_point;
    pic_phy.fillcolor = slcd_fill_color; 
    picinfo.lcdwidth = spbdev.spbwidth * SPB_PAGE_NUM + spbdev.spbahwidth*2;  
    gui_phy.read_point = (uint32_t(*)(uint16_t,uint16_t))slcd_read_point;
    gui_phy.draw_point = (void(*)(uint16_t,uint16_t,uint32_t))slcd_draw_point;
    g_sramlcdbuf = gui_memex_malloc((uint32_t)spbdev.spbheight * spbdev.spbwidth * 7);/* ����3.5֡�����С */
    
    if (g_sramlcdbuf == NULL)
    {
        return 1;/* ���� */
    }
    
    res = piclib_ai_load_picfile(spb_bkpic_path_tbl[lcdtype],0,0,spbdev.spbwidth*SPB_PAGE_NUM + spbdev.spbahwidth * 2,spbdev.spbheight,0);/* ���ر���ͼƬ */
    
    if (res == 0)
    {
        res = spb_load_icos();
    }
    /* ָ��LCD */
    pic_phy.read_point = lcd_read_point;
    pic_phy.draw_point = lcd_draw_point;
    pic_phy.fillcolor = piclib_fill_color;
    picinfo.lcdwidth = lcddev.width;
    gui_phy.read_point = lcd_read_point;
    gui_phy.draw_point = lcd_draw_point;
    spbdev.pos = spbdev.spbahwidth;/* Ĭ���ǵ�1ҳ��ʼλ�� */
    
    return 0;
    
}

/**
 * @brief       ɾ��SPB
 * @param       ��
 * @retval      ��
 */
void spb_delete(void)
{
    memset((void *)&spbdev, 0, sizeof(spbdev));
    gui_memex_free(g_sramlcdbuf);
}

/**
 * @brief       װ��icos
 * @param       ��
 * @retval      0,�ɹ�; ����,�������;
 */
uint8_t spb_load_icos(void)
{
    uint8_t j;
    uint8_t res = 0;

    for (j = 0; j < SPB_ICOS_NUM; j++)
    {
        res = minibmp_decode(spbdev.icos[j].path, spbdev.icos[j].x + spbdev.spbahwidth, spbdev.icos[j].y - spbdev.stabarheight, spbdev.icos[j].width, spbdev.icos[j].width, 0, 0);

        if (res)return 1;

        gui_show_strmid(spbdev.icos[j].x + spbdev.spbahwidth, spbdev.icos[j].y + spbdev.icos[j].width - spbdev.stabarheight, spbdev.icos[j].width, spbdev.spbfontsize, SPB_FONT_COLOR, spbdev.spbfontsize, spbdev.icos[j].name); /* ��ʾ���� */
    }

    return 0;
}

/**
 * @brief       װ��������icos
 * @param       frame           : ֡���
 * @retval      0,�ɹ�; ����,�������;
 */
uint8_t spb_load_micos(void)
{
    uint8_t j;
    uint8_t res = 0;
    gui_fill_rectangle(0, spbdev.stabarheight + spbdev.spbheight, lcddev.width, lcddev.height - spbdev.stabarheight - spbdev.spbheight, SPB_MICO_BKCOLOR);

    for (j = 0; j < 3; j++)
    {
        res = minibmp_decode(spbdev.micos[j].path, spbdev.micos[j].x, spbdev.micos[j].y, spbdev.micos[j].width, spbdev.micos[j].width, 0, 0);

        if (res)return 1;

        gui_show_strmid(spbdev.micos[j].x, spbdev.micos[j].y + spbdev.micos[j].width, spbdev.micos[j].width, spbdev.spbfontsize, SPB_FONT_COLOR, spbdev.spbfontsize, spbdev.micos[j].name); /* ��ʾ���� */
    }

    return 0;
}

/* SD��ͼ�� */
/* PCtoLCD2002ȡģ��ʽ:����,����ʽ,˳�� */
const uint8_t SPB_SD_ICO[60] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F,
    0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFC,
    0x00, 0x0F, 0xFE, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFC, 0x00, 0x0A, 0xAC, 0x00,
    0x0A, 0xAC, 0x00, 0x0A, 0xAC, 0x00, 0x0F, 0xFC, 0x00, 0x00, 0x00, 0x00,
};

/* USB ͼ��, ȡģ��ʽ���� */
const uint8_t SPB_USB_ICO[60] =
{
    0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xF0, 0x00, 0x01, 0xF8, 0x00, 0x00, 0x60, 0x00, 0x00,
    0x67, 0x00, 0x04, 0x67, 0x00, 0x0E, 0x62, 0x00, 0x0E, 0x62, 0x00, 0x04, 0x62, 0x00, 0x04, 0x7C,
    0x00, 0x06, 0x60, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00,
    0x00, 0xF0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
};

/**
 * @brief       ��ʾgsm�ź�ǿ��,ռ��20*20���ش�С
 * @param       x,y             : ��ʼ����
 * @param       signal          : �ź�ǿ��,��Χ:0~30
 * @retval      ��
 */
void spb_gsm_signal_show(uint16_t x, uint16_t y, uint8_t signal)
{
    uint8_t t;
    uint16_t color;
    signal /= 5;

    if (signal > 5)signal = 5;

    for (t = 0; t < 5; t++)
    {
        if (signal)   /* ���ź� */
        {
            signal--;
            color = WHITE;
        }
        else color = 0X6B4D;   /* ���ź� */

        gui_fill_rectangle(x + 1 + t * 4, y + 20 - 6 - t * 3, 3, (t + 1) * 3, color);
    }
}

/* GSMģ�K��ʾ��Ϣ */
uint8_t *const spb_gsma_msg[GUI_LANGUAGE_NUM][3] =
{
    "���ƶ���", "�й��ƶ�", "�й���ͨ",
    "�o�ƄӾW", "�Ї��Ƅ�", "�Ї�ͨ",
    " NO NET ", "CHN Mobi", "CHN Unic",
};

/**
 * @brief       ���¶�����Ϣ����Ϣ����
 * @param       clr             : 0,���������; 1,�������;
 * @retval      ��
 */
void spb_stabar_msg_show(uint8_t clr)
{
    uint8_t temp;

    if (clr)
    {
        gui_fill_rectangle(0, 0, lcddev.width, spbdev.stabarheight, BLACK);
        gui_show_string("CPU:  %", 24 + 64 + 20 + 2 + 20 + 2, (spbdev.stabarheight - 16) / 2, 64, 16, 16, WHITE);   /* ��ʾCPU���� */
        gui_show_string(":", lcddev.width - 42 + 16, (spbdev.stabarheight - 16) / 2, 8, 16, 16, WHITE);             /* ��ʾ':' */
    }

    /* GSM��Ϣ��ʾ */
    spb_gsm_signal_show(2, (spbdev.stabarheight - 20) / 2, gsmdev.csq); /* ��ʾ�ź����� */
    gui_fill_rectangle(2 + 20 + 2, (spbdev.stabarheight - 16) / 2, 64, 16, BLACK);

    if (gsmdev.status & (1 << 5))
    {
        if (gsmdev.status & (1 << 4))temp = 2;
        else temp = 1;

        gui_show_string(spb_gsma_msg[gui_phy.language][temp], 2 + 20 + 2, (spbdev.stabarheight - 16) / 2, 64, 16, 16, WHITE); /* ��ʾ��Ӫ������ */
    }
    else gui_show_string(spb_gsma_msg[gui_phy.language][0], 2 + 20 + 2, (spbdev.stabarheight - 16) / 2, 64, 16, 16, WHITE);   /* ��ʾ��Ӫ������ */

    /* ����SD����Ϣ */
    if (gui_phy.memdevflag & (1 << 0))app_show_mono_icos(24 + 64 + 2, (spbdev.stabarheight - 20) / 2, 20, 20, (uint8_t *)SPB_SD_ICO, WHITE, BLACK);
    else gui_fill_rectangle(24 + 64 + 2, 0, 20, spbdev.stabarheight, BLACK);

    /* ����U����Ϣ */
    if (gui_phy.memdevflag & (1 << 3))app_show_mono_icos(24 + 64 + 2 + 20, (spbdev.stabarheight - 20) / 2, 20, 20, (uint8_t *)SPB_USB_ICO, WHITE, BLACK);
    else gui_fill_rectangle(24 + 64 + 2 + 20, 0, 20, spbdev.stabarheight, BLACK);

    /* ��ʾCPUʹ���� */
    gui_phy.back_color = BLACK;
    temp = OSCPUUsage;

    if (OSCPUUsage > 99)temp = 99; /* �����ʾ��99% */

    gui_show_num(24 + 64 + 2 + 20 + 20 + 2 + 32, (spbdev.stabarheight - 16) / 2, 2, WHITE, 16, temp, 0);    /* ��ʾCPUʹ���� */
    
    /* ��ʾʱ�� */
    calendar_get_time(&calendar);
    gui_show_num(lcddev.width - 42, (spbdev.stabarheight - 16) / 2, 2, WHITE, 16, calendar.hour, 0X80);     /* ��ʾʱ�� */
    gui_show_num(lcddev.width - 2 - 16, (spbdev.stabarheight - 16) / 2, 2, WHITE, 16, calendar.min, 0X80);  /* ��ʾ���� */
}

extern uint8_t *const sysset_remindmsg_tbl[2][GUI_LANGUAGE_NUM];

/**
 * @brief       ����SPB������UI
 * @param       ��
 * @retval      0,�ɹ�; ����,�������;
 */
uint8_t spb_load_mui(void)
{
    uint8_t res = 0;
    
    if (spbdev.spbheight == 0 && spbdev.spbwidth == 0)
    {
        lcd_clear(BLACK);   /* ���� */
        window_msg_box((lcddev.width - 220) / 2, (lcddev.height - 100) / 2, 220, 100, (uint8_t *)sysset_remindmsg_tbl[1][gui_phy.language], (uint8_t *)sysset_remindmsg_tbl[0][gui_phy.language], 12, 0, 0, 0); /* ��ʾ��ʾ��Ϣ */
        res = spb_init(0);  /* ���³�ʼ�� */
    }
    
    if(res == 0)
    {
        spb_stabar_msg_show(1);         /* ��ʾ״̬����Ϣ,������к���ʾ */
        slcd_frame_show(spbdev.pos);    /* ��ʾ������icos */
        res = spb_load_micos();         /* ������ͼ�� */
    }
    return res;
}

/**
 * @brief       �ƶ���Ļ
 * @param       dir             : ����,0:����;1,����
 * @param       skips           : ÿ����Ծ����
 * @param       pos             : ��ʼλ��
 * @retval      ��
 */
void spb_frame_move(uint8_t dir, uint8_t skips, uint16_t pos)
{
    uint8_t i;
    uint16_t x;
    uint16_t endpos = spbdev.spbahwidth;

    for (i = 0; i < SPB_PAGE_NUM; i++)  /* �õ��յ�λ�� */
    {
        if (dir == 0)   /* ���� */
        {
            if (pos <= (spbdev.spbwidth * i + spbdev.spbahwidth))
            {
                endpos = spbdev.spbwidth * i + spbdev.spbahwidth;
                break;
            }
        }
        else    /* ���� */
        {
            if (pos >= (spbdev.spbwidth * (SPB_PAGE_NUM - i - 1) + spbdev.spbahwidth))
            {
                endpos = spbdev.spbwidth * (SPB_PAGE_NUM - i - 1) + spbdev.spbahwidth;
                break;
            }
        }
    }

    if (dir)    /* ��Ļ���� */
    {
        for (x = pos; x > endpos;)
        {
            if ((x - endpos) > skips)x -= skips;
            else x = endpos;

            slcd_frame_show(x);
        }
    }
    else        /* ��Ļ���� */
    {
        for (x = pos; x < endpos;)
        {
            x += skips;

            if (x > endpos)x = endpos;

            slcd_frame_show(x);
        }
    }

    spbdev.pos = endpos;
}

/**
 * @brief       ���ĳ��micoͼ���ѡ��״̬
 * @param       selx            : SPB_ICOS_NUM~SPB_ICOS_NUM+2,��ʾ��Ҫ���ѡ��״̬��mico���
 * @retval      ��
 */
void spb_unsel_micos(uint8_t selx)
{
    if(selx >= SPB_ICOS_NUM && selx < (SPB_ICOS_NUM + 3))   /* �Ϸ��ı�� */
    {
        selx -= SPB_ICOS_NUM;
        gui_fill_rectangle(spbdev.micos[selx].x,spbdev.micos[selx].y,spbdev.micos[selx].width,spbdev.micos[selx].height,SPB_MICO_BKCOLOR); 
        minibmp_decode(spbdev.micos[selx].path,spbdev.micos[selx].x,spbdev.micos[selx].y,spbdev.micos[selx].width,spbdev.micos[selx].width,0,0);
        gui_show_strmid(spbdev.micos[selx].x,spbdev.micos[selx].y+spbdev.micos[selx].width,spbdev.micos[selx].width,spbdev.spbfontsize,SPB_FONT_COLOR,spbdev.spbfontsize,spbdev.micos[selx].name);  /* ��ʾ���� */
    }
}

/**
 * @brief       ����ѡ���ĸ�ͼ��
 * @param       sel             : 0~SPB_ICOS_NUM����ǰҳ��ѡ��ico
 * @retval      ��
 */
void spb_set_sel(uint8_t sel)
{
    uint16_t temp = 0;
    slcd_frame_show(spbdev.pos);    /* ˢ�±��� */
    spb_unsel_micos(spbdev.selico); /* �����ͼ��ѡ��״̬ */
    spbdev.selico = sel; 
    
    if (sel < SPB_ICOS_NUM)
    { 
        temp = spbdev.icos[sel].x - spbdev.spbwidth * (sel / 8); 
        gui_alphablend_area(temp,spbdev.icos[sel].y,spbdev.icos[sel].width,spbdev.icos[sel].height,SPB_ALPHA_COLOR,SPB_ALPHA_VAL);
        minibmp_decode(spbdev.icos[sel].path,temp,spbdev.icos[sel].y,spbdev.icos[sel].width,spbdev.icos[sel].width,0,0);
        gui_show_strmid(temp,spbdev.icos[sel].y + spbdev.icos[sel].width,spbdev.icos[sel].width,spbdev.spbfontsize,SPB_FONT_COLOR,spbdev.spbfontsize,spbdev.icos[sel].name);/* ��ʾ���� */
    }
    else
    {
        sel -= SPB_ICOS_NUM;
        gui_alphablend_area(spbdev.micos[sel].x,spbdev.micos[sel].y,spbdev.micos[sel].width,spbdev.micos[sel].height,SPB_ALPHA_COLOR,SPB_ALPHA_VAL); 
        minibmp_decode(spbdev.micos[sel].path,spbdev.micos[sel].x,spbdev.micos[sel].y,spbdev.micos[sel].width,spbdev.micos[sel].width,0,0);
        gui_show_strmid(spbdev.micos[sel].x,spbdev.micos[sel].y + spbdev.micos[sel].width,spbdev.micos[sel].width,spbdev.spbfontsize,SPB_FONT_COLOR,spbdev.spbfontsize,spbdev.micos[sel].name);/* ��ʾ���� */
    }
}

/**
 * @brief       ��Ļ�������������
 * @param       ��
 * @retval      0~17,��˫����ͼ����
 *              0xff,û���κ�ͼ�걻˫�����߰���
 */
uint8_t spb_move_chk(void)
{
    uint8_t i = 0xff;
    uint16_t temp = 0;
    uint16_t movewidth = spbdev.spbwidth * (SPB_PAGE_NUM - 1) + 2 * spbdev.spbahwidth;/* �����ƶ���� */
    uint16_t movecnt = 0;                     /* �õ��������� */
    uint8_t skips = 5;                        /* Ĭ��ÿ����5������ */
    
    switch (lcddev.width)
    {
        case 240:
            skips = 5;
            break;
        case 272:
            skips = 2;
            break; 
        case 320:
            skips = 10;
            break;
        case 480:
            if(lcdltdc.pwidth)skips = 8;      /* RGB��,��8������ */
            else skips = 20;                  /* MCU��,��20������ */
            break;
        case 600:
            skips = 20;
            break; 
        case 800:
            skips = 25;
            break; 
    }
    
    tp_dev.scan(0);                             /* ɨ�� */
    
    if (tp_dev.sta & TP_PRES_DOWN)              /* �а��������� */
    {
        if (spbdev.spbsta&0X8000)               /* �Ѿ�������� */
        {
            movecnt=spbdev.spbsta&0X3FFF;       /* �õ��������� */
            
            if (gui_disabs(spbdev.curxpos,tp_dev.x[0]) >= SPB_MOVE_WIN)/* �ƶ����ڵ���SPB_MOVE_WIN���� */
            {
                if (movecnt < SPB_MOVE_MIN / SPB_MOVE_WIN)
                {
                    spbdev.spbsta++;            /* ��������1 */
                }
            }
        }
        
        spbdev.curxpos = tp_dev.x[0];           /* ��¼��ǰ���� */
        spbdev.curypos = tp_dev.y[0];           /* ��¼��ǰ���� */
        
        if ((spbdev.spbsta & 0X8000) == 0)      /* ������һ�α����� */
        {
            if (spbdev.curxpos > 4096 || spbdev.curypos > 4096)
            {
                return 0XFF;                    /* �Ƿ������� */
            }
            spbdev.spbsta = 0;                  /* ״̬���� */
            spbdev.spbsta |= 1 << 15;           /* ��ǰ��� */
            spbdev.oldxpos = tp_dev.x[0];       /* ��¼����ʱ������ */
            spbdev.oldpos = spbdev.pos;         /* ��¼����ʱ��֡λ�� */
        }
        else if (spbdev.spbsta & 0X4000)        /* �л��� */
        {
            if (spbdev.oldxpos > tp_dev.x[0])   /* x����,��Ļpos���� */
            {
                if(spbdev.pos < movewidth)
                {
                    spbdev.pos += spbdev.oldxpos - tp_dev.x[0]; 
                }
                if(spbdev.pos > movewidth)
                {
                    spbdev.pos = movewidth;     /* ����� */
                }
            }
            else                                /* ����,��Ļpos���� */
            {
                if (spbdev.pos > 0)
                {
                    spbdev.pos -= tp_dev.x[0] - spbdev.oldxpos;
                }
                
                if (spbdev.pos > movewidth)
                {
                    spbdev.pos  = 0;           /* ����� */
                }
            }
            spbdev.oldxpos = tp_dev.x[0];
            slcd_frame_show(spbdev.pos);
        }
        else if((gui_disabs(spbdev.curxpos,spbdev.oldxpos) >= SPB_MOVE_MIN) && (movecnt >= SPB_MOVE_MIN / SPB_MOVE_WIN))/* �������볬��SPB_MOVE_MIN,���Ҽ�⵽����Ч������������SPB_MOVE_MIN/SPB_MOVE_WIN */
        { 
            spbdev.spbsta |= 1 << 14;/* ��ǻ��� */
        }   
    }
    else /* �����ɿ��� */
    { 
        if(spbdev.spbsta & 0x8000)/* ֮ǰ�а��� */
        {
            if(spbdev.spbsta & 0X4000)/* �л��� */
            {
                if(spbdev.pos > spbdev.oldpos)/* ���� */
                {
                    if(((spbdev.pos - spbdev.oldpos) > SPB_MOVE_ACT) && (spbdev.oldpos < (movewidth - spbdev.spbahwidth)))
                    {
                        spb_frame_move(0,skips,spbdev.pos); 
                    }
                    else 
                    {
                        spb_frame_move(1,skips,spbdev.pos); 
                    }
                }
                else/* ���� */
                {
                    if(((spbdev.oldpos-spbdev.pos)>SPB_MOVE_ACT)&&(spbdev.pos>spbdev.spbahwidth))
                    {
                        spb_frame_move(1,skips,spbdev.pos); 
                    }
                    else 
                    {
                        spb_frame_move(0,skips,spbdev.pos); 
                    }
                } 
                spb_unsel_micos(spbdev.selico); /* �����ͼ��ѡ��״̬ */
                spbdev.selico = 0xff;           /* ȡ��spbdev.selicoԭ�ȵ����� */
            }
            else/* ���ڵ㰴 */
            {  
                for (i = 0;i < SPB_ICOS_NUM + 3;i++)    /* ��鰴�µ�ͼ���� */
                {
                    if (i < SPB_ICOS_NUM)
                    {
                        temp = spbdev.curxpos+spbdev.pos - spbdev.spbahwidth;
                        
                        if ((temp > spbdev.icos[i].x) && (temp < spbdev.icos[i].x+spbdev.icos[i].width) && (temp > spbdev.icos[i].x)&&
                           (spbdev.curypos<spbdev.icos[i].y+spbdev.icos[i].height))
                        {
                            break;/* �õ�ѡ�еı�� */
                        }
                    }
                    else
                    {
                        if ((spbdev.curxpos>spbdev.micos[i - SPB_ICOS_NUM].x)&&(spbdev.curxpos < spbdev.micos[i - SPB_ICOS_NUM].x+spbdev.micos[i - SPB_ICOS_NUM].width)&&
                           (spbdev.curypos>spbdev.micos[i - SPB_ICOS_NUM].y)&&(spbdev.curypos < spbdev.micos[i - SPB_ICOS_NUM].y+spbdev.micos[i - SPB_ICOS_NUM].height))
                        {
                            break;/* �õ�ѡ�еı�� */
                        } 
                    }
                }
                if (i < (SPB_ICOS_NUM + 3))/* ��Ч */
                { 
                    if (i != spbdev.selico)/* ѡ���˲�ͬ��ͼ��,�л�ͼ�� */
                    {
                        spb_set_sel(i);
                        i = 0xff;
                    }
                    else
                    {
                        spbdev.selico = 0XFF; /* ������˫��,���¸�λselico */
                    }
                }
                else
                {
                    i = 0XFF;/* ��Ч�ĵ㰴 */
                }
            }
        }
        spbdev.spbsta=0;    /* ��ձ�־ */
        tp_dev.sta=0;       /* ����״̬ */
    } 
    return i;

}
