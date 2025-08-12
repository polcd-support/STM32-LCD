/**
 ****************************************************************************************************
 * @file        audioplay.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-11-25
 * @brief       APP-���ֲ����� ����
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
 * V1.1 20221125
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 *
 ****************************************************************************************************
 */

#include "os.h"
#include "audioplay.h"
#include "settings.h"
#include "ucos_ii.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/SPBLCD/spblcd.h"
#include "wavplay.h"
#include "mp3play.h"
#include "flacplay.h"
#include "apeplay.h"


 /* ��ʿ����� */
_lyric_obj *g_lrcdev = NULL;

/* ���ֲ��ſ����� */
__audiodev g_audiodev;

/* ���ֲ��Ž�������� */
__audioui *g_aui;


/* AUDIO PLAY���� */
#define AUDIO_PLAY_TASK_PRIO    2       /* �����������ȼ� */
#define AUDIO_PLAY_STK_SIZE     512     /* ���������ջ��С */
OS_STK *AUDIO_PLAY_TASK_STK;            /* �����ջ�������ڴ����ķ�ʽ�������� */
void audio_play_task(void *pdata);      /* ������ */

/* ���������������� */
OS_EVENT *audiombox;                    /* �¼����ƿ� */


/* 5��ͼƬ��ť��·�� */
uint8_t *const AUDIO_BTN_PIC_TBL[2][5] =
{
    {
        "1:/SYSTEM/APP/AUDIO/ListR.bmp",
        "1:/SYSTEM/APP/AUDIO/PrevR.bmp",
        "1:/SYSTEM/APP/AUDIO/PauseR.bmp",
        "1:/SYSTEM/APP/AUDIO/NextR.bmp",
        "1:/SYSTEM/APP/AUDIO/ExitR.bmp",
    },
    {
        "1:/SYSTEM/APP/AUDIO/ListP.bmp",
        "1:/SYSTEM/APP/AUDIO/PrevP.bmp",
        "1:/SYSTEM/APP/AUDIO/PlayP.bmp",
        "1:/SYSTEM/APP/AUDIO/NextP.bmp",
        "1:/SYSTEM/APP/AUDIO/ExitP.bmp",
    },
};

uint8_t *const AUDIO_PLAYR_PIC = "1:/SYSTEM/APP/AUDIO/PlayR.bmp";       /* ���� �ɿ� */
uint8_t *const AUDIO_PLAYP_PIC = "1:/SYSTEM/APP/AUDIO/PlayP.bmp";       /* ���� ���� */
uint8_t *const AUDIO_PAUSER_PIC = "1:/SYSTEM/APP/AUDIO/PauseR.bmp";     /* ��ͣ �ɿ� */
uint8_t *const AUDIO_PAUSEP_PIC = "1:/SYSTEM/APP/AUDIO/PauseP.bmp";     /* ��ͣ ���� */

/* ����ͼƬ */
uint8_t *const AUDIO_BACK_PIC[6] =
{
    "1:/SYSTEM/APP/AUDIO/a_240164.jpg",
    "1:/SYSTEM/APP/AUDIO/a_272296.jpg",
    "1:/SYSTEM/APP/AUDIO/a_320296.jpg",
    "1:/SYSTEM/APP/AUDIO/a_480550.jpg",
    "1:/SYSTEM/APP/AUDIO/a_600674.jpg",
    "1:/SYSTEM/APP/AUDIO/a_800880.jpg",	
};


/**
 * @brief       ��ʼ��Ƶ����
 * @param       ��
 * @retval      ��
 */
void audio_start(void)
{
    g_audiodev.status |= 1 << 1;      /* �������� */
    g_audiodev.status |= 1 << 0;      /* ����ͣ״̬ */
    sai1_play_start();
}

/**
 * @brief       ֹͣ��Ƶ����
 * @param       ��
 * @retval      ��
 */
void audio_stop(void)
{
    g_audiodev.status &= ~(1 << 0);   /* ��ͣλ���� */
    g_audiodev.status &= ~(1 << 1);   /* �������� */
    sai1_play_stop();
}

void audio_task_delete(void);

/**
 * @brief       ���ֲ�������,ͨ���������򴴽�
 * @param       pdata           : ���ò���
 * @retval      ��
 */
void audio_play_task(void *pdata)
{
    DIR audiodir;           /* audiodirר�� */
    FILINFO *audioinfo;
    uint8_t rval;
    uint8_t *pname = 0;
    uint8_t res;
    es8388_input_cfg(0);    /* �ر�����ͨ�� */
    es8388_output_cfg(1, 1);/* ����ͨ��1��2����� */
    es8388_adda_cfg(1, 0);  /* ����DAC�ر�ADC */

    while (g_audiodev.status & 0x80)
    {
        g_audiodev.curindex = (uint32_t)OSMboxPend(audiombox, 0, &rval) - 1;    /* ��������,Ҫ��ȥ1,��Ϊ���͵�ʱ��������1 */
        audioinfo = (FILINFO *)gui_memin_malloc(sizeof(FILINFO));               /* ����FILENFO�ڴ� */
        rval = f_opendir(&audiodir, (const TCHAR *)g_audiodev.path);            /* ��ѡ�е�Ŀ¼ */

        while (rval == 0 && audioinfo)
        {
            ff_enter(audiodir.obj.fs);  /* ����fatfs,��ֹ����� */
            dir_sdi(&audiodir, g_audiodev.mfindextbl[g_audiodev.curindex]);
            ff_leave(audiodir.obj.fs);  /* �˳�fatfs,��������os�� */
            rval = f_readdir(&audiodir, audioinfo); /* ��ȡ�ļ���Ϣ */

            if (rval)break; /* ��ʧ�� */

            g_audiodev.name = (uint8_t *)(audioinfo->fname);
            pname = gui_memin_malloc(strlen((const char *)g_audiodev.name) + strlen((const char *)g_audiodev.path) + 2); /* �����ڴ� */

            if (pname == NULL)break;    /* ����ʧ�� */

            pname = gui_path_name(pname, g_audiodev.path, g_audiodev.name);	/* �ļ�������·�� */
            g_audiodev.status |= 1 << 5; /* ����и��� */
            g_audiodev.status |= 1 << 4; /* ������ڲ������� */
            printf("play:%s\r\n", pname);
            SCB_CleanInvalidateDCache();  /* �����Ч��D-Cache */
            app_es8388_volset(es8388set.mvol);

            switch (exfuns_file_type((char *)pname))
            {
                case T_WAV:
                    res = wav_play_song(pname);     /* ����wav�ļ� */
                    break;

                case T_MP3:
                    res = mp3_play_song(pname);     /* ����MP3�ļ� */
                    break;

                case T_FLAC:
                    res = flac_play_song(pname);    /* ����flac�ļ� */
                    break;

                case T_OGG:
                    res = ape_play_song(pname);     /* ����ape�ļ� */
                    break;
            }

            gui_memin_free(pname);/* �ͷ��ڴ� */

            if (res & 0X80)printf("audio error:%d\r\n", res);

            printf("g_audiodev.status:%d\r\n", g_audiodev.status);

            if ((g_audiodev.status & (1 << 6)) == 0)    /* ����ֹ���� */
            {
                if (systemset.audiomode == 0)           /* ˳�򲥷� */
                {
                    if (g_audiodev.curindex < (g_audiodev.mfilenum - 1))g_audiodev.curindex++;
                    else g_audiodev.curindex = 0;
                }
                else if (systemset.audiomode == 1)      /* ������� */
                {
                    g_audiodev.curindex = app_get_rand(g_audiodev.mfilenum);    /* �õ���һ�׸��������� */
                }
                else g_audiodev.curindex = g_audiodev.curindex;                 /* ����ѭ�� */
            }
            else break;
        }

        gui_memin_free(audioinfo);      /* �ͷ��ڴ� */
        g_audiodev.status &= ~(1 << 6); /* ����Ѿ��ɹ���ֹ���� */
        g_audiodev.status &= ~(1 << 4); /* ��������ֲ��� */
        printf("audio play over:%d\r\n", g_audiodev.status);
    }

    audio_task_delete();    /* ɾ������ */
}

/**
 * @brief       ����ֹͣaudio����
 * @param       audiodevx       : audio�ṹ��
 * @retval      ��
 */
void audio_stop_req(__audiodev *audiodevx)
{
    while (audiodevx->status & (1 << 4))  /* �ȴ���ֹ���ųɹ� */
    {
        audiodevx->status &= ~(1 << 1);   /* �����������,���˳����ڲ��ŵĽ��� */
        audiodevx->status |= 1 << 6;      /* ������ֹ����,ֹͣ�Զ�ѭ��/������� */
        delay_ms(10);
    };

    g_audiodev.status &= ~(1 << 6);         /* �������� */
}

/* �����б� */
uint8_t *const MUSIC_LIST[GUI_LANGUAGE_NUM] =
{
    "�����б�", "�����б�", "MUSIC LIST",
};

/**
 * @brief       audio�ļ����,���ļ��洢����
 * @param       audiodevx       : audio�ṹ��
 * @retval      0,��������/�����˳���ť
 *              1,�ڴ����ʧ��
 */
uint8_t audio_filelist(__audiodev *audiodevx)
{
    uint8_t res;
    uint8_t rval = 0;   /* ����ֵ */
    uint16_t i;
    _btn_obj *rbtn;     /* ���ذ�ť�ؼ� */
    _btn_obj *qbtn;     /* �˳���ť�ؼ� */

    _filelistbox_obj *flistbox;
    _filelistbox_list *filelistx;   /* �ļ� */
    app_filebrower((uint8_t *)MUSIC_LIST[gui_phy.language], 0X07);	/* ѡ��Ŀ���ļ�,���õ�Ŀ������ */

    flistbox = filelistbox_creat(0, gui_phy.tbheight, lcddev.width, lcddev.height - gui_phy.tbheight * 2, 1, gui_phy.listfsize); /* ����һ��filelistbox */

    if (flistbox == NULL)rval = 1;  /* �����ڴ�ʧ�� */
    else if (audiodevx->path == NULL)
    {
        flistbox->fliter = FLBOX_FLT_MUSIC; /* ���������ļ� */
        filelistbox_add_disk(flistbox);     /* ��Ӵ���·�� */
        filelistbox_draw_listbox(flistbox);
    }
    else
    {
        flistbox->fliter = FLBOX_FLT_MUSIC; /* ���������ļ� */
        flistbox->path = (uint8_t *)gui_memin_malloc(strlen((const char *)audiodevx->path) + 1); /* Ϊ·�������ڴ� */
        strcpy((char *)flistbox->path, (char *)audiodevx->path);            /* ����·�� */
        filelistbox_scan_filelist(flistbox);    /* ����ɨ���б� */
        flistbox->selindex = flistbox->foldercnt + audiodevx->curindex;     /* ѡ����ĿΪ��ǰ���ڲ��ŵ���Ŀ */

        if (flistbox->scbv->totalitems > flistbox->scbv->itemsperpage)flistbox->scbv->topitem = flistbox->selindex;

        filelistbox_draw_listbox(flistbox);     /* �ػ� */
    }


    rbtn = btn_creat(lcddev.width - 2 * gui_phy.tbfsize - 8 - 1, lcddev.height - gui_phy.tbheight, 2 * gui_phy.tbfsize + 8, gui_phy.tbheight - 1, 0, 0x03); /* �������ְ�ť */
    qbtn = btn_creat(0, lcddev.height - gui_phy.tbheight, 2 * gui_phy.tbfsize + 8, gui_phy.tbheight, 0, 0x03); /* �����˳����ְ�ť */

    if (rbtn == NULL || qbtn == NULL)rval = 1;  /* û���㹻�ڴ湻���� */
    else
    {
        rbtn->caption = (uint8_t *)GUI_BACK_CAPTION_TBL[gui_phy.language];	/* ���� */
        rbtn->font = gui_phy.tbfsize;   /* �����µ������С */
        rbtn->bcfdcolor = WHITE;        /* ����ʱ����ɫ */
        rbtn->bcfucolor = WHITE;        /* �ɿ�ʱ����ɫ */
        btn_draw(rbtn); /* ����ť */

        qbtn->caption = (uint8_t *)GUI_QUIT_CAPTION_TBL[gui_phy.language];	/* ���� */
        qbtn->font = gui_phy.tbfsize;   /* �����µ������С */
        qbtn->bcfdcolor = WHITE;        /* ����ʱ����ɫ */
        qbtn->bcfucolor = WHITE;        /* �ɿ�ʱ����ɫ */
        btn_draw(qbtn); /* ����ť */
    }

    while (rval == 0)
    {
        tp_dev.scan(0);
        in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* �õ�������ֵ */
        delay_ms(1000 / OS_TICKS_PER_SEC);      /* ��ʱһ��ʱ�ӽ��� */

        if(system_task_return)
        {
            delay_ms(10);
            if (tpad_scan(1)) break;  //TPAD����,�ٴ�ȷ��,�ų�����
            else system_task_return = 0;
        }

        filelistbox_check(flistbox, &in_obj);   /* ɨ���ļ� */
        res = btn_check(rbtn, &in_obj);

        if (res)
        {
            if (((rbtn->sta & 0X80) == 0))      /* ��ť״̬�ı��� */
            {
                if (flistbox->dbclick != 0X81)
                {
                    filelistx = filelist_search(flistbox->list, flistbox->selindex); /* �õ���ʱѡ�е�list����Ϣ */

                    if (filelistx->type == FICO_DISK)   /* �Ѿ������������� */
                    {
                        break;
                    }
                    else filelistbox_back(flistbox);    /* �˻���һ��Ŀ¼ */
                }
            }
        }

        res = btn_check(qbtn, &in_obj);

        if (res)
        {
            if (((qbtn->sta & 0X80) == 0))  /* ��ť״̬�ı��� */
            {
                break;  /* �˳� */
            }
        }

        if (flistbox->dbclick == 0X81)      /* ˫���ļ��� */
        {
            audio_stop_req(audiodevx);
            gui_memin_free(audiodevx->path);        /* �ͷ��ڴ� */
            gui_memex_free(audiodevx->mfindextbl);  /* �ͷ��ڴ� */
            audiodevx->path = (uint8_t *)gui_memin_malloc(strlen((const char *)flistbox->path) + 1); /* Ϊ�µ�·�������ڴ� */

            if (audiodevx->path == NULL)
            {
                rval = 1;
                break;
            }

            audiodevx->path[0] = '\0'; /* ���ʼ��������� */
            strcpy((char *)audiodevx->path, (char *)flistbox->path);
            audiodevx->mfindextbl = (uint16_t *)gui_memex_malloc(flistbox->filecnt * 2); /* Ϊ�µ�tbl�����ڴ� */

            if (audiodevx->mfindextbl == NULL)
            {
                rval = 1;
                break;
            }

            for (i = 0; i < flistbox->filecnt; i++)audiodevx->mfindextbl[i] = flistbox->findextbl[i]; /* ���� */

            audiodevx->mfilenum = flistbox->filecnt;    /* ��¼�ļ����� */
            OSMboxPost(audiombox, (void *)(flistbox->selindex - flistbox->foldercnt + 1)); /* ��������,��Ϊ���䲻��Ϊ��,������������1 */
            flistbox->dbclick = 0;
            break;
        }
    }

    filelistbox_delete(flistbox);   /* ɾ��filelist */
    btn_delete(qbtn);               /* ɾ����ť */
    btn_delete(rbtn);               /* ɾ����ť */

    if (rval)
    {
        gui_memin_free(audiodevx->path);        /* �ͷ��ڴ� */
        gui_memex_free(audiodevx->mfindextbl);  /* �ͷ��ڴ� */
        gui_memin_free(audiodevx);
    }

    return rval;
}

/**
 * @brief       audio����������
 * @param       audiodevx       : audio�ṹ��
 * @param       mode            : 0,�����ȡlrc��ʵı���ɫ
 *                                1,��Ҫ��ȡlrc��ʱ���ɫ
 * @retval      ��
 */
void audio_load_ui(uint8_t mode)
{
    uint8_t idx = 0;

    if (lcddev.width == 240)
    {
        g_aui->tpbar_height = 20;
        g_aui->capfsize = 12;
        g_aui->msgfsize = 12;       /* ���ܴ���16 */
        g_aui->lrcdheight = 4;      /* 4�и�� */

        g_aui->msgbar_height = 46;
        g_aui->nfsize = 12;
        g_aui->xygap = 3;
        g_aui->msgdis = 6;          /* ����3��dis */

        g_aui->prgbar_height = 30;
        g_aui->pbarwidth = 160;     /* ������������  12*g_aui->msgfsize/2 */

        g_aui->btnbar_height = 60;
        idx = 0;
    }
    else if (lcddev.width == 272)
    {
        g_aui->tpbar_height = 24;
        g_aui->capfsize = 12;
        g_aui->msgfsize = 12;       /* ���ܴ���16 */
        g_aui->lrcdheight = 6;      /* ��ʼ�� */
        
        g_aui->msgbar_height = 50;
        g_aui->nfsize = 12;
        g_aui->xygap = 4;
        g_aui->msgdis = 8;          /* ����3��dis */
        
        g_aui->prgbar_height = 30;
        g_aui->pbarwidth = 180;     /* ������������  12*aui->msgfsize/2 */
        
        g_aui->btnbar_height = 80;
        idx = 1;
    }
    else if (lcddev.width == 320)
    {
        g_aui->tpbar_height = 24;
        g_aui->capfsize = 12;
        g_aui->msgfsize = 12;       /* ���ܴ���16 */
        g_aui->lrcdheight = 6;      /* ��ʼ�� */

        g_aui->msgbar_height = 50;
        g_aui->nfsize = 12;
        g_aui->xygap = 4;
        g_aui->msgdis = 20;         /* ����3��dis */

        g_aui->prgbar_height = 30;
        g_aui->pbarwidth = 230;     /* ������������  12*g_aui->msgfsize/2 */

        g_aui->btnbar_height = 80;
        idx = 1;
    }
    else if (lcddev.width == 480)
    {
        g_aui->tpbar_height = 30;
        g_aui->capfsize = 16;
        g_aui->msgfsize = 12;       /* ���ܴ���16 */
        g_aui->lrcdheight = 10;     /* ��ʼ�� */

        g_aui->msgbar_height = 60;
        g_aui->nfsize = 12;
        g_aui->xygap = 6;
        g_aui->msgdis = 30;         /* ����3��dis */

        g_aui->prgbar_height = 40;
        g_aui->pbarwidth = 340;     /* ������������  12*g_aui->msgfsize/2 */

        g_aui->btnbar_height = 120;
        idx = 2;
    }
    else if (lcddev.width == 600)
    {
        g_aui->tpbar_height = 40;
        g_aui->capfsize = 24;
        g_aui->msgfsize = 16;       /* ���ܴ���16 */
        g_aui->lrcdheight = 10;     /* ��ʼ�� */
        
        g_aui->msgbar_height = 100;
        g_aui->nfsize = 16;
        g_aui->xygap = 10;
        g_aui->msgdis = 40;         /* ����3��dis */
        
        g_aui->prgbar_height = 60;
        g_aui->pbarwidth = 400;     /* ������������  12*aui->msgfsize/2 */
        
        g_aui->btnbar_height = 150;
        idx = 4;
    }
    else if (lcddev.width == 800)
    {
        g_aui->tpbar_height = 60;
        g_aui->capfsize = 32;
        g_aui->msgfsize = 16;       /* ���ܴ���16 */
        g_aui->lrcdheight = 10;     /* ��ʼ�� */
        
        g_aui->msgbar_height = 100;
        g_aui->nfsize = 16;
        g_aui->xygap = 10;
        g_aui->msgdis = 60;         /* ����3��dis */
        
        g_aui->prgbar_height = 60;
        g_aui->pbarwidth = 600;     /* ������������  12*aui->msgfsize/2 */
        
        g_aui->btnbar_height = 180;
        idx = 5;
    }  

    g_aui->vbarheight = g_aui->msgfsize; /* ����g_aui->msgfsize�Ĵ�С */
    g_aui->pbarheight = g_aui->msgfsize; /* ����g_aui->msgfsize�Ĵ�С */
    g_aui->vbarwidth = lcddev.width - 16 - 2 * g_aui->xygap - 3 * g_aui->msgdis - 13 * g_aui->msgfsize / 2;
    g_aui->vbarx = g_aui->msgdis + 16 + g_aui->xygap;
    g_aui->vbary = g_aui->tpbar_height + g_aui->xygap * 2 + g_aui->msgfsize + (g_aui->msgbar_height - (g_aui->msgfsize + g_aui->xygap * 2 + g_aui->xygap / 2 + g_aui->msgfsize + g_aui->vbarheight)) / 2;
    g_aui->pbarx = (lcddev.width - g_aui->pbarwidth - 12 * g_aui->msgfsize / 2) / 2 + g_aui->msgfsize * 6 / 2;
    g_aui->pbary = lcddev.height - g_aui->btnbar_height - g_aui->prgbar_height + (g_aui->prgbar_height - g_aui->pbarheight) / 2;


    gui_fill_rectangle(0, 0, lcddev.width, g_aui->tpbar_height, AUDIO_TITLE_BKCOLOR);	/* ����������ɫ */
    gui_show_strmid(0, 0, lcddev.width, g_aui->tpbar_height, AUDIO_TITLE_COLOR, g_aui->capfsize, (uint8_t *)APP_MFUNS_CAPTION_TBL[2][gui_phy.language]);	/* ��ʾ���� */
    gui_fill_rectangle(0, g_aui->tpbar_height, lcddev.width, g_aui->msgbar_height, AUDIO_MAIN_BKCOLOR);									/* �����Ϣ������ɫ */
    minibmp_decode((uint8_t *)APP_VOL_PIC, g_aui->msgdis, g_aui->vbary - (16 - g_aui->msgfsize) / 2, 16, 16, 0, 0);										/* ��������ͼ�� */
    gui_show_string("00%", g_aui->vbarx, g_aui->vbary + g_aui->vbarheight + g_aui->xygap / 2, 3 * g_aui->msgfsize / 2, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR); /* ��ʾ���� */
    gui_fill_rectangle(0, lcddev.height - g_aui->btnbar_height - g_aui->prgbar_height, lcddev.width, g_aui->prgbar_height, AUDIO_MAIN_BKCOLOR);	/* ��������������ɫ */
    gui_fill_rectangle(0, lcddev.height - g_aui->btnbar_height, lcddev.width, g_aui->btnbar_height, AUDIO_BTN_BKCOLOR);						/* ��䰴ť������ɫ */
    gui_fill_rectangle(0, g_aui->tpbar_height + g_aui->msgbar_height, lcddev.width, lcddev.height - g_aui->tpbar_height - g_aui->msgbar_height - g_aui->prgbar_height - g_aui->btnbar_height, AUDIO_MAIN_BKCOLOR); /* ����ɫ */

    //if (lcdltdc.pwidth == 0)piclib_ai_load_picfile(AUDIO_BACK_PIC[idx], 0, g_aui->tpbar_height + g_aui->msgbar_height, lcddev.width, lcddev.height - g_aui->tpbar_height - g_aui->msgbar_height - g_aui->prgbar_height - g_aui->btnbar_height, 0); /* ���ر���ͼƬ,MCU������Ӳ��JPEG����,��������ʧ�� */
    //else 
    piclib_ai_load_picfile((char *)AUDIO_BACK_PIC[idx], 0, g_aui->tpbar_height + g_aui->msgbar_height, lcddev.width, lcddev.height - g_aui->tpbar_height - g_aui->msgbar_height - g_aui->prgbar_height - g_aui->btnbar_height, 1);                    /* ���ر���ͼƬ,RGB��,��Ӳ��JPEG���� */

    if ((g_lrcdev != NULL) && mode)audio_lrc_bkcolor_process(g_lrcdev, 0); /* ��ȡLRC����ɫ */

    slcd_dma_init();    /* Ӳ��JPEG����,������dma2stream0,���Ա������³�ʼ�� */
}

/**
 * @brief       ��ʾ�����ٷֱ�
 * @param       pctx            : �ٷֱ�ֵ
 * @retval      ��
 */
void audio_show_vol(uint8_t pctx)
{
    uint8_t *buf;
    uint8_t sy = g_aui->vbary + g_aui->vbarheight + g_aui->xygap / 2;
    gui_phy.back_color = AUDIO_MAIN_BKCOLOR; /* ���ñ���ɫΪ��ɫ */
    gui_fill_rectangle(g_aui->vbarx, sy, 4 * g_aui->msgfsize / 2, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR); /* ��䱳��ɫ */
    buf = gui_memin_malloc(32);
    sprintf((char *)buf, "%d%%", pctx);
    gui_show_string(buf, g_aui->vbarx, sy, 4 * g_aui->msgfsize / 2, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR); /* ��ʾ���� */
    gui_memin_free(buf);
}

/**
 * @brief       ��ʾaudio����ʱ��
 * @param       sx,sy           : ��ʼ����
 * @param       sec             : ʱ��
 * @retval      ��
 */
void audio_time_show(uint16_t sx, uint16_t sy, uint16_t sec)
{
    uint16_t min;
    min = sec / 60; /* �õ������� */

    if (min > 99)min = 99;

    sec = sec % 60; /* �õ������� */
    gui_phy.back_color = AUDIO_MAIN_BKCOLOR; /* ���ñ���ɫΪ��ɫ */
    gui_show_num(sx, sy, 2, AUDIO_INFO_COLOR, g_aui->msgfsize, min, 0x80); /* ��ʾʱ�� */
    gui_show_ptchar(sx + g_aui->msgfsize, sy, lcddev.width, lcddev.height, 0, AUDIO_INFO_COLOR, g_aui->msgfsize, ':', 0); /* ��ʾð�� */
    gui_show_num(sx + (g_aui->msgfsize / 2) * 3, sy, 2, AUDIO_INFO_COLOR, g_aui->msgfsize, sec, 0x80); /* ��ʾʱ�� */
}

/**
 * @brief       ������Ϣ����
 * @param       audiodevx       : audio������
 * @param       audioprgbx      : ������
 * @param       lrcx            : ��ʿ�����
 * @retval      ��
 */
void audio_info_upd(__audiodev *audiodevx, _progressbar_obj *audioprgbx, _progressbar_obj *volprgbx, _lyric_obj *lrcx)
{
    static uint16_t temp;
    uint16_t tempx, tempy;
    uint8_t *buf;
    float ftemp;

    if ((audiodevx->status & (1 << 5)) && (audiodevx->status & (1 << 1))) /* ִ����һ�θ����л�,���������Ѿ��ڲ�����,���¸������ֺ͵�ǰ������Ŀ����,audioprgb���ȵ���Ϣ */
    {
        audiodevx->status &= ~(1 << 5); /* ��ձ�־λ */
        buf = gui_memin_malloc(100);    /* ����100�ֽ��ڴ� */

        if (buf == NULL)return;         /* game over */

        gui_fill_rectangle(0, g_aui->tpbar_height + g_aui->xygap - 1, lcddev.width, g_aui->msgfsize + 2, AUDIO_MAIN_BKCOLOR); /* ���¸������һ��,���֮ǰ����ʾ */
        gui_show_ptstrwhiterim(g_aui->xygap, g_aui->tpbar_height + g_aui->xygap, lcddev.width - g_aui->xygap, lcddev.height, 0, 0X0000, 0XFFFF, g_aui->msgfsize, audiodevx->name);    /* ��ʾ�µ����� */
        audiodevx->namelen = strlen((const char *)audiodevx->name); /* �õ���ռ�ַ��ĸ��� */
        audiodevx->namelen *= 6;        /* �õ����� */
        audiodevx->curnamepos = 0;      /* �õ����� */
        gui_phy.back_color = AUDIO_MAIN_BKCOLOR; /* ���ñ���ɫΪ��ɫ */

        /* ��ʾ�����ٷֱ� */
        audio_show_vol((volprgbx->curpos * 100) / volprgbx->totallen); /* ��ʾ�����ٷֱ� */

        /* ��ʾ��Ŀ��� */
        sprintf((char *)buf, "%03d/%03d", audiodevx->curindex + 1, audiodevx->mfilenum);
        tempx = g_aui->vbarx + g_aui->vbarwidth - 7 * (g_aui->msgfsize) / 2;
        tempy = g_aui->vbary + g_aui->xygap / 2 + g_aui->vbarheight;
        gui_fill_rectangle(tempx, tempy, 7 * (g_aui->msgfsize) / 2, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR);           /* ���֮ǰ����ʾ */
        gui_show_string(buf, tempx, tempy, 7 * (g_aui->msgfsize) / 2, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR);
       
        /* ��ʾxxxKhz */
        tempx = g_aui->vbarx + g_aui->vbarwidth + g_aui->msgdis;
        gui_fill_rectangle(tempx, g_aui->vbary, 4 * g_aui->msgfsize, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR);          /* ���֮ǰ����ʾ */
        ftemp = (float)audiodevx->samplerate / 1000; /* xxx.xKhz */
        sprintf((char *)buf, "%3.1fKhz", ftemp);
        gui_show_string(buf, tempx, g_aui->vbary, 4 * g_aui->msgfsize, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR);
        
        /* ��ʾλ�� */
        tempx = g_aui->vbarx + g_aui->vbarwidth + g_aui->msgdis + 4 * g_aui->msgfsize + g_aui->xygap;
        gui_fill_rectangle(tempx, g_aui->vbary, 5 * (g_aui->msgfsize) / 2, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR);    /* ���֮ǰ����ʾ */
        sprintf((char *)buf, "%02dbit", audiodevx->bps);
        gui_show_string(buf, tempx, g_aui->vbary, 5 * (g_aui->msgfsize) / 2, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR);
       
        /* �������� */
        temp = 0;
        audioprgbx->totallen = audiodevx->file->obj.objsize;    /* �����ܳ��� */
        audioprgbx->curpos = 0;

        if (lrcx)
        {
            lrc_read(lrcx, audiodevx->path, audiodevx->name);
            audio_lrc_bkcolor_process(lrcx, 1); /* �ָ�����ɫ */
            g_lrcdev->curindex = 0;     /* �������ø��λ��Ϊ0 */
            g_lrcdev->curtime = 0;      /* ����ʱ�� */
        }

        gui_memin_free(buf);/* �ͷ��ڴ� */
    }

    if (audiodevx->namelen > lcddev.width - 8) /* ������Ļ���� */
    {
        gui_fill_rectangle(0, g_aui->tpbar_height + g_aui->xygap - 1, lcddev.width, g_aui->msgfsize + 2, AUDIO_MAIN_BKCOLOR); /* ���¸������һ��,���֮ǰ����ʾ */
        gui_show_ptstrwhiterim(g_aui->xygap, g_aui->tpbar_height + g_aui->xygap, lcddev.width - g_aui->xygap, lcddev.height, audiodevx->curnamepos, 0X0000, 0XFFFF, g_aui->msgfsize, audiodevx->name);	/* ��ʾ�µ����� */
        audiodevx->curnamepos++;

        if (audiodevx->curnamepos + lcddev.width - 8 > (audiodevx->namelen + lcddev.width / 2 - 10))audiodevx->curnamepos = 0; /* ѭ����ʾ */
    }

    if (audiodevx->status & (1 << 7))       /* audio���ڲ��� */
    {
        audioprgbx->curpos = f_tell(audiodevx->file);   /* �õ���ǰ�Ĳ���λ�� */
        progressbar_draw_progressbar(audioprgbx);       /* ���½�����λ�� */

        if (temp != audiodevx->cursec)
        {
            temp = audiodevx->cursec;
            buf = gui_memin_malloc(100);    /* ����100�ֽ��ڴ� */

            if (buf == NULL)return;         /* game over */

            /* ��ʾ����(Kbps ) */
            tempx = g_aui->vbarx + g_aui->vbarwidth + g_aui->msgdis;
            tempy = g_aui->vbary + g_aui->xygap / 2 + g_aui->vbarheight;
            gui_fill_rectangle(tempx, tempy, 4 * g_aui->msgfsize, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR);                     /* ���֮ǰ����ʾ */
            sprintf((char *)buf, "%04dKbps", audiodevx->bitrate / 1000);
            gui_show_string(buf, tempx, tempy, 4 * g_aui->msgfsize, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR);    /* ��ʾ�ֱ��� */
            gui_memin_free(buf);    /* �ͷ��ڴ� */
            
            /* ��ʾʱ�� */
            tempx = g_aui->pbarx - 6 * g_aui->msgfsize / 2;
            audio_time_show(tempx, g_aui->pbary, audiodevx->cursec);    /* ��ʾ����ʱ�� */
            tempx = g_aui->pbarx + g_aui->pbarwidth + g_aui->msgfsize / 2;
            audio_time_show(tempx, g_aui->pbary, audiodevx->totsec);    /* ��ʾ��ʱ�� */
        }
    }
}

/**
 * @brief       lrc����ɫ����
 * @param       lrcx            : ��ʿ�����
 * @param       mode            : 0,��ȡ����ɫ
 *                                1,�ָ�����ɫ
 * @retval      ��
 */
void audio_lrc_bkcolor_process(_lyric_obj *lrcx, uint8_t mode)
{
    uint16_t sy;
    uint16_t i;
    uint16_t imgheight = lcddev.height - (g_aui->tpbar_height + g_aui->msgbar_height + g_aui->prgbar_height + g_aui->btnbar_height);
    sy = g_aui->tpbar_height + g_aui->msgbar_height + (imgheight - 8 * g_aui->lrcdheight - 112) / 2;

    for (i = 0; i < 7; i++)
    {
        if (mode == 0) /* ��ȡ����ɫ */
        {
            app_read_bkcolor(20, sy, lcddev.width - 40, 16, lrcx->lrcbkcolor[i]);
        }
        else
        {
            app_recover_bkcolor(20, sy, lcddev.width - 40, 16, lrcx->lrcbkcolor[i]);
        }

        if (i == 2 || i == 3)sy += 16 + g_aui->lrcdheight * 2;
        else sy += 16 + g_aui->lrcdheight;
    }
}

/**
 * @brief       ��ʾ���
 * @param       audiodevx       : audio������
 * @param       lrcx            : ��ʿ�����
 * @retval      ��
 */
void audio_lrc_show(__audiodev *audiodevx, _lyric_obj *lrcx)
{
    uint8_t t;
    uint16_t temp, temp1;
    uint16_t sy;
    uint16_t syadd;
    uint16_t imgheight = lcddev.height - (g_aui->tpbar_height + g_aui->msgbar_height + g_aui->prgbar_height + g_aui->btnbar_height);
    sy = g_aui->tpbar_height + g_aui->msgbar_height + (imgheight - 8 * g_aui->lrcdheight - 112) / 2;

    if (lrcx->oldostime != GUI_TIMER_10MS) /* ÿ10ms����һ�� */
    {
        t = gui_disabs(GUI_TIMER_10MS, lrcx->oldostime); /* ��ֹ�ܾ�û�н����������µ�©�� */
        lrcx->oldostime = GUI_TIMER_10MS;

        if (t > 10)t = 1;

        lrcx->curtime += t; /* ����10ms */

        if (lrcx->indexsize) /* �и�ʴ��� */
        {
            lrcx->detatime += t; /* ��־ʱ��������10ms */

            if (lrcx->curindex < lrcx->indexsize) /* ��û��ʾ�� */
            {
                if ((lrcx->curtime % 100) > 80) /* 1���Ӻ����800ms,��Ҫ��ѯ����ʱ��Ĵ�����ͬ����� */
                {
                    lrcx->curtime = audiodevx->cursec * 100; /* �������� */
                }

                if (lrcx->curtime >= lrcx->time_tbl[lrcx->curindex]) /* ��ǰʱ�䳬����,��Ҫ���¸�� */
                {
                    syadd = sy;
                    temp1 = lrcx->curindex; /* ���ݵ�ǰlrcx->curindex */

                    if (lrcx->curindex >= 3)
                    {
                        lrcx->curindex -= 3;
                        temp = 0;
                    }
                    else
                    {
                        temp = 3 - lrcx->curindex;
                        lrcx->curindex = 0;
                    }

                    for (t = 0; t < 7; t++)	/* ��ʾ7����� */
                    {
                        if (t != 3)lrcx->color = AUDIO_LRC_SCOLOR;
                        else lrcx->color = AUDIO_LRC_MCOLOR;

                        app_recover_bkcolor(20, syadd, lcddev.width - 40, 16, lrcx->lrcbkcolor[t]); /* �ָ�����ɫ */

                        if (lrcx->curindex <= (lrcx->indexsize - 1) && lrcx->curindex >= temp)
                        {
                            lrc_show_linelrc(lrcx, 20, syadd, lcddev.width - 40, 16); /* ��ʾ��� */
                            lrcx->curindex++;
                        }

                        if (temp)temp--;

                        if (t == 2 || t == 3)syadd += 16 + g_aui->lrcdheight * 2;
                        else syadd += 16 + g_aui->lrcdheight;
                    }

                    lrcx->curindex = temp1; /* �ָ�ԭ����ֵ */
                    lrc_show_linelrc(lrcx, 0, 0, 0, 0); /* ��ȡ��ǰ���,���ǲ���ʾ */
                    lrcx->curindex++;	/* ƫ�Ƶ���һ����� */

                    if (lrcx->namelen > (lcddev.width - 40)) /* ��Ҫ������� */
                    {
                        if (lrcx->curindex < lrcx->indexsize) /* �������һ���ʻ��Ǵ��ڵ� */
                        {
                            temp = lrcx->time_tbl[lrcx->curindex - 1]; /* ��ǰ��ʵ�ʱ�� */
                            temp1 = lrcx->time_tbl[lrcx->curindex]; /* ��һ���ʵ�ʱ�� */
                            lrcx->updatetime = (temp1 - temp) / (lrcx->namelen - 150); /* ����õ��������ʱ��,�������50����λ,��Ϊǰ��ĳ���ִ��ʱ��Ӱ�� */

                            if (lrcx->updatetime > 20)lrcx->updatetime = 20; /* ��󲻳���200ms */
                        }
                        else lrcx->updatetime = 5; /* Ĭ�Ϲ���ʱ��.50ms */
                    }
                }
            }

            if (lrcx->detatime >= lrcx->updatetime) /* ÿlrcx->updatetime*10ms������ʾ��ǰ���(�����Ҫ�����Ļ�) */
            {
                if (lrcx->namelen > (lcddev.width - 40)) /* ��������ʾ����,��Ҫ������ʾ������ */
                {
                    syadd = sy + 3 * 16 + g_aui->lrcdheight * 4;
                    app_recover_bkcolor(20, syadd, lcddev.width - 40, 16, lrcx->lrcbkcolor[3]);
                    gui_show_ptstr(20, syadd, lcddev.width - 21, lcddev.height, lrcx->curnamepos, AUDIO_LRC_MCOLOR, lrcx->font, lrcx->buf, 1); /* ������ʾ������ */
                    lrcx->curnamepos++;

                    if (lrcx->curnamepos + 200 > lrcx->namelen + 50)lrcx->curnamepos = 0; /* ѭ����ʾ */
                }

                lrcx->detatime = 0;
            }
        }
    }
}

/**
 * @brief       ����audio task
 * @param       ��
 * @retval      0, �ɹ�
 *              ����, �������
 */
uint8_t audio_task_creat(void)
{
    OS_CPU_SR cpu_sr = 0;
    uint8_t res;
    uint8_t i;
    uint32_t size;
    g_lrcdev = lrc_creat(); /* ������ʹ���ṹ�� */

    if (g_lrcdev)
    {
        g_lrcdev->font = 16;
        size = (lcddev.width - 40) * 16 * 2; /* һ����ʱ������ڴ��С */

        for (i = 0; i < 7; i++) /* ����7����ʵı���ɫ�� */
        {
            g_lrcdev->lrcbkcolor[i] = gui_memex_malloc(size);

            if (g_lrcdev->lrcbkcolor[i] == NULL)break;
        }

        if (i != 7)             /* �ڴ�����ʧ�� */
        {
            audio_task_delete();
            return 2;
        }
    }
    else return 1;  /* ����ʧ�� */

    AUDIO_PLAY_TASK_STK = gui_memin_malloc(AUDIO_PLAY_STK_SIZE * sizeof(OS_STK));

    if (AUDIO_PLAY_TASK_STK == 0)return 1;  /* �ڴ�����ʧ�� */

    g_audiodev.status = 1 << 7;             /* ������Ƶ������������ */
    
    OS_ENTER_CRITICAL();/* �����ٽ���(�޷����жϴ��) */
    res = OSTaskCreate(audio_play_task, (void *)0, (OS_STK *)&AUDIO_PLAY_TASK_STK[AUDIO_PLAY_STK_SIZE - 1], AUDIO_PLAY_TASK_PRIO);
    OS_EXIT_CRITICAL();	/* �˳��ٽ���(���Ա��жϴ��) */
    return res;
}

/**
 * @brief       ɾ��audio_task
 * @param       ��
 * @retval      ��
 */
void audio_task_delete(void)
{
    uint8_t i;

    if (g_audiodev.status == 0)return;  /* �����Ѿ�ֹͣ�� */

    g_audiodev.status = 0;              /* ����ֹͣ */
    gui_memin_free(g_audiodev.path);    /* �ͷ��ڴ� */
    gui_memin_free(g_audiodev.mfindextbl);  /* �ͷ��ڴ� */

    for (i = 0; i < 7; i++) /* �ͷ��ڴ� */
    {
        gui_memex_free(g_lrcdev->lrcbkcolor[i]);
    }

    lrc_delete(g_lrcdev);       /* �ͷŸ����ʾ������ڴ� */
    g_lrcdev = NULL;            /* ָ��յ�ַ */
    
    es8388_adda_cfg(0, 0);      /* �ر�DAC&ADC */
    es8388_input_cfg(0);        /* �ر�����ͨ�� */
    es8388_output_cfg(0, 0);    /* �ر�DAC��� */
    app_es8388_volset(0);       /* �ر�ES8388������� */
    
    gui_memin_free(AUDIO_PLAY_TASK_STK);    /* �ͷ��ڴ� */
    OSTaskDel(AUDIO_PLAY_TASK_PRIO);        /* ɾ�����ֲ������� */
}

/**
 * @brief       audio����
 * @param       ��
 * @retval      ��
 */
uint8_t audio_play(void)
{
    uint8_t i;
    uint8_t res;
    uint8_t tcnt = 0;
    uint8_t rval = 0;       /* 1,�ڴ����;2,����,audio��������;3,����,ֹͣaudio���� */
    uint16_t lastvolpos;
    uint8_t btnsize = 0;    /* ��ť�ߴ� */
    uint8_t btnxpit = 0;    /* ��ť��x�����ϵļ�϶ */
    _progressbar_obj *audioprgb, *volprgb;
    _btn_obj *tbtn[5];

    if ((g_audiodev.status & (1 << 7)) == 0)    /* ��Ƶ���������Ѿ�ɾ��?/��һ�ν���? */
    {
        memset(&g_audiodev, 0, sizeof(__audiodev)); /* g_audiodev������������ */
        res = audio_task_creat();               /* �������� */

        if (res == 0)res = audio_filelist(&g_audiodev); /* ѡ����Ƶ�ļ����в��� */

        if (res || g_audiodev.status == 0X80)   /* ��������ʧ��/�ڴ����ʧ��/û��ѡ����Ƶ���� */
        {
            audio_task_delete();
            return 1;
        }

        system_task_return = 0;
    }
    else
    {
        g_audiodev.status |= 1 << 5;    /* ģ��һ���и�,�Ը������������� */
    }

    g_aui = (__audioui *)gui_memin_malloc(sizeof(__audioui));
    audio_load_ui(1);   /* ���������� */
    audioprgb = progressbar_creat(g_aui->pbarx, g_aui->pbary, g_aui->pbarwidth, g_aui->pbarheight, 0X20);   /* audio���Ž����� */

    if (audioprgb == NULL)rval = 1;

    volprgb = progressbar_creat(g_aui->vbarx, g_aui->vbary, g_aui->vbarwidth, g_aui->vbarheight, 0X20);     /* ������С������ */

    if (volprgb == NULL)rval = 1;

    volprgb->totallen = 30;

    if (es8388set.mvol <= 30)
    {
        volprgb->curpos = es8388set.mvol;
    }
    else    /* ��������� */
    {
        es8388set.mvol = 0;
        volprgb->curpos = 0;
    }

    lastvolpos = volprgb->curpos; /* �趨�����λ�� */

    switch (lcddev.width)
    {
        case 240:
            btnsize = 48;
            break;

        case 272:
            btnsize = 50;
            break;

        case 320:
            btnsize = 60;
            break;

        case 480:
            btnsize = 80;
            break;

        case 600:
            btnsize = 100;
            break;

        case 800:
            btnsize = 140;
            break;

    }

    btnxpit = (lcddev.width - 5 * btnsize) / 5;

    for (i = 0; i < 5; i++) /* ѭ������5����ť */
    {
        tbtn[i] = btn_creat(btnxpit / 2 + i * (btnsize + btnxpit), lcddev.height - btnsize - (g_aui->btnbar_height - btnsize) / 2, btnsize, btnsize, 0, 1); /* ����ͼƬ��ť */

        if (tbtn[i] == NULL)
        {
            rval = 1;    /* ����ʧ�� */
            break;
        }

        tbtn[i]->bcfdcolor = 0X2CFF;    /* ����ʱ�ı���ɫ */
        tbtn[i]->bcfucolor = AUDIO_BTN_BKCOLOR;	/* �ɿ�ʱ����ɫ */
        tbtn[i]->picbtnpathu = (uint8_t *)AUDIO_BTN_PIC_TBL[0][i];
        tbtn[i]->picbtnpathd = (uint8_t *)AUDIO_BTN_PIC_TBL[1][i];
        tbtn[i]->sta = 0;
    }

    if (rval == 0) /* û�д��� */
    {
        audioprgb->inbkcolora = 0x738E; /* Ĭ��ɫ */
        audioprgb->inbkcolorb = AUDIO_INFO_COLOR;   /* Ĭ��ɫ */
        audioprgb->infcolora = 0X75D;   /* Ĭ��ɫ */
        audioprgb->infcolorb = 0X596;   /* Ĭ��ɫ */
        volprgb->inbkcolora = AUDIO_INFO_COLOR;     /* Ĭ��ɫ */
        volprgb->inbkcolorb = AUDIO_INFO_COLOR;     /* Ĭ��ɫ */
        volprgb->infcolora = 0X75D;     /* Ĭ��ɫ */
        volprgb->infcolorb = 0X596;     /* Ĭ��ɫ */

        for (i = 0; i < 5; i++)btn_draw(tbtn[i]);   /* ����ť */

        tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PLAYR_PIC; /* ����һ��֮���Ϊ�����ɿ�״̬ */
        progressbar_draw_progressbar(audioprgb);    /* �������� */
        progressbar_draw_progressbar(volprgb);      /* �������� */
        system_task_return = 0;
        tcnt = 0;

        while (rval == 0)
        {
            tcnt++;/* ��ʱ���� */
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* �õ�������ֵ */
            delay_ms(1000 / OS_TICKS_PER_SEC);      /* ��ʱһ��ʱ�ӽ��� */

            for (i = 0; i < 5; i++)
            {
                res = btn_check(tbtn[i], &in_obj);

                if ((res && ((tbtn[i]->sta & (1 << 7)) == 0) && (tbtn[i]->sta & (1 << 6))) || system_task_return) /* �а����������ɿ�,����TP�ɿ��˻���TPAD���� */
                {
                    if (system_task_return)
                    {
                        i = 4;
                    }

                    switch (i)
                    {
                        case 0:/* file list */
                            audio_filelist(&g_audiodev);
                            //delay_ms(500);
                            audio_load_ui(0);/* ���¼��������� */
                            g_audiodev.status |= 1 << 5; /* ģ��һ���и�,�Ը������������� */

                            if (g_audiodev.status & (1 << 0))   /* ��������? */
                            {
                                tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PAUSER_PIC;
                            }
                            else tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PLAYR_PIC;

                            for (i = 0; i < 5; i++)
                            {
                                btn_draw(tbtn[i]);   /* ����ť */
                            }
                            
                            if (g_audiodev.status & (1 << 0))
                            {
                                tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PLAYR_PIC;
                            }
                            else tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PAUSER_PIC;

                            progressbar_draw_progressbar(audioprgb);    /* �������� */
                            progressbar_draw_progressbar(volprgb);      /* �������� */

                            if (system_task_return)
                            {
                                delay_ms(10);
                                if (tpad_scan(1)) 
                                {
                                    break;  /* TPAD����,�ٴ�ȷ��,�ų����� */
                                }
                                else system_task_return = 0;
                            }

                            break;

                        case 1:/* ��һ��������һ�� */
                        case 3:
                            audio_stop_req(&g_audiodev);

                            if (systemset.audiomode == 1) /* ������� */
                            {
                                g_audiodev.curindex = app_get_rand(g_audiodev.mfilenum); /* �õ���һ�׸��������� */
                            }
                            else
                            {
                                if (i == 1) /* ��һ�� */
                                {
                                    if (g_audiodev.curindex)g_audiodev.curindex--;
                                    else g_audiodev.curindex = g_audiodev.mfilenum - 1;
                                }
                                else
                                {
                                    if (g_audiodev.curindex < (g_audiodev.mfilenum - 1))g_audiodev.curindex++;
                                    else g_audiodev.curindex = 0;
                                }
                            }

                            OSMboxPost(audiombox, (void *)(g_audiodev.curindex + 1)); /* ��������,��Ϊ���䲻��Ϊ��,������������1 */
                            break;

                        case 2:/* ����/��ͣ */
                            if (g_audiodev.status & (1 << 0)) /* ����ͣ */
                            {
                                g_audiodev.status &= ~(1 << 0); /* �����ͣ */
                                tbtn[2]->picbtnpathd = (uint8_t *)AUDIO_PLAYP_PIC;
                                tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PAUSER_PIC;
                            }
                            else /* ��ͣ״̬ */
                            {
                                g_audiodev.status |= 1 << 0; /* ȡ����ͣ */
                                tbtn[2]->picbtnpathd = (uint8_t *)AUDIO_PAUSEP_PIC;
                                tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PLAYR_PIC;
                            }

                            break;

                        case 4:/* ֹͣ����/���� */
                            if ((g_audiodev.status & (1 << 0)) == 0) /* ��ͣ״̬�°��˷��� */
                            {
                                rval = 3; /* �˳�����,audioֹͣ���� */
                                audio_stop_req(&g_audiodev);/* ����ֹͣ���� */
                            }
                            else /* ��ͣ״̬�°�����,�Ǿ͹ر�audio���Ź��� */
                            {
                                rval = 2; /* �˳����Ž���,audio�������� */
                            }

                            break;
                    }
                }
            }

            res = progressbar_check(volprgb, &in_obj); /* ������������� */

            if (res && lastvolpos != volprgb->curpos) /* ��������,��λ�ñ仯��.ִ���������� */
            {
                lastvolpos = volprgb->curpos;

                if (volprgb->curpos)es8388set.mvol = volprgb->curpos; /* �������� */
                else es8388set.mvol = 0;

                app_es8388_volset(es8388set.mvol);
                audio_show_vol((volprgb->curpos * 100) / volprgb->totallen);	/* ��ʾ�����ٷֱ� */
            }

            res = progressbar_check(audioprgb, &in_obj);

            if (res && ((audioprgb->sta && PRGB_BTN_DOWN) == 0)) /* ��������,�����ɿ���,ִ�п������ */
            {
                //printf("audioprgb->curpos:%d\r\n",audioprgb->curpos);
                g_lrcdev->curindex = 0; /* �������ø��λ��Ϊ0 */
                g_lrcdev->curtime = 0;  /* ����ʱ�� */
                audioprgb->curpos = g_audiodev.file_seek(audioprgb->curpos); /* ������� */
            }

            if ((tcnt % 100) == 0)audio_info_upd(&g_audiodev, audioprgb, volprgb, g_lrcdev); /* ������ʾ��Ϣ,ÿ100msִ��һ�� */

            if (g_lrcdev != NULL && ((g_audiodev.status & (1 << 5)) == 0) && (g_audiodev.status & (1 << 7))) /* ���ڲ���,������ͣ,�Ҹ�ʽṹ������ */
            {
                audio_lrc_show(&g_audiodev, g_lrcdev);  /* ������ʾ��� */

            }
        }
    }

    for (i = 0; i < 5; i++)btn_delete(tbtn[i]); /* ɾ����ť */


    progressbar_delete(audioprgb);
    progressbar_delete(volprgb);
    gui_memin_free(g_aui);

    if (rval == 3)  /* �˳�audio����.�Ҳ���̨���� */

    {
        audio_task_delete();    /* ɾ����Ƶ�������� */
    }

    return rval;
}
