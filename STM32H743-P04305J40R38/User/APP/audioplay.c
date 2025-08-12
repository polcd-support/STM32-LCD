/**
 ****************************************************************************************************
 * @file        audioplay.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2022-11-25
 * @brief       APP-音乐播放器 代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
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
 * V1.1 20221125
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
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


 /* 歌词控制器 */
_lyric_obj *g_lrcdev = NULL;

/* 音乐播放控制器 */
__audiodev g_audiodev;

/* 音乐播放界面控制器 */
__audioui *g_aui;


/* AUDIO PLAY任务 */
#define AUDIO_PLAY_TASK_PRIO    2       /* 设置任务优先级 */
#define AUDIO_PLAY_STK_SIZE     512     /* 设置任务堆栈大小 */
OS_STK *AUDIO_PLAY_TASK_STK;            /* 任务堆栈，采用内存管理的方式控制申请 */
void audio_play_task(void *pdata);      /* 任务函数 */

/* 播放音乐任务邮箱 */
OS_EVENT *audiombox;                    /* 事件控制块 */


/* 5个图片按钮的路径 */
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

uint8_t *const AUDIO_PLAYR_PIC = "1:/SYSTEM/APP/AUDIO/PlayR.bmp";       /* 播放 松开 */
uint8_t *const AUDIO_PLAYP_PIC = "1:/SYSTEM/APP/AUDIO/PlayP.bmp";       /* 播放 按下 */
uint8_t *const AUDIO_PAUSER_PIC = "1:/SYSTEM/APP/AUDIO/PauseR.bmp";     /* 暂停 松开 */
uint8_t *const AUDIO_PAUSEP_PIC = "1:/SYSTEM/APP/AUDIO/PauseP.bmp";     /* 暂停 按下 */

/* 背景图片 */
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
 * @brief       开始音频播放
 * @param       无
 * @retval      无
 */
void audio_start(void)
{
    g_audiodev.status |= 1 << 1;      /* 开启播放 */
    g_audiodev.status |= 1 << 0;      /* 非暂停状态 */
    sai1_play_start();
}

/**
 * @brief       停止音频播放
 * @param       无
 * @retval      无
 */
void audio_stop(void)
{
    g_audiodev.status &= ~(1 << 0);   /* 暂停位清零 */
    g_audiodev.status &= ~(1 << 1);   /* 结束播放 */
    sai1_play_stop();
}

void audio_task_delete(void);

/**
 * @brief       音乐播放任务,通过其他程序创建
 * @param       pdata           : 无用参数
 * @retval      无
 */
void audio_play_task(void *pdata)
{
    DIR audiodir;           /* audiodir专用 */
    FILINFO *audioinfo;
    uint8_t rval;
    uint8_t *pname = 0;
    uint8_t res;
    es8388_input_cfg(0);    /* 关闭输入通道 */
    es8388_output_cfg(1, 1);/* 开启通道1和2的输出 */
    es8388_adda_cfg(1, 0);  /* 开启DAC关闭ADC */

    while (g_audiodev.status & 0x80)
    {
        g_audiodev.curindex = (uint32_t)OSMboxPend(audiombox, 0, &rval) - 1;    /* 请求邮箱,要减去1,因为发送的时候增加了1 */
        audioinfo = (FILINFO *)gui_memin_malloc(sizeof(FILINFO));               /* 申请FILENFO内存 */
        rval = f_opendir(&audiodir, (const TCHAR *)g_audiodev.path);            /* 打开选中的目录 */

        while (rval == 0 && audioinfo)
        {
            ff_enter(audiodir.obj.fs);  /* 进入fatfs,防止被打断 */
            dir_sdi(&audiodir, g_audiodev.mfindextbl[g_audiodev.curindex]);
            ff_leave(audiodir.obj.fs);  /* 退出fatfs,继续运行os等 */
            rval = f_readdir(&audiodir, audioinfo); /* 读取文件信息 */

            if (rval)break; /* 打开失败 */

            g_audiodev.name = (uint8_t *)(audioinfo->fname);
            pname = gui_memin_malloc(strlen((const char *)g_audiodev.name) + strlen((const char *)g_audiodev.path) + 2); /* 申请内存 */

            if (pname == NULL)break;    /* 申请失败 */

            pname = gui_path_name(pname, g_audiodev.path, g_audiodev.name);	/* 文件名加入路径 */
            g_audiodev.status |= 1 << 5; /* 标记切歌了 */
            g_audiodev.status |= 1 << 4; /* 标记正在播放音乐 */
            printf("play:%s\r\n", pname);
            SCB_CleanInvalidateDCache();  /* 清除无效的D-Cache */
            app_es8388_volset(es8388set.mvol);

            switch (exfuns_file_type((char *)pname))
            {
                case T_WAV:
                    res = wav_play_song(pname);     /* 播放wav文件 */
                    break;

                case T_MP3:
                    res = mp3_play_song(pname);     /* 播放MP3文件 */
                    break;

                case T_FLAC:
                    res = flac_play_song(pname);    /* 播放flac文件 */
                    break;

                case T_OGG:
                    res = ape_play_song(pname);     /* 播放ape文件 */
                    break;
            }

            gui_memin_free(pname);/* 释放内存 */

            if (res & 0X80)printf("audio error:%d\r\n", res);

            printf("g_audiodev.status:%d\r\n", g_audiodev.status);

            if ((g_audiodev.status & (1 << 6)) == 0)    /* 不终止播放 */
            {
                if (systemset.audiomode == 0)           /* 顺序播放 */
                {
                    if (g_audiodev.curindex < (g_audiodev.mfilenum - 1))g_audiodev.curindex++;
                    else g_audiodev.curindex = 0;
                }
                else if (systemset.audiomode == 1)      /* 随机播放 */
                {
                    g_audiodev.curindex = app_get_rand(g_audiodev.mfilenum);    /* 得到下一首歌曲的索引 */
                }
                else g_audiodev.curindex = g_audiodev.curindex;                 /* 单曲循环 */
            }
            else break;
        }

        gui_memin_free(audioinfo);      /* 释放内存 */
        g_audiodev.status &= ~(1 << 6); /* 标记已经成功终止播放 */
        g_audiodev.status &= ~(1 << 4); /* 标记无音乐播放 */
        printf("audio play over:%d\r\n", g_audiodev.status);
    }

    audio_task_delete();    /* 删除任务 */
}

/**
 * @brief       请求停止audio播放
 * @param       audiodevx       : audio结构体
 * @retval      无
 */
void audio_stop_req(__audiodev *audiodevx)
{
    while (audiodevx->status & (1 << 4))  /* 等待终止播放成功 */
    {
        audiodevx->status &= ~(1 << 1);   /* 请求结束播放,以退出正在播放的进程 */
        audiodevx->status |= 1 << 6;      /* 请求终止播放,停止自动循环/随机播放 */
        delay_ms(10);
    };

    g_audiodev.status &= ~(1 << 6);         /* 撤销请求 */
}

/* 音乐列表 */
uint8_t *const MUSIC_LIST[GUI_LANGUAGE_NUM] =
{
    "音乐列表", "音樂列表", "MUSIC LIST",
};

/**
 * @brief       audio文件浏览,带文件存储功能
 * @param       audiodevx       : audio结构体
 * @retval      0,正常返回/按了退出按钮
 *              1,内存分配失败
 */
uint8_t audio_filelist(__audiodev *audiodevx)
{
    uint8_t res;
    uint8_t rval = 0;   /* 返回值 */
    uint16_t i;
    _btn_obj *rbtn;     /* 返回按钮控件 */
    _btn_obj *qbtn;     /* 退出按钮控件 */

    _filelistbox_obj *flistbox;
    _filelistbox_list *filelistx;   /* 文件 */
    app_filebrower((uint8_t *)MUSIC_LIST[gui_phy.language], 0X07);	/* 选择目标文件,并得到目标数量 */

    flistbox = filelistbox_creat(0, gui_phy.tbheight, lcddev.width, lcddev.height - gui_phy.tbheight * 2, 1, gui_phy.listfsize); /* 创建一个filelistbox */

    if (flistbox == NULL)rval = 1;  /* 申请内存失败 */
    else if (audiodevx->path == NULL)
    {
        flistbox->fliter = FLBOX_FLT_MUSIC; /* 查找音乐文件 */
        filelistbox_add_disk(flistbox);     /* 添加磁盘路径 */
        filelistbox_draw_listbox(flistbox);
    }
    else
    {
        flistbox->fliter = FLBOX_FLT_MUSIC; /* 查找音乐文件 */
        flistbox->path = (uint8_t *)gui_memin_malloc(strlen((const char *)audiodevx->path) + 1); /* 为路径申请内存 */
        strcpy((char *)flistbox->path, (char *)audiodevx->path);            /* 复制路径 */
        filelistbox_scan_filelist(flistbox);    /* 重新扫描列表 */
        flistbox->selindex = flistbox->foldercnt + audiodevx->curindex;     /* 选中条目为当前正在播放的条目 */

        if (flistbox->scbv->totalitems > flistbox->scbv->itemsperpage)flistbox->scbv->topitem = flistbox->selindex;

        filelistbox_draw_listbox(flistbox);     /* 重画 */
    }


    rbtn = btn_creat(lcddev.width - 2 * gui_phy.tbfsize - 8 - 1, lcddev.height - gui_phy.tbheight, 2 * gui_phy.tbfsize + 8, gui_phy.tbheight - 1, 0, 0x03); /* 创建文字按钮 */
    qbtn = btn_creat(0, lcddev.height - gui_phy.tbheight, 2 * gui_phy.tbfsize + 8, gui_phy.tbheight, 0, 0x03); /* 创建退出文字按钮 */

    if (rbtn == NULL || qbtn == NULL)rval = 1;  /* 没有足够内存够分配 */
    else
    {
        rbtn->caption = (uint8_t *)GUI_BACK_CAPTION_TBL[gui_phy.language];	/* 返回 */
        rbtn->font = gui_phy.tbfsize;   /* 设置新的字体大小 */
        rbtn->bcfdcolor = WHITE;        /* 按下时的颜色 */
        rbtn->bcfucolor = WHITE;        /* 松开时的颜色 */
        btn_draw(rbtn); /* 画按钮 */

        qbtn->caption = (uint8_t *)GUI_QUIT_CAPTION_TBL[gui_phy.language];	/* 名字 */
        qbtn->font = gui_phy.tbfsize;   /* 设置新的字体大小 */
        qbtn->bcfdcolor = WHITE;        /* 按下时的颜色 */
        qbtn->bcfucolor = WHITE;        /* 松开时的颜色 */
        btn_draw(qbtn); /* 画按钮 */
    }

    while (rval == 0)
    {
        tp_dev.scan(0);
        in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
        delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

        if(system_task_return)
        {
            delay_ms(10);
            if (tpad_scan(1)) break;  //TPAD返回,再次确认,排除干扰
            else system_task_return = 0;
        }

        filelistbox_check(flistbox, &in_obj);   /* 扫描文件 */
        res = btn_check(rbtn, &in_obj);

        if (res)
        {
            if (((rbtn->sta & 0X80) == 0))      /* 按钮状态改变了 */
            {
                if (flistbox->dbclick != 0X81)
                {
                    filelistx = filelist_search(flistbox->list, flistbox->selindex); /* 得到此时选中的list的信息 */

                    if (filelistx->type == FICO_DISK)   /* 已经不能再往上了 */
                    {
                        break;
                    }
                    else filelistbox_back(flistbox);    /* 退回上一层目录 */
                }
            }
        }

        res = btn_check(qbtn, &in_obj);

        if (res)
        {
            if (((qbtn->sta & 0X80) == 0))  /* 按钮状态改变了 */
            {
                break;  /* 退出 */
            }
        }

        if (flistbox->dbclick == 0X81)      /* 双击文件了 */
        {
            audio_stop_req(audiodevx);
            gui_memin_free(audiodevx->path);        /* 释放内存 */
            gui_memex_free(audiodevx->mfindextbl);  /* 释放内存 */
            audiodevx->path = (uint8_t *)gui_memin_malloc(strlen((const char *)flistbox->path) + 1); /* 为新的路径申请内存 */

            if (audiodevx->path == NULL)
            {
                rval = 1;
                break;
            }

            audiodevx->path[0] = '\0'; /* 在最开始加入结束符 */
            strcpy((char *)audiodevx->path, (char *)flistbox->path);
            audiodevx->mfindextbl = (uint16_t *)gui_memex_malloc(flistbox->filecnt * 2); /* 为新的tbl申请内存 */

            if (audiodevx->mfindextbl == NULL)
            {
                rval = 1;
                break;
            }

            for (i = 0; i < flistbox->filecnt; i++)audiodevx->mfindextbl[i] = flistbox->findextbl[i]; /* 复制 */

            audiodevx->mfilenum = flistbox->filecnt;    /* 记录文件个数 */
            OSMboxPost(audiombox, (void *)(flistbox->selindex - flistbox->foldercnt + 1)); /* 发送邮箱,因为邮箱不能为空,所以在这必须加1 */
            flistbox->dbclick = 0;
            break;
        }
    }

    filelistbox_delete(flistbox);   /* 删除filelist */
    btn_delete(qbtn);               /* 删除按钮 */
    btn_delete(rbtn);               /* 删除按钮 */

    if (rval)
    {
        gui_memin_free(audiodevx->path);        /* 释放内存 */
        gui_memex_free(audiodevx->mfindextbl);  /* 释放内存 */
        gui_memin_free(audiodevx);
    }

    return rval;
}

/**
 * @brief       audio加载主界面
 * @param       audiodevx       : audio结构体
 * @param       mode            : 0,无需读取lrc歌词的背景色
 *                                1,需要读取lrc歌词背景色
 * @retval      无
 */
void audio_load_ui(uint8_t mode)
{
    uint8_t idx = 0;

    if (lcddev.width == 240)
    {
        g_aui->tpbar_height = 20;
        g_aui->capfsize = 12;
        g_aui->msgfsize = 12;       /* 不能大于16 */
        g_aui->lrcdheight = 4;      /* 4行歌词 */

        g_aui->msgbar_height = 46;
        g_aui->nfsize = 12;
        g_aui->xygap = 3;
        g_aui->msgdis = 6;          /* 横向3个dis */

        g_aui->prgbar_height = 30;
        g_aui->pbarwidth = 160;     /* 两边延伸至少  12*g_aui->msgfsize/2 */

        g_aui->btnbar_height = 60;
        idx = 0;
    }
    else if (lcddev.width == 272)
    {
        g_aui->tpbar_height = 24;
        g_aui->capfsize = 12;
        g_aui->msgfsize = 12;       /* 不能大于16 */
        g_aui->lrcdheight = 6;      /* 歌词间距 */
        
        g_aui->msgbar_height = 50;
        g_aui->nfsize = 12;
        g_aui->xygap = 4;
        g_aui->msgdis = 8;          /* 横向3个dis */
        
        g_aui->prgbar_height = 30;
        g_aui->pbarwidth = 180;     /* 两边延伸至少  12*aui->msgfsize/2 */
        
        g_aui->btnbar_height = 80;
        idx = 1;
    }
    else if (lcddev.width == 320)
    {
        g_aui->tpbar_height = 24;
        g_aui->capfsize = 12;
        g_aui->msgfsize = 12;       /* 不能大于16 */
        g_aui->lrcdheight = 6;      /* 歌词间距 */

        g_aui->msgbar_height = 50;
        g_aui->nfsize = 12;
        g_aui->xygap = 4;
        g_aui->msgdis = 20;         /* 横向3个dis */

        g_aui->prgbar_height = 30;
        g_aui->pbarwidth = 230;     /* 两边延伸至少  12*g_aui->msgfsize/2 */

        g_aui->btnbar_height = 80;
        idx = 1;
    }
    else if (lcddev.width == 480)
    {
        g_aui->tpbar_height = 30;
        g_aui->capfsize = 16;
        g_aui->msgfsize = 12;       /* 不能大于16 */
        g_aui->lrcdheight = 10;     /* 歌词间距 */

        g_aui->msgbar_height = 60;
        g_aui->nfsize = 12;
        g_aui->xygap = 6;
        g_aui->msgdis = 30;         /* 横向3个dis */

        g_aui->prgbar_height = 40;
        g_aui->pbarwidth = 340;     /* 两边延伸至少  12*g_aui->msgfsize/2 */

        g_aui->btnbar_height = 120;
        idx = 2;
    }
    else if (lcddev.width == 600)
    {
        g_aui->tpbar_height = 40;
        g_aui->capfsize = 24;
        g_aui->msgfsize = 16;       /* 不能大于16 */
        g_aui->lrcdheight = 10;     /* 歌词间距 */
        
        g_aui->msgbar_height = 100;
        g_aui->nfsize = 16;
        g_aui->xygap = 10;
        g_aui->msgdis = 40;         /* 横向3个dis */
        
        g_aui->prgbar_height = 60;
        g_aui->pbarwidth = 400;     /* 两边延伸至少  12*aui->msgfsize/2 */
        
        g_aui->btnbar_height = 150;
        idx = 4;
    }
    else if (lcddev.width == 800)
    {
        g_aui->tpbar_height = 60;
        g_aui->capfsize = 32;
        g_aui->msgfsize = 16;       /* 不能大于16 */
        g_aui->lrcdheight = 10;     /* 歌词间距 */
        
        g_aui->msgbar_height = 100;
        g_aui->nfsize = 16;
        g_aui->xygap = 10;
        g_aui->msgdis = 60;         /* 横向3个dis */
        
        g_aui->prgbar_height = 60;
        g_aui->pbarwidth = 600;     /* 两边延伸至少  12*aui->msgfsize/2 */
        
        g_aui->btnbar_height = 180;
        idx = 5;
    }  

    g_aui->vbarheight = g_aui->msgfsize; /* 等于g_aui->msgfsize的大小 */
    g_aui->pbarheight = g_aui->msgfsize; /* 等于g_aui->msgfsize的大小 */
    g_aui->vbarwidth = lcddev.width - 16 - 2 * g_aui->xygap - 3 * g_aui->msgdis - 13 * g_aui->msgfsize / 2;
    g_aui->vbarx = g_aui->msgdis + 16 + g_aui->xygap;
    g_aui->vbary = g_aui->tpbar_height + g_aui->xygap * 2 + g_aui->msgfsize + (g_aui->msgbar_height - (g_aui->msgfsize + g_aui->xygap * 2 + g_aui->xygap / 2 + g_aui->msgfsize + g_aui->vbarheight)) / 2;
    g_aui->pbarx = (lcddev.width - g_aui->pbarwidth - 12 * g_aui->msgfsize / 2) / 2 + g_aui->msgfsize * 6 / 2;
    g_aui->pbary = lcddev.height - g_aui->btnbar_height - g_aui->prgbar_height + (g_aui->prgbar_height - g_aui->pbarheight) / 2;


    gui_fill_rectangle(0, 0, lcddev.width, g_aui->tpbar_height, AUDIO_TITLE_BKCOLOR);	/* 填充标题栏底色 */
    gui_show_strmid(0, 0, lcddev.width, g_aui->tpbar_height, AUDIO_TITLE_COLOR, g_aui->capfsize, (uint8_t *)APP_MFUNS_CAPTION_TBL[2][gui_phy.language]);	/* 显示标题 */
    gui_fill_rectangle(0, g_aui->tpbar_height, lcddev.width, g_aui->msgbar_height, AUDIO_MAIN_BKCOLOR);									/* 填充信息栏背景色 */
    minibmp_decode((uint8_t *)APP_VOL_PIC, g_aui->msgdis, g_aui->vbary - (16 - g_aui->msgfsize) / 2, 16, 16, 0, 0);										/* 解码音量图标 */
    gui_show_string("00%", g_aui->vbarx, g_aui->vbary + g_aui->vbarheight + g_aui->xygap / 2, 3 * g_aui->msgfsize / 2, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR); /* 显示音量 */
    gui_fill_rectangle(0, lcddev.height - g_aui->btnbar_height - g_aui->prgbar_height, lcddev.width, g_aui->prgbar_height, AUDIO_MAIN_BKCOLOR);	/* 填充进度条栏背景色 */
    gui_fill_rectangle(0, lcddev.height - g_aui->btnbar_height, lcddev.width, g_aui->btnbar_height, AUDIO_BTN_BKCOLOR);						/* 填充按钮栏背景色 */
    gui_fill_rectangle(0, g_aui->tpbar_height + g_aui->msgbar_height, lcddev.width, lcddev.height - g_aui->tpbar_height - g_aui->msgbar_height - g_aui->prgbar_height - g_aui->btnbar_height, AUDIO_MAIN_BKCOLOR); /* 填充底色 */

    //if (lcdltdc.pwidth == 0)piclib_ai_load_picfile(AUDIO_BACK_PIC[idx], 0, g_aui->tpbar_height + g_aui->msgbar_height, lcddev.width, lcddev.height - g_aui->tpbar_height - g_aui->msgbar_height - g_aui->prgbar_height - g_aui->btnbar_height, 0); /* 加载背景图片,MCU屏不用硬件JPEG解码,否则会读点失败 */
    //else 
    piclib_ai_load_picfile((char *)AUDIO_BACK_PIC[idx], 0, g_aui->tpbar_height + g_aui->msgbar_height, lcddev.width, lcddev.height - g_aui->tpbar_height - g_aui->msgbar_height - g_aui->prgbar_height - g_aui->btnbar_height, 1);                    /* 加载背景图片,RGB屏,用硬件JPEG解码 */

    if ((g_lrcdev != NULL) && mode)audio_lrc_bkcolor_process(g_lrcdev, 0); /* 读取LRC背景色 */

    slcd_dma_init();    /* 硬件JPEG解码,会重置dma2stream0,所以必须重新初始化 */
}

/**
 * @brief       显示音量百分比
 * @param       pctx            : 百分比值
 * @retval      无
 */
void audio_show_vol(uint8_t pctx)
{
    uint8_t *buf;
    uint8_t sy = g_aui->vbary + g_aui->vbarheight + g_aui->xygap / 2;
    gui_phy.back_color = AUDIO_MAIN_BKCOLOR; /* 设置背景色为底色 */
    gui_fill_rectangle(g_aui->vbarx, sy, 4 * g_aui->msgfsize / 2, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR); /* 填充背景色 */
    buf = gui_memin_malloc(32);
    sprintf((char *)buf, "%d%%", pctx);
    gui_show_string(buf, g_aui->vbarx, sy, 4 * g_aui->msgfsize / 2, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR); /* 显示音量 */
    gui_memin_free(buf);
}

/**
 * @brief       显示audio播放时间
 * @param       sx,sy           : 起始坐标
 * @param       sec             : 时间
 * @retval      无
 */
void audio_time_show(uint16_t sx, uint16_t sy, uint16_t sec)
{
    uint16_t min;
    min = sec / 60; /* 得到分钟数 */

    if (min > 99)min = 99;

    sec = sec % 60; /* 得到秒钟数 */
    gui_phy.back_color = AUDIO_MAIN_BKCOLOR; /* 设置背景色为底色 */
    gui_show_num(sx, sy, 2, AUDIO_INFO_COLOR, g_aui->msgfsize, min, 0x80); /* 显示时间 */
    gui_show_ptchar(sx + g_aui->msgfsize, sy, lcddev.width, lcddev.height, 0, AUDIO_INFO_COLOR, g_aui->msgfsize, ':', 0); /* 显示冒号 */
    gui_show_num(sx + (g_aui->msgfsize / 2) * 3, sy, 2, AUDIO_INFO_COLOR, g_aui->msgfsize, sec, 0x80); /* 显示时间 */
}

/**
 * @brief       歌曲信息更新
 * @param       audiodevx       : audio控制器
 * @param       audioprgbx      : 进度条
 * @param       lrcx            : 歌词控制器
 * @retval      无
 */
void audio_info_upd(__audiodev *audiodevx, _progressbar_obj *audioprgbx, _progressbar_obj *volprgbx, _lyric_obj *lrcx)
{
    static uint16_t temp;
    uint16_t tempx, tempy;
    uint8_t *buf;
    float ftemp;

    if ((audiodevx->status & (1 << 5)) && (audiodevx->status & (1 << 1))) /* 执行了一次歌曲切换,且新音乐已经在播放了,更新歌曲名字和当前播放曲目索引,audioprgb长度等信息 */
    {
        audiodevx->status &= ~(1 << 5); /* 清空标志位 */
        buf = gui_memin_malloc(100);    /* 申请100字节内存 */

        if (buf == NULL)return;         /* game over */

        gui_fill_rectangle(0, g_aui->tpbar_height + g_aui->xygap - 1, lcddev.width, g_aui->msgfsize + 2, AUDIO_MAIN_BKCOLOR); /* 上下各多清空一点,清空之前的显示 */
        gui_show_ptstrwhiterim(g_aui->xygap, g_aui->tpbar_height + g_aui->xygap, lcddev.width - g_aui->xygap, lcddev.height, 0, 0X0000, 0XFFFF, g_aui->msgfsize, audiodevx->name);    /* 显示新的名字 */
        audiodevx->namelen = strlen((const char *)audiodevx->name); /* 得到所占字符的个数 */
        audiodevx->namelen *= 6;        /* 得到点数 */
        audiodevx->curnamepos = 0;      /* 得到点数 */
        gui_phy.back_color = AUDIO_MAIN_BKCOLOR; /* 设置背景色为底色 */

        /* 显示音量百分比 */
        audio_show_vol((volprgbx->curpos * 100) / volprgbx->totallen); /* 显示音量百分比 */

        /* 显示曲目编号 */
        sprintf((char *)buf, "%03d/%03d", audiodevx->curindex + 1, audiodevx->mfilenum);
        tempx = g_aui->vbarx + g_aui->vbarwidth - 7 * (g_aui->msgfsize) / 2;
        tempy = g_aui->vbary + g_aui->xygap / 2 + g_aui->vbarheight;
        gui_fill_rectangle(tempx, tempy, 7 * (g_aui->msgfsize) / 2, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR);           /* 清空之前的显示 */
        gui_show_string(buf, tempx, tempy, 7 * (g_aui->msgfsize) / 2, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR);
       
        /* 显示xxxKhz */
        tempx = g_aui->vbarx + g_aui->vbarwidth + g_aui->msgdis;
        gui_fill_rectangle(tempx, g_aui->vbary, 4 * g_aui->msgfsize, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR);          /* 清空之前的显示 */
        ftemp = (float)audiodevx->samplerate / 1000; /* xxx.xKhz */
        sprintf((char *)buf, "%3.1fKhz", ftemp);
        gui_show_string(buf, tempx, g_aui->vbary, 4 * g_aui->msgfsize, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR);
        
        /* 显示位数 */
        tempx = g_aui->vbarx + g_aui->vbarwidth + g_aui->msgdis + 4 * g_aui->msgfsize + g_aui->xygap;
        gui_fill_rectangle(tempx, g_aui->vbary, 5 * (g_aui->msgfsize) / 2, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR);    /* 清空之前的显示 */
        sprintf((char *)buf, "%02dbit", audiodevx->bps);
        gui_show_string(buf, tempx, g_aui->vbary, 5 * (g_aui->msgfsize) / 2, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR);
       
        /* 其他处理 */
        temp = 0;
        audioprgbx->totallen = audiodevx->file->obj.objsize;    /* 更新总长度 */
        audioprgbx->curpos = 0;

        if (lrcx)
        {
            lrc_read(lrcx, audiodevx->path, audiodevx->name);
            audio_lrc_bkcolor_process(lrcx, 1); /* 恢复背景色 */
            g_lrcdev->curindex = 0;     /* 重新设置歌词位置为0 */
            g_lrcdev->curtime = 0;      /* 重设时间 */
        }

        gui_memin_free(buf);/* 释放内存 */
    }

    if (audiodevx->namelen > lcddev.width - 8) /* 大于屏幕长度 */
    {
        gui_fill_rectangle(0, g_aui->tpbar_height + g_aui->xygap - 1, lcddev.width, g_aui->msgfsize + 2, AUDIO_MAIN_BKCOLOR); /* 上下各多清空一点,清空之前的显示 */
        gui_show_ptstrwhiterim(g_aui->xygap, g_aui->tpbar_height + g_aui->xygap, lcddev.width - g_aui->xygap, lcddev.height, audiodevx->curnamepos, 0X0000, 0XFFFF, g_aui->msgfsize, audiodevx->name);	/* 显示新的名字 */
        audiodevx->curnamepos++;

        if (audiodevx->curnamepos + lcddev.width - 8 > (audiodevx->namelen + lcddev.width / 2 - 10))audiodevx->curnamepos = 0; /* 循环显示 */
    }

    if (audiodevx->status & (1 << 7))       /* audio正在播放 */
    {
        audioprgbx->curpos = f_tell(audiodevx->file);   /* 得到当前的播放位置 */
        progressbar_draw_progressbar(audioprgbx);       /* 更新进度条位置 */

        if (temp != audiodevx->cursec)
        {
            temp = audiodevx->cursec;
            buf = gui_memin_malloc(100);    /* 申请100字节内存 */

            if (buf == NULL)return;         /* game over */

            /* 显示码率(Kbps ) */
            tempx = g_aui->vbarx + g_aui->vbarwidth + g_aui->msgdis;
            tempy = g_aui->vbary + g_aui->xygap / 2 + g_aui->vbarheight;
            gui_fill_rectangle(tempx, tempy, 4 * g_aui->msgfsize, g_aui->msgfsize, AUDIO_MAIN_BKCOLOR);                     /* 清空之前的显示 */
            sprintf((char *)buf, "%04dKbps", audiodevx->bitrate / 1000);
            gui_show_string(buf, tempx, tempy, 4 * g_aui->msgfsize, g_aui->msgfsize, g_aui->msgfsize, AUDIO_INFO_COLOR);    /* 显示分辨率 */
            gui_memin_free(buf);    /* 释放内存 */
            
            /* 显示时间 */
            tempx = g_aui->pbarx - 6 * g_aui->msgfsize / 2;
            audio_time_show(tempx, g_aui->pbary, audiodevx->cursec);    /* 显示播放时间 */
            tempx = g_aui->pbarx + g_aui->pbarwidth + g_aui->msgfsize / 2;
            audio_time_show(tempx, g_aui->pbary, audiodevx->totsec);    /* 显示总时间 */
        }
    }
}

/**
 * @brief       lrc背景色处理
 * @param       lrcx            : 歌词控制器
 * @param       mode            : 0,读取背景色
 *                                1,恢复背景色
 * @retval      无
 */
void audio_lrc_bkcolor_process(_lyric_obj *lrcx, uint8_t mode)
{
    uint16_t sy;
    uint16_t i;
    uint16_t imgheight = lcddev.height - (g_aui->tpbar_height + g_aui->msgbar_height + g_aui->prgbar_height + g_aui->btnbar_height);
    sy = g_aui->tpbar_height + g_aui->msgbar_height + (imgheight - 8 * g_aui->lrcdheight - 112) / 2;

    for (i = 0; i < 7; i++)
    {
        if (mode == 0) /* 读取背景色 */
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
 * @brief       显示歌词
 * @param       audiodevx       : audio控制器
 * @param       lrcx            : 歌词控制器
 * @retval      无
 */
void audio_lrc_show(__audiodev *audiodevx, _lyric_obj *lrcx)
{
    uint8_t t;
    uint16_t temp, temp1;
    uint16_t sy;
    uint16_t syadd;
    uint16_t imgheight = lcddev.height - (g_aui->tpbar_height + g_aui->msgbar_height + g_aui->prgbar_height + g_aui->btnbar_height);
    sy = g_aui->tpbar_height + g_aui->msgbar_height + (imgheight - 8 * g_aui->lrcdheight - 112) / 2;

    if (lrcx->oldostime != GUI_TIMER_10MS) /* 每10ms更新一下 */
    {
        t = gui_disabs(GUI_TIMER_10MS, lrcx->oldostime); /* 防止很久没有进入主程序导致的漏加 */
        lrcx->oldostime = GUI_TIMER_10MS;

        if (t > 10)t = 1;

        lrcx->curtime += t; /* 增加10ms */

        if (lrcx->indexsize) /* 有歌词存在 */
        {
            lrcx->detatime += t; /* 标志时间增加了10ms */

            if (lrcx->curindex < lrcx->indexsize) /* 还没显示完 */
            {
                if ((lrcx->curtime % 100) > 80) /* 1秒钟后过了800ms,需要查询解码时间寄存器以同步歌词 */
                {
                    lrcx->curtime = audiodevx->cursec * 100; /* 更新秒钟 */
                }

                if (lrcx->curtime >= lrcx->time_tbl[lrcx->curindex]) /* 当前时间超过了,需要更新歌词 */
                {
                    syadd = sy;
                    temp1 = lrcx->curindex; /* 备份当前lrcx->curindex */

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

                    for (t = 0; t < 7; t++)	/* 显示7条歌词 */
                    {
                        if (t != 3)lrcx->color = AUDIO_LRC_SCOLOR;
                        else lrcx->color = AUDIO_LRC_MCOLOR;

                        app_recover_bkcolor(20, syadd, lcddev.width - 40, 16, lrcx->lrcbkcolor[t]); /* 恢复背景色 */

                        if (lrcx->curindex <= (lrcx->indexsize - 1) && lrcx->curindex >= temp)
                        {
                            lrc_show_linelrc(lrcx, 20, syadd, lcddev.width - 40, 16); /* 显示歌词 */
                            lrcx->curindex++;
                        }

                        if (temp)temp--;

                        if (t == 2 || t == 3)syadd += 16 + g_aui->lrcdheight * 2;
                        else syadd += 16 + g_aui->lrcdheight;
                    }

                    lrcx->curindex = temp1; /* 恢复原来的值 */
                    lrc_show_linelrc(lrcx, 0, 0, 0, 0); /* 读取当前歌词,但是不显示 */
                    lrcx->curindex++;	/* 偏移到下一条歌词 */

                    if (lrcx->namelen > (lcddev.width - 40)) /* 需要滚动歌词 */
                    {
                        if (lrcx->curindex < lrcx->indexsize) /* 本句的下一句歌词还是存在的 */
                        {
                            temp = lrcx->time_tbl[lrcx->curindex - 1]; /* 当前歌词的时间 */
                            temp1 = lrcx->time_tbl[lrcx->curindex]; /* 下一句歌词的时间 */
                            lrcx->updatetime = (temp1 - temp) / (lrcx->namelen - 150); /* 计算得到滚动间隔时间,这里多了50个单位,因为前面的程序执行时间影响 */

                            if (lrcx->updatetime > 20)lrcx->updatetime = 20; /* 最大不超过200ms */
                        }
                        else lrcx->updatetime = 5; /* 默认滚动时间.50ms */
                    }
                }
            }

            if (lrcx->detatime >= lrcx->updatetime) /* 每lrcx->updatetime*10ms滚动显示当前歌词(如果需要滚动的话) */
            {
                if (lrcx->namelen > (lcddev.width - 40)) /* 超过了显示区域,需要滚动显示本句歌词 */
                {
                    syadd = sy + 3 * 16 + g_aui->lrcdheight * 4;
                    app_recover_bkcolor(20, syadd, lcddev.width - 40, 16, lrcx->lrcbkcolor[3]);
                    gui_show_ptstr(20, syadd, lcddev.width - 21, lcddev.height, lrcx->curnamepos, AUDIO_LRC_MCOLOR, lrcx->font, lrcx->buf, 1); /* 滚动显示本句歌词 */
                    lrcx->curnamepos++;

                    if (lrcx->curnamepos + 200 > lrcx->namelen + 50)lrcx->curnamepos = 0; /* 循环显示 */
                }

                lrcx->detatime = 0;
            }
        }
    }
}

/**
 * @brief       创建audio task
 * @param       无
 * @retval      0, 成功
 *              其他, 错误代码
 */
uint8_t audio_task_creat(void)
{
    OS_CPU_SR cpu_sr = 0;
    uint8_t res;
    uint8_t i;
    uint32_t size;
    g_lrcdev = lrc_creat(); /* 创建歌词管理结构体 */

    if (g_lrcdev)
    {
        g_lrcdev->font = 16;
        size = (lcddev.width - 40) * 16 * 2; /* 一条歌词背景的内存大小 */

        for (i = 0; i < 7; i++) /* 申请7条歌词的背景色表 */
        {
            g_lrcdev->lrcbkcolor[i] = gui_memex_malloc(size);

            if (g_lrcdev->lrcbkcolor[i] == NULL)break;
        }

        if (i != 7)             /* 内存申请失败 */
        {
            audio_task_delete();
            return 2;
        }
    }
    else return 1;  /* 创建失败 */

    AUDIO_PLAY_TASK_STK = gui_memin_malloc(AUDIO_PLAY_STK_SIZE * sizeof(OS_STK));

    if (AUDIO_PLAY_TASK_STK == 0)return 1;  /* 内存申请失败 */

    g_audiodev.status = 1 << 7;             /* 允许音频播放任务运行 */
    
    OS_ENTER_CRITICAL();/* 进入临界区(无法被中断打断) */
    res = OSTaskCreate(audio_play_task, (void *)0, (OS_STK *)&AUDIO_PLAY_TASK_STK[AUDIO_PLAY_STK_SIZE - 1], AUDIO_PLAY_TASK_PRIO);
    OS_EXIT_CRITICAL();	/* 退出临界区(可以被中断打断) */
    return res;
}

/**
 * @brief       删除audio_task
 * @param       无
 * @retval      无
 */
void audio_task_delete(void)
{
    uint8_t i;

    if (g_audiodev.status == 0)return;  /* 播放已经停止了 */

    g_audiodev.status = 0;              /* 播放停止 */
    gui_memin_free(g_audiodev.path);    /* 释放内存 */
    gui_memin_free(g_audiodev.mfindextbl);  /* 释放内存 */

    for (i = 0; i < 7; i++) /* 释放内存 */
    {
        gui_memex_free(g_lrcdev->lrcbkcolor[i]);
    }

    lrc_delete(g_lrcdev);       /* 释放歌词显示申请的内存 */
    g_lrcdev = NULL;            /* 指向空地址 */
    
    es8388_adda_cfg(0, 0);      /* 关闭DAC&ADC */
    es8388_input_cfg(0);        /* 关闭输入通道 */
    es8388_output_cfg(0, 0);    /* 关闭DAC输出 */
    app_es8388_volset(0);       /* 关闭ES8388音量输出 */
    
    gui_memin_free(AUDIO_PLAY_TASK_STK);    /* 释放内存 */
    OSTaskDel(AUDIO_PLAY_TASK_PRIO);        /* 删除音乐播放任务 */
}

/**
 * @brief       audio播放
 * @param       无
 * @retval      无
 */
uint8_t audio_play(void)
{
    uint8_t i;
    uint8_t res;
    uint8_t tcnt = 0;
    uint8_t rval = 0;       /* 1,内存错误;2,返回,audio继续播放;3,返回,停止audio播放 */
    uint16_t lastvolpos;
    uint8_t btnsize = 0;    /* 按钮尺寸 */
    uint8_t btnxpit = 0;    /* 按钮在x方向上的间隙 */
    _progressbar_obj *audioprgb, *volprgb;
    _btn_obj *tbtn[5];

    if ((g_audiodev.status & (1 << 7)) == 0)    /* 音频播放任务已经删除?/第一次进入? */
    {
        memset(&g_audiodev, 0, sizeof(__audiodev)); /* g_audiodev所有数据清零 */
        res = audio_task_creat();               /* 创建任务 */

        if (res == 0)res = audio_filelist(&g_audiodev); /* 选择音频文件进行播放 */

        if (res || g_audiodev.status == 0X80)   /* 创建任务失败/内存分配失败/没有选择音频播放 */
        {
            audio_task_delete();
            return 1;
        }

        system_task_return = 0;
    }
    else
    {
        g_audiodev.status |= 1 << 5;    /* 模拟一次切歌,以更新主界面内容 */
    }

    g_aui = (__audioui *)gui_memin_malloc(sizeof(__audioui));
    audio_load_ui(1);   /* 加载主界面 */
    audioprgb = progressbar_creat(g_aui->pbarx, g_aui->pbary, g_aui->pbarwidth, g_aui->pbarheight, 0X20);   /* audio播放进度条 */

    if (audioprgb == NULL)rval = 1;

    volprgb = progressbar_creat(g_aui->vbarx, g_aui->vbary, g_aui->vbarwidth, g_aui->vbarheight, 0X20);     /* 声音大小进度条 */

    if (volprgb == NULL)rval = 1;

    volprgb->totallen = 30;

    if (es8388set.mvol <= 30)
    {
        volprgb->curpos = es8388set.mvol;
    }
    else    /* 错误的数据 */
    {
        es8388set.mvol = 0;
        volprgb->curpos = 0;
    }

    lastvolpos = volprgb->curpos; /* 设定最近的位置 */

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

    for (i = 0; i < 5; i++) /* 循环创建5个按钮 */
    {
        tbtn[i] = btn_creat(btnxpit / 2 + i * (btnsize + btnxpit), lcddev.height - btnsize - (g_aui->btnbar_height - btnsize) / 2, btnsize, btnsize, 0, 1); /* 创建图片按钮 */

        if (tbtn[i] == NULL)
        {
            rval = 1;    /* 创建失败 */
            break;
        }

        tbtn[i]->bcfdcolor = 0X2CFF;    /* 按下时的背景色 */
        tbtn[i]->bcfucolor = AUDIO_BTN_BKCOLOR;	/* 松开时背景色 */
        tbtn[i]->picbtnpathu = (uint8_t *)AUDIO_BTN_PIC_TBL[0][i];
        tbtn[i]->picbtnpathd = (uint8_t *)AUDIO_BTN_PIC_TBL[1][i];
        tbtn[i]->sta = 0;
    }

    if (rval == 0) /* 没有错误 */
    {
        audioprgb->inbkcolora = 0x738E; /* 默认色 */
        audioprgb->inbkcolorb = AUDIO_INFO_COLOR;   /* 默认色 */
        audioprgb->infcolora = 0X75D;   /* 默认色 */
        audioprgb->infcolorb = 0X596;   /* 默认色 */
        volprgb->inbkcolora = AUDIO_INFO_COLOR;     /* 默认色 */
        volprgb->inbkcolorb = AUDIO_INFO_COLOR;     /* 默认色 */
        volprgb->infcolora = 0X75D;     /* 默认色 */
        volprgb->infcolorb = 0X596;     /* 默认色 */

        for (i = 0; i < 5; i++)btn_draw(tbtn[i]);   /* 画按钮 */

        tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PLAYR_PIC; /* 按下一次之后变为播放松开状态 */
        progressbar_draw_progressbar(audioprgb);    /* 画进度条 */
        progressbar_draw_progressbar(volprgb);      /* 画进度条 */
        system_task_return = 0;
        tcnt = 0;

        while (rval == 0)
        {
            tcnt++;/* 计时增加 */
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
            delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

            for (i = 0; i < 5; i++)
            {
                res = btn_check(tbtn[i], &in_obj);

                if ((res && ((tbtn[i]->sta & (1 << 7)) == 0) && (tbtn[i]->sta & (1 << 6))) || system_task_return) /* 有按键按下且松开,并且TP松开了或者TPAD返回 */
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
                            audio_load_ui(0);/* 重新加载主界面 */
                            g_audiodev.status |= 1 << 5; /* 模拟一次切歌,以更新主界面内容 */

                            if (g_audiodev.status & (1 << 0))   /* 继续播放? */
                            {
                                tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PAUSER_PIC;
                            }
                            else tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PLAYR_PIC;

                            for (i = 0; i < 5; i++)
                            {
                                btn_draw(tbtn[i]);   /* 画按钮 */
                            }
                            
                            if (g_audiodev.status & (1 << 0))
                            {
                                tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PLAYR_PIC;
                            }
                            else tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PAUSER_PIC;

                            progressbar_draw_progressbar(audioprgb);    /* 画进度条 */
                            progressbar_draw_progressbar(volprgb);      /* 画进度条 */

                            if (system_task_return)
                            {
                                delay_ms(10);
                                if (tpad_scan(1)) 
                                {
                                    break;  /* TPAD返回,再次确认,排除干扰 */
                                }
                                else system_task_return = 0;
                            }

                            break;

                        case 1:/* 上一曲或者下一曲 */
                        case 3:
                            audio_stop_req(&g_audiodev);

                            if (systemset.audiomode == 1) /* 随机播放 */
                            {
                                g_audiodev.curindex = app_get_rand(g_audiodev.mfilenum); /* 得到下一首歌曲的索引 */
                            }
                            else
                            {
                                if (i == 1) /* 上一曲 */
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

                            OSMboxPost(audiombox, (void *)(g_audiodev.curindex + 1)); /* 发送邮箱,因为邮箱不能为空,所以在这必须加1 */
                            break;

                        case 2:/* 播放/暂停 */
                            if (g_audiodev.status & (1 << 0)) /* 是暂停 */
                            {
                                g_audiodev.status &= ~(1 << 0); /* 标记暂停 */
                                tbtn[2]->picbtnpathd = (uint8_t *)AUDIO_PLAYP_PIC;
                                tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PAUSER_PIC;
                            }
                            else /* 暂停状态 */
                            {
                                g_audiodev.status |= 1 << 0; /* 取消暂停 */
                                tbtn[2]->picbtnpathd = (uint8_t *)AUDIO_PAUSEP_PIC;
                                tbtn[2]->picbtnpathu = (uint8_t *)AUDIO_PLAYR_PIC;
                            }

                            break;

                        case 4:/* 停止播放/返回 */
                            if ((g_audiodev.status & (1 << 0)) == 0) /* 暂停状态下按了返回 */
                            {
                                rval = 3; /* 退出播放,audio停止播放 */
                                audio_stop_req(&g_audiodev);/* 请求停止播放 */
                            }
                            else /* 暂停状态下按返回,那就关闭audio播放功能 */
                            {
                                rval = 2; /* 退出播放界面,audio继续播放 */
                            }

                            break;
                    }
                }
            }

            res = progressbar_check(volprgb, &in_obj); /* 检查音量进度条 */

            if (res && lastvolpos != volprgb->curpos) /* 被按下了,且位置变化了.执行音量调整 */
            {
                lastvolpos = volprgb->curpos;

                if (volprgb->curpos)es8388set.mvol = volprgb->curpos; /* 设置音量 */
                else es8388set.mvol = 0;

                app_es8388_volset(es8388set.mvol);
                audio_show_vol((volprgb->curpos * 100) / volprgb->totallen);	/* 显示音量百分比 */
            }

            res = progressbar_check(audioprgb, &in_obj);

            if (res && ((audioprgb->sta && PRGB_BTN_DOWN) == 0)) /* 被按下了,并且松开了,执行快进快退 */
            {
                //printf("audioprgb->curpos:%d\r\n",audioprgb->curpos);
                g_lrcdev->curindex = 0; /* 重新设置歌词位置为0 */
                g_lrcdev->curtime = 0;  /* 重设时间 */
                audioprgb->curpos = g_audiodev.file_seek(audioprgb->curpos); /* 快进快退 */
            }

            if ((tcnt % 100) == 0)audio_info_upd(&g_audiodev, audioprgb, volprgb, g_lrcdev); /* 更新显示信息,每100ms执行一次 */

            if (g_lrcdev != NULL && ((g_audiodev.status & (1 << 5)) == 0) && (g_audiodev.status & (1 << 7))) /* 正在播放,不是暂停,且歌词结构体正常 */
            {
                audio_lrc_show(&g_audiodev, g_lrcdev);  /* 可以显示歌词 */

            }
        }
    }

    for (i = 0; i < 5; i++)btn_delete(tbtn[i]); /* 删除按钮 */


    progressbar_delete(audioprgb);
    progressbar_delete(volprgb);
    gui_memin_free(g_aui);

    if (rval == 3)  /* 退出audio播放.且不后台播放 */

    {
        audio_task_delete();    /* 删除音频播放任务 */
    }

    return rval;
}
