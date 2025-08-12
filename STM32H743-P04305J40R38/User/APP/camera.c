/**
 ****************************************************************************************************
 * @file        camera.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-30
 * @brief       APP-����� ����
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20221130
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "camera.h"
#include "common.h"
#include "calendar.h"
#include "audioplay.h"
#include "./BSP/OV5640/ov5640.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/KEY/key.h"
#include "qrplay.h"
#include "facereco.h"
#include "calculator.h"
#include "./BSP/LCD/ltdc.h"
#include "./BSP/OV5640/ov5640.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/LED/led.h"
#include "audioplay.h"
#include "./BSP/PCF8574/pcf8574.h"


extern uint8_t g_framecnt;              /* ͳһ��֡������,��timer.c���涨�� */
volatile uint8_t hsync_int = 0;         /* ֡�жϱ�־ */
uint8_t ov_flash = 0;                   /* ����� */
uint8_t bmp_request = 0;                /* bmp��������:0,��bmp��������;1,��bmp��������,��Ҫ��֡�ж�����,�ر�DCMI�ӿ� */
uint8_t ovx_mode = 0;                   /* bit0:0,RGB565ģʽ;1,JPEGģʽ */
uint16_t curline = 0;                   /* ����ͷ�������,��ǰ�б�� */
uint16_t yoffset = 0;                   /* y�����ƫ���� */

#define jpeg_buf_size   4 * 1024 * 1024 /* ����JPEG���ݻ���jpeg_buf�Ĵ�С(4M�ֽ�) */
#define jpeg_line_size  4 * 1024        /* ����DMA��������ʱ,һ�����ݵ����ֵ */

uint32_t *dcmi_line_buf[2];             /* RGB��ʱ,����ͷ����һ��һ�ж�ȡ,�����л��� */
uint32_t *jpeg_data_buf;                /* JPEG���ݻ���buf */

volatile uint32_t jpeg_data_len = 0;    /* buf�е�JPEG��Ч���ݳ��� */
/**
 * JPEG���ݲɼ���ɱ�־
 * 0,����û�вɼ���;
 * 1,���ݲɼ�����,���ǻ�û����;
 * 2,�����Ѿ����������,���Կ�ʼ��һ֡����
 */
volatile uint8_t jpeg_data_ok = 0;


/* ����ͷ��ʾ */
uint8_t *const camera_remind_tbl[4][GUI_LANGUAGE_NUM] =
{
    {"��ʼ��OV5640,���Ժ�...", "��ʼ��OV5640,Ո����...", "OV5640 Init,Please wait...",},
    {"δ��⵽OV5640,����...", "δ�z�y��OV5640,Ո�z��...", "No OV5640 find,Please check...",},
    {"����Ϊ:", "�����:", "SAVE AS:",},
    {"�ֱ���", "�ֱ���", "Resolution",},
};

/* �s����ʾ */
uint8_t *const camera_scalemsg_tbl[2][GUI_LANGUAGE_NUM] =
{
    "1:1��ʾ(������)��", "1:1�@ʾ(�o�s��)��", "1:1 Display(No Scale)��",
    "ȫ�ߴ����ţ�", "ȫ�ߴ�s�ţ�", "Full Scale��",
};

/* JPEGͼƬ 12�ֳߴ� */
uint8_t *const camera_jpegimgsize_tbl[12] = 
{"QQVGA", "QVGA", "VGA", "SVGA", "XGA", "WXGA", "WXGA+", "SXGA", "UXGA", "1080P", "QXGA", "500W"};
/* ���ճɹ���ʾ����� */
uint8_t *const camera_saveok_caption[GUI_LANGUAGE_NUM] =
{
    "���ճɹ���", "���ճɹ���", "Take Photo OK��",
};

/* ����ʧ����ʾ��Ϣ */
uint8_t *const camera_failmsg_tbl[3][GUI_LANGUAGE_NUM] =
{
    "�����ļ�ʧ��,����!", "�����ļ�ʧ��,Ո�z��!", "Creat File Failed,Please check!",
    "�ڴ治��!", "�ȴ治��!", "Out of memory!",
    "���ݴ���(ͼƬ�ߴ�̫��)!", "�����e�`(�DƬ�ߴ�̫��)!", "Data Error(Picture is too big)!",
};

/* 00������ѡ������ */
uint8_t *const camera_l00fun_caption[GUI_LANGUAGE_NUM] =
{
    "�������", "���C�O��", "Camera Set",
};

/* 00������ѡ��� */
uint8_t *const camera_l00fun_table[GUI_LANGUAGE_NUM][6] =
{
    {"��������", "��Ч����", "��������", "ɫ������", "�Աȶ�����", "���������",},
    {"�����O��", "��Ч�O��", "�����O��", "ɫ���O��", "���ȶ��O��", "�W����O��",},
    {"Scene", "Effects", "Brightness", "Saturation", "Contrast", "Flashlight",},
};

/* 10������ѡ��� */
/* ����ģʽ */
uint8_t *const camera_l10fun_table[GUI_LANGUAGE_NUM][5] =
{
    {"�Զ�", "����", "�칫��", "����", "��ͥ",},
    {"�Ԅ�", "����", "�k����", "���", "��ͥ",},
    {"Auto", "Sunny", "Office", "Cloudy", "Home"},
};

/* 11������ѡ��� */
/* ��Ч���� */
uint8_t *const camera_l11fun_table[GUI_LANGUAGE_NUM][7] =
{
    {"����", "��ɫ", "ůɫ", "�ڰ�", "ƫ��", "��ɫ", "ƫ��"},
    {"����", "��ɫ", "ůɫ", "�ڰ�", "ƫ�S", "��ɫ", "ƫ�G"},
    {"Normal", "Cool", "Warm", "W&B", "Yellowish", "Invert", "Greenish",},
};

/* 12~14������ѡ��� */
/* ����-3~3��7��ֵ */
uint8_t *const camera_l124fun_table[GUI_LANGUAGE_NUM][7] =
{
    {"-3", "-2", "-1", "0", "+1", "+2", "+3",},
    {"-3", "-2", "-1", "0", "+1", "+2", "+3",},
    {"-3", "-2", "-1", "0", "+1", "+2", "+3",},
};

/* 15������ѡ��� */
/* ��������� */
uint8_t *const camera_l15fun_table[GUI_LANGUAGE_NUM][2] =
{
    {"�ر�", "��",},
    {"�P�]", "���_",},
    {"OFF", "ON",},
};

/**
 * @brief       ����JPEG����
 *  @note       ���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
 * @param       ��
 * @retval      ��
 */
void jpeg_data_process(void)
{
    uint16_t i;
    uint16_t rlen;  /* ʣ�����ݳ��� */
    uint32_t *pbuf;

    if (qr_dcmi_rgbbuf_sta & 0X02)      /* QRʶ��ģʽ */
    {
        qr_dcmi_curline = 0;            /* �������� */
        qr_dcmi_rgbbuf_sta |= 1 << 0;   /* �������׼������ */
        return ;        /* ֱ�ӷ���,����ִ��ʣ�µĴ��� */
    }

    curline = yoffset;  /* �������� */
    g_framecnt++;

    if (ovx_mode & 0X01)    /* ֻ����JPEG��ʽ��,����Ҫ������ */
    {
        if (jpeg_data_ok == 0)  /* jpeg���ݻ�δ�ɼ���? */
        {
            DMA1_Stream1->CR &= ~(1 << 0);      /* ֹͣ��ǰ���� */

            while (DMA1_Stream1->CR & 0X01);    /* �ȴ�DMA1_Stream1������ */

            rlen = jpeg_line_size - DMA1_Stream1->NDTR;     /* �õ�ʣ�����ݳ��� */
            pbuf = jpeg_data_buf + jpeg_data_len;   /* ƫ�Ƶ���Ч����ĩβ,������� */

            if (DMA1_Stream1->CR & (1 << 19))
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = dcmi_line_buf[1][i]; /* ��ȡbuf1�����ʣ������ */
                }
            }
            else
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = dcmi_line_buf[0][i]; /* ��ȡbuf0�����ʣ������ */
                }
            }

            jpeg_data_len += rlen;  /* ����ʣ�೤�� */
            jpeg_data_ok = 1;       /* ���JPEG���ݲɼ��갴��,�ȴ������������� */
        }

        if (jpeg_data_ok == 2)      /* ��һ�ε�jpeg�����Ѿ��������� */
        {
            DMA1_Stream1->NDTR = jpeg_line_size; /* ���䳤��Ϊjpeg_buf_size*4�ֽ� */
            DMA1_Stream1->CR |= 1 << 0; /* ���´��� */
            jpeg_data_ok = 0;           /* �������δ�ɼ� */
            jpeg_data_len = 0;          /* �������¿�ʼ */
        }
    }
    else
    {
        if (bmp_request == 1)   /* ��bmp��������,�ر�DCMI */
        {
            dcmi_stop();        /* ֹͣDCMI */
            bmp_request = 0;    /* ������������ */
        }

        if (lcdltdc.pwidth == 0)
        {
            lcd_set_cursor(frec_dev.xoff, frec_dev.yoff);
            lcd_write_ram_prepare();    /* ��ʼд��GRAM */
        }

        frec_curline = frec_dev.yoff;
        hsync_int = 1;
    }
}

/**
 * @brief       jpeg���ݽ��ջص�����
 * @param       ��
 * @retval      ��
 */
void jpeg_dcmi_rx_callback(void)
{
    uint16_t i;
    uint32_t *pbuf;

    pbuf = jpeg_data_buf + jpeg_data_len;   /* ƫ�Ƶ���Ч����ĩβ */

    if (DMA1_Stream1->CR & (1 << 19))       /* buf0����,��������buf1 */
    {
        for (i = 0; i < jpeg_line_size; i++)pbuf[i] = dcmi_line_buf[0][i]; /* ��ȡbuf0��������� */

        jpeg_data_len += jpeg_line_size; /* ƫ�� */
    }
    else     /* buf1����,��������buf0 */
    {
        for (i = 0; i < jpeg_line_size; i++)pbuf[i] = dcmi_line_buf[1][i]; /* ��ȡbuf1��������� */

        jpeg_data_len += jpeg_line_size; /* ƫ�� */
    }
}

/**
 * @brief       RGB�����ݽ��ջص�����
 * @param       ��
 * @retval      ��
 */
void rgblcd_dcmi_rx_callback(void)
{  
    uint16_t *pbuf;
    
    if(DMA1_Stream1->CR & (1 << 19))    //DMAʹ��buf1,��ȡbuf0
    { 
        pbuf = (uint16_t*)dcmi_line_buf[0]; 
    }
    else                               //DMAʹ��buf0,��ȡbuf1
    {
        pbuf = (uint16_t*)dcmi_line_buf[1]; 
    }
    
    ltdc_color_fill(0,curline,lcddev.width - 1,curline,pbuf);//DM2D��� 
    
    if(curline < lcddev.height)curline++;
}

/**
 * @brief       �л�ΪOV5640ģʽ
 *   @note      �л�PC8/PC9/PC11ΪDCMI���ù���(AF13)
 * @param       ��
 * @retval      ��
 */
void sw_ov5640_mode(void)
{
    ov5640_write_reg(0X3017, 0XFF); /* ����OV5650���(����������ʾ) */
    ov5640_write_reg(0X3018, 0XFF);
    
    /* GPIOC8/9/11�л�Ϊ DCMI�ӿ� */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 13);  /* PC8 , AF13  DCMI_D2 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 13);  /* PC9 , AF13  DCMI_D3 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 13); /* PC11, AF13  DCMI_D4 */
}

/**
 * @brief       �л�ΪSD��ģʽ
 *   @note      �л�PC8/PC9/PC11ΪSDMMC���ù���(AF12)
 * @param       ��
 * @retval      ��
 */
void sw_sdcard_mode(void)
{
    ov5640_write_reg(0X3017, 0X00); /* �ر�OV5640ȫ�����(��Ӱ��SD��ͨ��) */
    ov5640_write_reg(0X3018, 0X00);
    
    /* GPIOC8/9/11�л�Ϊ SDIO�ӿ� */ 
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 12);  /* PC8 , AF12  SD1_D0 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 12);  /* PC9 , AF12  SD1_D1 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 12); /* PC11, AF12  SD1_D3 */
}
 
/**
 * @brief       �õ��ļ���,��������ʱ������
 *  @note       bmp��ϳ�:����"0:PHOTO/PIC20120321210633.bmp"/"2:PHOTO/PIC20120321210633.bmp"���ļ���
 *              jpg��ϳ�:����"0:PHOTO/PIC20120321210633.jpg"/"2:PHOTO/PIC20120321210633.jpg"���ļ���
 * @param       pname           : ��·��������
 * @param       mode            : 0,����.bmp�ļ�;    1,����.jpg�ļ�;
 * @retval      ��
 */
void camera_new_pathname(uint8_t *pname, uint8_t mode)
{
    calendar_get_time(&calendar);
    calendar_get_date(&calendar);

    if (mode == 0)
    {
        if (gui_phy.memdevflag & (1 << 0))sprintf((char *)pname, "0:PHOTO/PIC%04d%02d%02d%02d%02d%02d.bmp", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);         /* ��ѡ������SD�� */
        else if (gui_phy.memdevflag & (1 << 3))sprintf((char *)pname, "3:PHOTO/PIC%04d%02d%02d%02d%02d%02d.bmp", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);    /* SD��������,�򱣴���U�� */
    }
    else
    {
        if (gui_phy.memdevflag & (1 << 0))sprintf((char *)pname, "0:PHOTO/PIC%04d%02d%02d%02d%02d%02d.jpg", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);         /* ��ѡ������SD�� */
        else if (gui_phy.memdevflag & (1 << 3))sprintf((char *)pname, "3:PHOTO/PIC%04d%02d%02d%02d%02d%02d.jpg", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);    /* SD��������,�򱣴���U�� */
    }
}

/**
 * @brief       OV5640����jpgͼƬ
 * @param       pname           : ��·��������
 * @retval      0, �ɹ�
 *              ����, ʧ��
 */
uint8_t ov5640_jpg_photo(uint8_t *pname)
{
    FIL *f_jpg;
    uint8_t res = 0, headok = 0;
    uint32_t bwr;
    uint32_t i, jpgstart, jpglen;
    uint8_t *pbuf;
    
    uint16_t datasize = 0;          /* ����д�������� */
    uint32_t datalen = 0;           /* ��д�������� */
    uint8_t  *databuf;              /* ���ݻ��棬����ֱ��д�ⲿSRAM���ݵ�SD��������д��������� */
    
    f_jpg = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* ����FIL�ֽڵ��ڴ����� */

    databuf = mymalloc(SRAMIN, 4096);   /* ����4K�ڴ� */

    if (databuf == NULL)
    {
        myfree(SRAMIN, f_jpg);      /* �ͷ�f_jpg�ڴ� */
        return 0XFF;                /* �ڴ�����ʧ�� */
    }

    ovx_mode = 1;
    jpeg_data_ok = 0;
    sw_ov5640_mode();   /* �л�ΪOV5640ģʽ */
    ov5640_jpeg_mode(); /* JPEGģʽ */
    ov5640_outsize_set(16,4,2592,1944);        /* ��������ߴ�(500W) */
    dcmi_rx_callback = jpeg_dcmi_rx_callback;   /* JPEG�������ݻص����� */
    dcmi_dma_init((uint32_t)dcmi_line_buf[0], (uint32_t)dcmi_line_buf[1], jpeg_line_size, 2, 1); /* DCMI DMA���� */
    dcmi_start();   /* �������� */

    while (jpeg_data_ok != 1);  /* �ȴ���һ֡ͼƬ�ɼ��� */

    jpeg_data_ok = 2;   /* ���Ա�֡ͼƬ,������һ֡�ɼ� */
    
    while (jpeg_data_ok != 1);  /* �ȴ��ڶ�֡ͼƬ�ɼ���,�ڶ�֡,�ű��浽SD��ȥ */

    dcmi_stop();    /* ֹͣDMA���� */
    ovx_mode = 0;
    sw_sdcard_mode();   /* �л�ΪSD��ģʽ */
    
    printf("jpeg data size:%d\r\n", jpeg_data_len * 4);   /* ���ڴ�ӡJPEG�ļ���С */
    pbuf = (uint8_t *)jpeg_data_buf;
    jpglen = 0;                                     /* ����jpg�ļ���СΪ0 */
    headok = 0;                                     /* ���jpgͷ��� */
    
    for (i = 0; i < jpeg_data_len * 4; i++)       /* ����0XFF,0XD8��0XFF,0XD9,��ȡjpg�ļ���С */
    {
        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD8))     /* �ҵ�FF D8 */
        {
            jpgstart = i;
            headok = 1;                             /* ����ҵ�jpgͷ(FF D8) */
        }

        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD9) && headok)   /* �ҵ�ͷ�Ժ�,����FF D9 */
        {
            jpglen = i - jpgstart + 2;
            break;
        }
    }

    if (jpglen)                     /* ������jpeg���� */
    {
        res = f_open(f_jpg, (const TCHAR *)pname, FA_WRITE | FA_CREATE_NEW);    /* ģʽ0,���߳��Դ�ʧ��,�򴴽����ļ� */

        if (res == 0)
        {
            pbuf += jpgstart;       /* ƫ�Ƶ�0XFF,0XD8�� */
            
            while(datalen < jpglen) /* ѭ��д�룡����ֱ��д�ⲿSRAM���ݵ�SDIO�������������FIFO������� */
            {
                if((jpglen - datalen) > 4096)
                {
                    datasize = 4096;
                }else
                {
                    datasize = jpglen - datalen;    /* �������� */
                }

                my_mem_copy(databuf, pbuf, datasize);
                res = f_write(f_jpg, databuf, datasize, (UINT *)&bwr); /* д������ */
                pbuf += datasize;
                jpglen -= datasize;

                if (res)break;
            }
        }

        f_close(f_jpg);
    }
    else
    {
        res = 0XFD;
    }
    
    jpeg_data_len = 0;
    sw_ov5640_mode();           /* �л�ΪOV5640ģʽ */
    ov5640_rgb565_mode();       /* RGB565ģʽ */

    if (lcdltdc.pwidth != 0)    /* RGB�� */
    {
        dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB���������ݻص����� */
        dcmi_dma_init((uint32_t)dcmi_line_buf[0], (uint32_t)dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA���� */
    }
    else                                            /* MCU �� */
    {
        dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA����,MCU��,���� */
    }
    
    myfree(SRAMIN, f_jpg);
    myfree(SRAMIN, databuf);

    return res;
}

/**
 * @brief       ����ͷ����
 *  @note       ������Ƭ�ļ�,��������SD��PHOTO�ļ�����.
 * @param       ��
 * @retval      δ�õ�
 */
uint8_t camera_play(void)
{
    uint8_t rval = 0;
    uint8_t res, fac;
    uint8_t *caption = 0;
    uint8_t *pname;
    uint8_t selx = 0;
    uint8_t l00sel = 0, l10sel = 0, l11sel = 0; /* Ĭ��ѡ���� */
    uint8_t l2345sel[3];
    uint8_t *psn;
    uint8_t key;
    uint8_t scale = 1;              /* Ĭ����ȫ�ߴ����� */
    uint8_t tcnt = 0;
    uint16_t outputheight = 0;      /* RGB ͼ������Ŀ�� */
    
    if(g_audiodev.status & (1<<7))      /* ��ǰ�ڷŸ�??����ֹͣ */
    {
        audio_stop_req(&g_audiodev);  /* ֹͣ��Ƶ���� */
        audio_task_delete();        /* ɾ�����ֲ������� */
    }
    
    /* ��ʾ��ʼ���OV5640 */
    window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)camera_remind_tbl[0][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);

    if (gui_phy.memdevflag & (1 << 0))f_mkdir("0:PHOTO"); /* ǿ�ƴ����ļ���,��������� */

    if (gui_phy.memdevflag & (1 << 3))f_mkdir("3:PHOTO"); /* ǿ�ƴ����ļ���,��������� */

    if (ov5640_init())   /* ��ʼ��OV5640 */
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)camera_remind_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
        delay_ms(500);
        rval = 1;
    }

    dcmi_line_buf[0] = gui_memin_malloc(jpeg_line_size * 4);    /* Ϊjpeg dma���������ڴ� */
    dcmi_line_buf[1] = gui_memin_malloc(jpeg_line_size * 4);    /* Ϊjpeg dma���������ڴ� */
    jpeg_data_buf = gui_memex_malloc(jpeg_buf_size);            /* Ϊjpeg�ļ������ڴ�(���4MB) */
    pname = gui_memin_malloc(40);   /* ����40���ֽ��ڴ�,���ڴ���ļ��� */
    psn = gui_memin_malloc(50);     /* ����50���ֽ��ڴ�,���ڴ�����ƣ�������Ϊ:0:PHOTO/PIC20120321210633.bmp"������ʾ�� */

    if (!dcmi_line_buf[1] || !jpeg_data_buf || !pname || !psn)rval = 1; /* ����ʧ�� */

    if (rval == 0)   /* OV5640���� */
    {
        l2345sel[0] = 3;        /* ����Ĭ��Ϊ3,ʵ��ֵ0 */
        l2345sel[1] = 3;        /* ɫ��Ĭ��Ϊ3,ʵ��ֵ0 */
        l2345sel[2] = 3;        /* �Աȶ�Ĭ��Ϊ3,ʵ��ֵ0 */
        ov5640_rgb565_mode();   /* RGB565ģʽ */
        ov5640_focus_init();
        ov5640_focus_constant();/* ���������Խ� */
        ov5640_light_mode(0);   /* �Զ�ģʽ */
        ov5640_brightness(l2345sel[0] + 1);     /* �������� */
        ov5640_color_saturation(l2345sel[1]);   /* ɫ������ */
        ov5640_contrast(l2345sel[2]);           /* �Աȶ����� */
        ov5640_exposure(3);     /* �ع�ȼ� */
        ov5640_sharpness(33);   /* �Զ���� */
        dcmi_init();            /* DCMI���� */
        
        if(lcdltdc.pwidth!=0)   /* RGB�� */
        {
            dcmi_rx_callback = rgblcd_dcmi_rx_callback;   /* RGB���������ݻص����� */
            dcmi_dma_init((uint32_t)dcmi_line_buf[0],(uint32_t)dcmi_line_buf[1],lcddev.width / 2,1,1);    /* DCMI DMA���� */
        }else                   /* MCU �� */
        {
            dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA����,MCU��,���� */
        }

        if (lcddev.height >= 1280)
        {
            yoffset = (lcddev.height - 600) / 2;
            outputheight = 600;
            ov5640_write_reg(0x3035, 0X51);    /* �������֡�ʣ�������ܶ��� */
        }
        else if (lcddev.height > 800)
        {
            yoffset = (lcddev.height - 800) / 2;
            outputheight = 800;
            ov5640_write_reg(0x3035, 0X51);    /* �������֡�ʣ�������ܶ��� */
        }
        else
        {
            yoffset = 0;
            outputheight = lcddev.height;
        }

        curline = yoffset;          /* ������λ */
        ov5640_outsize_set(16, 4, lcddev.width, outputheight); /* ��������ͷ����ߴ�ΪLCD�ĳߴ��С */
        dcmi_start();               /* �������� */
        lcd_clear(BLACK);
        memshow_flag = 0;           /* ��ֹ��ӡ�ڴ���Ϣ */
        system_task_return = 0;     /* ���TPAD */

        while (1)
        {
            tp_dev.scan(0);

            if (tp_dev.sta & TP_PRES_DOWN)
            {
                ov5640_focus_single();  /* ִ���Զ��Խ� */
                tp_dev.scan(0);
                tp_dev.sta = 0;
            }

            key = key_scan(0);    /* ����ɨ�� */

            if (key)
            {
                
                if (key == KEY2_PRES)   /* �����BMP����,��ȴ�300ms,ȥ����,�Ի���ȶ���bmp��Ƭ */
                {
                    delay_ms(300);
                    hsync_int = 0;

                    while (hsync_int == 0); /* �ȴ�֡�ж� */

                    hsync_int = 0;

                    if (ov_flash)ov5640_flash_ctrl(1);  /* ������� */

                    bmp_request = 1;        /* ����ر�DCMI */

                    while (hsync_int == 0);

                    if (ov_flash)ov5640_flash_ctrl(0);  /* �ر������ */

                    while (bmp_request);    /* �ȴ���������� */
                }
                else
                {
                    dcmi_stop();
                }
                
                while (key_scan(1)); /* �ȴ������ɿ� */

                tcnt = 0;

                switch (key)
                {
                    case KEY0_PRES: /* KEY1����,JPEG���� */
                    case KEY2_PRES: /* KEY2����,BMP���� */
                        LED1(0);    /* LED1��,��ʾ������ */
                        sw_sdcard_mode();       /* �л�ΪSD��ģʽ */

                        if (key == KEY0_PRES)   /* JPEG���� */
                        {
                            camera_new_pathname(pname, 1); /* �õ�jpg�ļ��� */
                            res = ov5640_jpg_photo(pname);

                            if (scale == 0)
                            {
                                fac = (float)800 / lcddev.height;     /* �õ��������� */
                                ov5640_outsize_set((1280 - fac * lcddev.width) / 2, (800 - fac * lcddev.height) / 2, lcddev.width, lcddev.height);      
                             }
                            else 
                            {
                                ov5640_outsize_set(16, 4, lcddev.width, outputheight);
                             }
                            if (lcddev.height > 800)
                            {
                                ov5640_write_reg(0x3035, 0X51); /* �������֡�ʣ�������ܶ��� */
                            }
                        }
                        else    /* BMP���� */
                        {
                            camera_new_pathname(pname, 0); /* �õ�bmp�ļ��� */
                            res = bmp_encode(pname, 0, yoffset, lcddev.width, outputheight, 0); /* bmp���� */
                        }

                        if (res)    /* ����ʧ���� */
                        {
                            if (res == 0XFF)window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* �ڴ���� */
                            else if (res == 0XFD)window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[2][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* ���ݴ��� */
                            else window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[0][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* ��ʾSD���Ƿ���� */
                        }
                        else
                        {
                            strcpy((char *)psn, (const char *)camera_remind_tbl[2][gui_phy.language]);
                            strcat((char *)psn, (const char *)pname);
                            window_msg_box((lcddev.width - 180) / 2, (lcddev.height - 80) / 2, 180, 80, psn, (uint8_t *)camera_saveok_caption[gui_phy.language], 12, 0, 0, 0);
                            pcf8574_write_bit(BEEP_IO,0);;        /* �������̽У���ʾ������� */
                            delay_ms(100);
                        }

                        LED1(1);            /* LED1��,��ʾ������� */
                        pcf8574_write_bit(BEEP_IO,1);;            /* �رշ����� */
                        delay_ms(2000);
                        sw_ov5640_mode();   /* �л�ΪOV5640ģʽ */

                        if (scale == 0)
                        {
                            if (lcddev.height == 1280)
                            {
                                fac = 600 / outputheight;   /* �õ��������� */
                                ov5640_outsize_set((1280 - fac * lcddev.width) / 2,(600 - fac * outputheight) / 2,lcddev.width,outputheight);
                            }
                            else
                            {
                                fac = 800 / outputheight;   /* �õ��������� */
                                ov5640_outsize_set((1280 - fac * lcddev.width) / 2,(800 - fac * outputheight) / 2,lcddev.width,outputheight);
                            }
                        }
                        else
                        {
                            ov5640_outsize_set(16, 4, lcddev.width, outputheight);
                        }

                        system_task_return = 0; /* ���TPAD */
                        break;

                    case KEY1_PRES: /* KEY1����,����/1:1��ʾ(������) */
                        scale = !scale;
                    
                        if (scale == 0)
                        {
                            if (lcddev.height == 1280)
                            {
                                fac = 600 / outputheight;   /* �õ��������� */
                                ov5640_outsize_set((1280 - fac * lcddev.width) / 2, (600 - fac * outputheight) / 2, lcddev.width, outputheight);
                            }
                            else
                            {
                                fac = 800 / outputheight;   /* �õ��������� */
                                ov5640_outsize_set((1280 - fac * lcddev.width) / 2, (800 - fac * outputheight) / 2, lcddev.width, outputheight);
                            }
                        }
                        else
                        {
                            OSTaskResume(7);            /* �ָ�usart_task */
                            ov5640_outsize_set(16,4,lcddev.width,outputheight);
                        }
                        window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)camera_scalemsg_tbl[scale][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
                        
                        while (key_scan(1) == 0 && tcnt < 80)/* �ȴ�800ms,���û�а������µĻ� */
                        {
                            delay_ms(10);
                            tcnt++;
                        }
                        break;

                    case WKUP_PRES:         /* ��������ͷ����  */
                        sw_sdcard_mode();   /* �л�ΪSD��ģʽ */
                        caption = (uint8_t *)camera_l00fun_caption[gui_phy.language];
                        res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 6) / 2, 160, 72 + 32 * 6, (uint8_t **)camera_l00fun_table[gui_phy.language], 6, (uint8_t *)&l00sel, 0X90, caption); /* ��ѡ */
                        sw_ov5640_mode();   /* �л�ΪOV5640ģʽ */

                        if (res == 0)
                        {
                            dcmi_start();
                            delay_ms(200);
                            dcmi_stop();
                            sw_sdcard_mode();   /* �л�ΪSD��ģʽ */
                            caption = (uint8_t *)camera_l00fun_table[gui_phy.language][l00sel];

                            switch (l00sel)
                            {
                                case 0:/* �������� */
                                    res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 5) / 2, 160, 72 + 32 * 5, (uint8_t **)camera_l10fun_table[gui_phy.language], 5, (uint8_t *)&l10sel, 0X90, caption); /* ��ѡ */
                                    sw_ov5640_mode();/* �л�ΪOV5640ģʽ */

                                    if (res == 0)
                                    {
                                        ov5640_light_mode(l10sel);
                                    }

                                    break;

                                case 1:/* ��Ч���� */
                                    res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 7) / 2, 160, 72 + 32 * 7, (uint8_t **)camera_l11fun_table[gui_phy.language], 7, (uint8_t *)&l11sel, 0X90, caption); /* ��ѡ */
                                    sw_ov5640_mode();/* �л�ΪOV5640ģʽ */

                                    if (res == 0)
                                    {
                                        ov5640_special_effects(l11sel);
                                    }

                                    break;

                                case 2:/* �������� */
                                case 3:/* ɫ������ */
                                case 4:/* �Աȶ����� */
                                    selx = l2345sel[l00sel - 2]; /* �õ�֮ǰ��ѡ�� */
                                    res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 7) / 2, 160, 72 + 32 * 7, (uint8_t **)camera_l124fun_table[gui_phy.language], 7, (uint8_t *)&selx, 0X90, caption); /* ��ѡ */
                                    sw_ov5640_mode();/* �л�ΪOV5640ģʽ */

                                    if (res == 0)
                                    {
                                        l2345sel[l00sel - 2] = selx; /* ��¼��ֵ */

                                        if (l00sel == 2)ov5640_brightness(selx + 1);    /* �������� */

                                        if (l00sel == 3)ov5640_color_saturation(selx);  /* ɫ������ */

                                        if (l00sel == 4)ov5640_contrast(selx);          /* �Աȶ����� */
                                    }
                                    break;

                                case 5:/* ��������� */
                                    selx = ov_flash;
                                    res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 2) / 2, 160, 72 + 32 * 2, (uint8_t **)camera_l15fun_table[gui_phy.language], 2, (uint8_t *)&selx, 0X90, caption); /* ��ѡ */

                                    if (res == 0)ov_flash = selx;

                                    break;
                            }
                        }

                        break;

                }

                sw_ov5640_mode();   /* �л�ΪOV5640ģʽ */
                dcmi_start();       /* ������ʹ��dcmi,Ȼ�������ر�DCMI,�����ٿ���DCMI,���Է�ֹRGB���Ĳ������� */
                dcmi_stop();
                dcmi_start();
            }

            if (system_task_return)
            {
                if (system_task_return)
                {
                    delay_ms(100);
                    
                    if(tpad_scan(1))
                    {
                        break;         /* TPAD����,�ٴ�ȷ��,�ų����� */
                    }
                    else
                    {
                        system_task_return = 0;
                    }
                }
            }

            if (hsync_int)  /* �ող���֡�ж�,������ʱ */
            {
                delay_ms(10);
                hsync_int = 0;
            }
        }
    }

    dcmi_stop();        /* ֹͣ����ͷ���� */
    sw_sdcard_mode();   /* �л�ΪSD��ģʽ */
    gui_memin_free(dcmi_line_buf[0]);
    gui_memin_free(dcmi_line_buf[1]);
    gui_memex_free(jpeg_data_buf);
    gui_memin_free(pname);
    gui_memin_free(psn);
    memshow_flag = 1;   /* �����ӡ�ڴ���Ϣ */
    return 0;
}
