/**
 ****************************************************************************************************
 * @file        nesplay.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.2
 * @date        2022-05-26
 * @brief       APP-NESģ���� ����
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
 * V1.1 20160627
 * ������SMS��Ϸ��֧��
 * V1.2 20220526
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "nesplay.h"
#include "nes_main.h"
#include "sms_main.h"
#include "usb_app.h"
#include "spb.h" 
#include "./FATFS/exfuns/exfuns.h"


uint8_t * rom_file;
uint8_t * const nes_caption_tbl[GUI_LANGUAGE_NUM]={"NES&SMSģ����","NES&SMSģ�M��","NES&SMS Emulator",};

uint8_t * const nes_remindmsg_tbl[4][GUI_LANGUAGE_NUM]=
{
    {"���Ȱε�U��...","Ո�Ȱε�U�P...","Please pull U disk first...",},
    {"�����USB�ֱ�/����!","Ո����USB�ֱ�/�I�P!","Please plug USB gamepad/keyboard!",},
    {"��⵽����!","�z�y���I�P!","Keyboard detected!",},
    {"��⵽��Ϸ�ֱ�!","�z�y���[���ֱ�!","Gamepad detected!",}, 
};

/* ������ʾ */
uint8_t*const nes_errormsg_tbl[3][GUI_LANGUAGE_NUM]=
{
    {"�ڴ治��!","�ȴ治��!","Out of memory!",},
    {"�ļ���ȡ����!","�ļ��xȡ�e�`!","Read file error!",},
    {"MAP��֧��!","MAP��֧��!","Not supported MAP!",},
};

/* ������Ϸ���� */
void nes_load_ui(void)
{
    app_filebrower((uint8_t*)nes_caption_tbl[gui_phy.language],0X05);/* ��ʾ���� */
    gui_fill_rectangle(0,20,lcddev.width,lcddev.height - 20,BLACK);/* ����ɫ */
}

/* NES��Ϸ */
uint8_t nes_play(void)
{
    DIR nesdir;         /* nesdirר�� */
    FILINFO *nesinfo;
    uint8_t res;
    uint8_t rval = 0;   /* ����ֵ */
    uint8_t *pname = 0;
    uint8_t *fn;
    uint8_t remindflag = 0;
  
    _btn_obj* rbtn;     /* ���ذ�ť�ؼ� */
    _filelistbox_obj * flistbox;
    _filelistbox_list * filelistx; /* �ļ� */

    /* ���U����λ,��ʾҪ�ε�U�� */
    if (gui_phy.memdevflag & (1 << 3))
    {
        window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)nes_remindmsg_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        
        while (gui_phy.memdevflag & (1 << 3))
        {
            delay_ms(5);                /* ��ѭ���ȴ�U�̱��γ� */
        }
    }
    
    usbapp_mode_set(USBH_HID_MODE);     /* ����USBΪHIDģʽ */
    lcd_clear(BLACK);
    /* ��ʾ�����ֱ�/���� */
    window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)nes_remindmsg_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
    delay_ms(800);
    app_filebrower((uint8_t*)APP_MFUNS_CAPTION_TBL[6][gui_phy.language],0X07);  /* ѡ��Ŀ���ļ�,���õ�Ŀ������ */
    flistbox = filelistbox_creat(0,gui_phy.tbheight,lcddev.width,lcddev.height - gui_phy.tbheight * 2,1,gui_phy.listfsize); /* ����һ��filelistbox */
    
    if (flistbox == NULL) rval = 1;      /* �����ڴ�ʧ�� */
    else  
    {
        flistbox->fliter = FLBOX_FLT_NES;   /* �����ı��ļ� */
        filelistbox_add_disk(flistbox);     /* ��Ӵ���·�� */
        filelistbox_draw_listbox(flistbox);
    }
    
    rbtn = btn_creat(lcddev.width - 2 * gui_phy.tbfsize - 8 - 1,lcddev.height-gui_phy.tbheight,2 * gui_phy.tbfsize + 8,gui_phy.tbheight - 1,0,0x03);    /* �������ְ�ť */
    nesinfo = (FILINFO*)gui_memin_malloc(sizeof(FILINFO));/* ����FILENFO�ڴ� */
    
    if (!nesinfo || !rbtn) rval = 1;    /* û���㹻�ڴ湻���� */
    else
    {
        rbtn->caption = (uint8_t*)GUI_BACK_CAPTION_TBL[gui_phy.language];   /* ���� */
        rbtn->font=gui_phy.tbfsize;         /* �����µ������С */
        rbtn->bcfdcolor = WHITE;            /* ����ʱ����ɫ */
        rbtn->bcfucolor = WHITE;            /* �ɿ�ʱ����ɫ */
        btn_draw(rbtn);                     /* ����ť */
    }   
    while (rval == 0)
    {
        tp_dev.scan(0);
        in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);      /* �õ�������ֵ */
        delay_ms(5);
        
        if (system_task_return)
        {
            delay_ms(15);
            if(tpad_scan(1)) break;         /* TPAD����,�ٴ�ȷ��,�ų����� */
            else system_task_return = 0;
        }
        
        filelistbox_check(flistbox,&in_obj);        /* ɨ���ļ� */
        res = btn_check(rbtn,&in_obj);
        
        if (res)
        {
            if (((rbtn->sta & 0X80) == 0))          /* ��ť״̬�ı��� */
            {
                if (flistbox->dbclick != 0X81)
                {
                    filelistx = filelist_search(flistbox->list,flistbox->selindex); /* �õ���ʱѡ�е�list����Ϣ */
                    
                    if (filelistx->type == FICO_DISK)   /* �Ѿ������������� */
                    {
                        break;
                    }
                    else filelistbox_back(flistbox);    /* �˻���һ��Ŀ¼ */
                } 
            }
        }
        
        if (flistbox->dbclick == 0X81)                  /* ˫���ļ��� */
        {
            rval = f_opendir(&nesdir,(const TCHAR*)flistbox->path); /* ��ѡ�е�Ŀ¼ */
            
            if (rval) break;
            
            dir_sdi(&nesdir,flistbox->findextbl[flistbox->selindex-flistbox->foldercnt]);
            rval = f_readdir(&nesdir,nesinfo);  /* ��ȡ�ļ���Ϣ */
            
            if (rval) break;/* �򿪳ɹ� */
            
            fn = (uint8_t*)(nesinfo->fname);
            
            pname = gui_memin_malloc(strlen((const char*)fn) + strlen((const char*)flistbox->path) + 2);/* �����ڴ� */
            
            if (!pname) rval = 1;/* ����ʧ�� */
            else
            {
                pname = gui_path_name(pname,flistbox->path,fn); /* �ļ�������·�� */
                lcd_clear(BLACK); 
                app_filebrower(fn,0X05);    /* ��ʾ��ǰ���������Ϸ���� */
                
                switch (exfuns_file_type((char *)pname))
                {
                    case T_NES:
                        res = nes_load(pname);    /* NES��Ϸ */
                        break; 
                   case T_SMS:
                       res = sms_load(pname);    /* SMS��Ϸ */
                       break;
                }  
                if (res)
                {
                    window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)nes_errormsg_tbl[res - 1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
                    delay_ms(1200); 
                }
                system_task_return = 0;     /* �˳���־���� */
            }
            
            flistbox->dbclick = 0;          /* ���÷��ļ����״̬ */
            gui_memin_free(pname);          /* �ͷ��ڴ� */
            pname = NULL;
            app_filebrower((uint8_t*)nes_caption_tbl[gui_phy.language],0X07);   /* ѡ��Ŀ���ļ�,���õ�Ŀ������ */
            btn_draw(rbtn);                 /* ����ť */
            delay_ms(100);
            filelistbox_rebuild_filelist(flistbox); /* �ؽ�flistbox */
            system_task_return = 0;             /* �ո��˳����������Ϸ,�������˳����ѭ�� */
        }
        
        while ((usbx.bDeviceState & 0XC0) == 0X40)      /* USB�豸������,���ǻ�û���ӳɹ�,�Ͳ�ѯ. */
        {
            usbapp_pulling();   /* ��ѯ����USB���� */
        }
        usbapp_pulling();       /* ��ѯ����USB���� */
        
        if (remindflag == 0)
        {
            if (usbx.hdevclass == 3 || usbx.hdevclass == 4)
            {
                window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)nes_remindmsg_tbl[usbx.hdevclass - 1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
                delay_ms(800);
                filelistbox_rebuild_filelist(flistbox); /* �ؽ�flistbox */
                remindflag = 1;
            }
        }
        
        if (usbx.bDeviceState == 0) remindflag = 0;
    }
    
    filelistbox_delete(flistbox);   /* ɾ��filelist */
    btn_delete(rbtn);               /* ɾ����ť */
    gui_memin_free(pname);          /* �ͷ��ڴ� */
    gui_memin_free(nesinfo);        /* �ͷ��ڴ� */
    usbapp_mode_set(USBH_MSC_MODE); /* �ָ�USBΪUSBH MSCģʽ */
    
    return rval;
}

