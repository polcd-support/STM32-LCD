/**
 ****************************************************************************************************
 * @file        usbplay.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       APP-USB���� ����
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

#include "usbplay.h"
#include "usb_app.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"


/* ��nesplay.c���涨�� */
extern uint8_t *const nes_remindmsg_tbl[5][GUI_LANGUAGE_NUM];


/**
 * @brief       USB����
 * @param       ��
 * @retval      0, �����˳�; ����, �������;
 */
uint8_t usb_play(void)
{
    uint8_t rval = 0;       /* ����ֵ */
    uint8_t offline_cnt = 0;
    uint8_t tct = 0;
    uint8_t USB_STA = 0;
    uint8_t busycnt = 0;    /* USBæ������ */
    uint8_t errcnt = 0;     /* USB��������� */

    if (gui_phy.memdevflag & (1 << 3))
    {
        window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)nes_remindmsg_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        while (gui_phy.memdevflag & (1 << 3))
        {
            delay_ms(5);    /* ��ѭ���ȴ�U�̱��γ� */
        }
    }

    g_back_color = LGRAY;
    lcd_clear(g_back_color);
    
    app_filebrower((uint8_t *)APP_MFUNS_CAPTION_TBL[12][gui_phy.language], 0X05); /* ��ʾ���� */
    usbapp_mode_set(USBD_MSC_MODE); /* DEVICE MSC */
    lcd_show_string(60 + (lcddev.width - 240) / 2, 130 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB DisConnected ", BLUE); /* ������ʾUSB������ */

    while (rval == 0)
    {
        delay_ms(1000 / OS_TICKS_PER_SEC); /* ��ʱһ��ʱ�ӽ��� */

        
        if (system_task_return)
        {
            delay_ms(10);
            tpad_scan(1);
            delay_ms(10);
            if(tpad_scan(1))
            {
                if (busycnt) /* USB���ڶ�д */
                {
                    lcd_show_string(60 + (lcddev.width - 240) / 2, 110 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB BUSY!!!", RED); /* ��ʾSD���Ѿ�׼���� */
                    system_task_return = 0;	/* ȡ�� */
                }
                else break; /* USB����,���˳�USB */
                
                break;         /* TPAD����,�ٴ�ȷ��,�ų����� */
            }
            else system_task_return = 0;
        }

        tct++;

        if (tct == 40) /* ÿ40ms����һ�� */
        {
            tct = 0;

            if (busycnt)busycnt--;
            else gui_fill_rectangle(60 + (lcddev.width - 240) / 2, 110 + (lcddev.height - 320) / 2, 100, 16, g_back_color); /* �����ʾ */

            if (errcnt)errcnt--;
            else gui_fill_rectangle(60 + (lcddev.width - 240) / 2, 170 + (lcddev.height - 320) / 2, 128, 16, g_back_color); /* �����ʾ */

            if (usbx.bDeviceState & 0x10) /* ����ѯ���� */
            {
                offline_cnt = 0;                /* USB������,�����offline������ */
                usbx.bDeviceState |= 0X80;      /* ���USB�������� */
                usbx.bDeviceState &= ~(1 << 4); /* �����ѯ��־λ */
            }
            else
            {
                offline_cnt++;

                if (offline_cnt > 50)usbx.bDeviceState = 0; /* 2s��û�յ����߱��,����USB���γ��� */
            }
        }

        if (USB_STA != usbx.bDeviceState) /* ״̬�ı��� */
        {
            gui_fill_rectangle(60 + (lcddev.width - 240) / 2, 150 + (lcddev.height - 320) / 2, 120, 16, g_back_color); /* �����ʾ */

            if (usbx.bDeviceState & 0x01) /* ����д */
            {
                if (busycnt < 5)busycnt++;

                lcd_show_string(60 + (lcddev.width - 240) / 2, 150 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Writing...", BLUE); /* ��ʾUSB����д������ */
            }

            if (usbx.bDeviceState & 0x02) /* ���ڶ� */
            {
                if (busycnt < 5)busycnt++;

                lcd_show_string(60 + (lcddev.width - 240) / 2, 150 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Reading...", BLUE); /* ��ʾUSB���ڶ������� */
            }

            if (usbx.bDeviceState & 0x04)
            {
                if (errcnt < 5)errcnt++;

                lcd_show_string(60 + (lcddev.width - 240) / 2, 170 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Write Error", RED); /* ��ʾд����� */
            }

            if (usbx.bDeviceState & 0x08)
            {
                if (errcnt < 5)errcnt++;

                lcd_show_string(60 + (lcddev.width - 240) / 2, 170 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Read  Error", RED); /* ��ʾ�������� */
            }
            
            if (usbx.bDeviceState & 0X80)lcd_show_string(60 + (lcddev.width - 240) / 2, 130 + (lcddev.height - 320) / 2, lcddev.width, lcddev.height, 16, "USB Connected   ", BLUE);    /* ��ʾUSB�����Ѿ����� */
            else lcd_show_string(60 + (lcddev.width - 240) / 2, 130 + (lcddev.height - 320) / 2, lcddev.width, 320, 16, "USB DisConnected", BLUE);  /* ��ʾUSB���γ��� */

            usbx.bDeviceState &= 0X90;      /* ���������״̬&��ѯ��־�����������λ */
            USB_STA = usbx.bDeviceState;    /* ��¼����״̬ */
        }
    }

    g_back_color = BLACK;
    usbapp_mode_set(USBH_MSC_MODE);         /* HOST MSC */
    return rval;
}







