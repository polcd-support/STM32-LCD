/**
 ****************************************************************************************************
 * @file        videoplay.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-18
 * @brief       视频播放器 应用代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220118
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./APP/videoplayer.h"
#include "./FATFS/source/ff.h"
#include "./MALLOC/malloc.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/I2S/i2s.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/KEY/key.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./TEXT/text.h"
#include "./FATFS/source/ffconf.h"
#include "./FATFS/exfuns/fattester.h"
#include "./MJPEG/mjpeg.h"
#include "./MJPEG/avi.h"
#include "./BSP/TIMER/btim.h"
#include "string.h"


extern uint16_t g_avi_frame;                /* 播放帧率 */
extern volatile uint8_t g_avi_frameup;      /* 视频播放时隙控制变量,当等于1的时候,可以更新下一帧视频 */

volatile uint8_t g_avi_i2s_playbuf;         /* 即将播放的音频帧缓冲编号 */
uint8_t *p_avi_i2s_buf[AVI_AUDIO_BUF_NUM];  /* 音频缓冲帧 */


/**
 * @brief       音频数据I2S DMA传输回调函数
 * @param       无
 * @retval      无
 */
void audio_i2s_dma_callback(void)
{
    g_avi_i2s_playbuf++;

    if (g_avi_i2s_playbuf >= AVI_AUDIO_BUF_NUM)   /* 限定范围 */
    {
        g_avi_i2s_playbuf = 0;
    }

    if (I2S_TX_DMASx->CR & (1 << 19))
    {
        I2S_TX_DMASx->M0AR = (uint32_t)p_avi_i2s_buf[g_avi_i2s_playbuf];   /* 指向下一个buf */
    }
    else
    {
        I2S_TX_DMASx->M1AR = (uint32_t)p_avi_i2s_buf[g_avi_i2s_playbuf];   /* 指向下一个buf */
    }
}

/**
 * @brief       得到path路径下,目标文件的总个数
 * @param       path:路径
 * @retval      返回值:总有效文件数
 */
uint16_t video_get_tnum(char *path)
{
    uint8_t res;
    uint16_t rval = 0;
    DIR tdir;           /* 临时目录 */
    FILINFO *tfileinfo; /* 临时文件信息 */

    tfileinfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));   /* 申请内存 */
    res = f_opendir(&tdir, (const TCHAR *)path);    /* 打开目录 */

    if (res == FR_OK && tfileinfo)
    {
        while (1)       /* 查询总的有效文件数 */
        {
            res = f_readdir(&tdir, tfileinfo);                  /* 读取目录下的一个文件 */

            if (res != FR_OK || tfileinfo->fname[0] == 0)
            {
                break; /* 错误了/到末尾了,退出 */
            }

            res = exfuns_file_type(tfileinfo->fname);

            if ((res & 0XF0) == 0X60)   /* 取高四位,看看是不是视频文件 */
            {
                rval++;                 /* 有效文件数增加1 */
            }
        }
    }

    myfree(SRAMIN, tfileinfo);          /* 释放内存 */
    return rval;
}

/**
 * @brief       显示当前播放时间
 * @param       favi    : 当前播放的视频文件
 * @param       aviinfo : avi控制结构体
 * @retval      无
 */
void video_time_show(FIL *favi, AVI_INFO *aviinfo)
{
    static uint32_t oldsec; /* 上一次的播放时间 */
    uint8_t *buf;
    uint32_t totsec = 0;    /* video文件总时间 */
    uint32_t cursec;        /* 当前播放时间 */

    totsec = (aviinfo->SecPerFrame / 1000) * aviinfo->TotalFrame;   /* 歌曲总长度(单位:ms) */
    totsec /= 1000;         /* 秒钟数. */
    cursec = ((double)favi->fptr / favi->obj.objsize) * totsec;     /* 当前播放到多少秒了? */

    if (oldsec != cursec)   /* 需要更新显示时间 */
    {
        buf = mymalloc(SRAMIN, 100); /* 申请100字节内存 */
        oldsec = cursec;
        sprintf((char *)buf, "播放时间:%02d:%02d:%02d/%02d:%02d:%02d", cursec / 3600, (cursec % 3600) / 60, cursec % 60, totsec / 3600, (totsec % 3600) / 60, totsec % 60);
        text_show_string(10, 90, lcddev.width - 10, 16, (char *)buf, 16, 0, RED);   /* 显示歌曲名字 */
        myfree(SRAMIN, buf);
    }
}

/**
 * @brief       显示当前视频文件的相关信息
 * @param       aviinfo : avi控制结构体
 * @retval      无
 */
void video_info_show(AVI_INFO *aviinfo)
{
    uint8_t *buf;

    buf = mymalloc(SRAMIN, 100);    /* 申请100字节内存 */

    sprintf((char *)buf, "声道数:%d,采样率:%d", aviinfo->Channels, aviinfo->SampleRate * 10);
    text_show_string(10, 50, lcddev.width - 10, 16, (char *)buf, 16, 0, RED);   /* 显示歌曲名字 */

    sprintf((char *)buf, "帧率:%d帧", 1000 / (aviinfo->SecPerFrame / 1000));
    text_show_string(10, 70, lcddev.width - 10, 16, (char *)buf, 16, 0, RED);   /* 显示歌曲名字 */

    myfree(SRAMIN, buf);            /* 释放内存 */
}

/**
 * @brief       视频基本信息显示
 * @param       name    : 视频名字
 * @param       index   : 当前索引
 * @param       total   : 总文件数
 * @retval      无
 */
void video_bmsg_show(uint8_t *name, uint16_t index, uint16_t total)
{
    uint8_t *buf;

    buf = mymalloc(SRAMIN, 100);    /* 申请100字节内存 */

    sprintf((char *)buf, "文件名:%s", name);
    text_show_string(10, 10, lcddev.width - 10, 16, (char *)buf, 16, 0, RED);   /* 显示文件名 */

    sprintf((char *)buf, "索引:%d/%d", index, total);
    text_show_string(10, 30, lcddev.width - 10, 16, (char *)buf, 16, 0, RED);   /* 显示索引 */

    myfree(SRAMIN, buf);            /* 释放内存 */
}

/**
 * @brief       播放视频
 * @param       无
 * @retval      无
 */
void video_play(void)
{
    uint8_t res;
    DIR vdir;               /* 目录 */
    FILINFO *vfileinfo;     /* 文件信息 */
    uint8_t *pname;         /* 带路径的文件名 */
    uint16_t totavinum;     /* 视频文件总数 */
    uint16_t curindex;      /* 视频文件当前索引 */
    uint8_t key;            /* 键值 */
    uint32_t temp;
    uint32_t *voffsettbl;   /* 视频文件off block索引表 */

    while (f_opendir(&vdir, "0:/VIDEO"))    /* 打开视频文件夹 */
    {
        text_show_string(60, 190, 240, 16, "VIDEO文件夹错误!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(60, 190, 240, 206, WHITE); /* 清除显示 */
        delay_ms(200);
    }

    totavinum = video_get_tnum("0:/VIDEO"); /* 得到总有效文件数 */

    while (totavinum == NULL)               /* 视频文件总数为0 */
    {
        text_show_string(60, 190, 240, 16, "没有视频文件!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(60, 190, 240, 146, WHITE); /* 清除显示 */
        delay_ms(200);
    }

    vfileinfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));   /* 为长文件缓存区分配内存 */
    pname = mymalloc(SRAMIN, 2 * FF_MAX_LFN + 1);               /* 为带路径的文件名分配内存 */
    voffsettbl = mymalloc(SRAMIN, 4 * totavinum);               /* 申请4*totavinum个字节的内存,用于存放视频文件索引 */

    while (vfileinfo == NULL || pname == NULL || voffsettbl == NULL)    /* 内存分配出错 */
    {
        text_show_string(60, 190, 240, 16, "内存分配失败!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(60, 190, 240, 146, WHITE); /* 清除显示 */
        delay_ms(200);
    }

    /* 记录索引 */
    res = f_opendir(&vdir, "0:/VIDEO");     /* 打开目录 */

    if (res == FR_OK)
    {
        curindex = 0;   /* 当前索引为0 */

        while (1)       /* 全部查询一遍 */
        {
            temp = vdir.dptr;                                   /* 记录当前offset */
            res = f_readdir(&vdir, vfileinfo);                  /* 读取目录下的一个文件 */

            if (res != FR_OK || vfileinfo->fname[0] == 0)
            {
                break; /* 错误了/到末尾了,退出 */
            }

            res = exfuns_file_type(vfileinfo->fname);

            if ((res & 0XF0) == 0X60)           /* 取高四位,看看是不是音乐文件 */
            {
                voffsettbl[curindex] = temp;    /* 记录索引 */
                curindex++;
            }
        }
    }

    curindex = 0;           /* 从0开始显示 */
    res = f_opendir(&vdir, (const TCHAR *)"0:/VIDEO");  /* 打开目录 */

    while (res == FR_OK)    /* 打开成功 */
    {
        dir_sdi(&vdir, voffsettbl[curindex]);   /* 改变当前目录索引 */
        res = f_readdir(&vdir, vfileinfo);      /* 读取目录下的一个文件 */

        if (res != FR_OK || vfileinfo->fname[0] == 0)
        {
            break;          /* 错误了/到末尾了,退出 */
        }

        strcpy((char *)pname, "0:/VIDEO/");                     /* 复制路径(目录) */
        strcat((char *)pname, (const char *)vfileinfo->fname);  /* 将文件名接在后面 */
        lcd_clear(WHITE);   /* 先清屏 */
        video_bmsg_show((uint8_t *)vfileinfo->fname, curindex + 1, totavinum);  /* 显示名字,索引等信息 */
        key = video_play_mjpeg(pname);  /* 播放这个音频文件 */

        if (key == KEY2_PRES)           /* 上一个视频 */
        {
            if (curindex)
            {
                curindex--;
            }
            else
            {
                curindex = totavinum - 1;
            }
        }
        else if (key == KEY0_PRES)     /* 下一个视频 */
        {
            curindex++;

            if (curindex >= totavinum)
            {
                curindex = 0;     /* 到末尾的时候,自动从头开始 */
            }
        }
        else
        {
            break;     /* 产生了错误 */
        }
    }

    myfree(SRAMIN, vfileinfo);      /* 释放内存 */
    myfree(SRAMIN, pname);          /* 释放内存 */
    myfree(SRAMIN, voffsettbl);     /* 释放内存 */
}

/**
 * @brief       播放一个mjpeg文件
 * @param       pname : 文件名
 * @retval      res
 *   @arg       KEY0_PRES , 下一个视频.
 *   @arg       KEY1_PRES , 上一个视频.
 *   @arg       其他 , 错误
 */
uint8_t video_play_mjpeg(uint8_t *pname)
{
    uint8_t *framebuf;      /* 视频解码buf */
    uint8_t *pbuf;          /* buf指针 */
    FIL *favi;
    uint8_t  res = 0;
    uint8_t i = 0;
    uint16_t offset = 0;
    uint32_t nr;
    uint8_t key;
    uint8_t i2ssavebuf;

    for(i = 0; i < AVI_AUDIO_BUF_NUM; i++)
    {
        p_avi_i2s_buf[i] = mymalloc(SRAMIN, AVI_AUDIO_BUF_SIZE);   /* 申请音频内存 */
        
        if(p_avi_i2s_buf[i] == NULL)                                /* 申请失败, 直接退出 */
        {
            break;
        }
        
        memset(p_avi_i2s_buf[0], 0, AVI_AUDIO_BUF_SIZE);            /* 数据清零 */
    }

    favi = (FIL *)mymalloc(SRAMIN, sizeof(FIL));                    /* 申请favi内存 */
    framebuf = mymalloc(SRAMIN, AVI_VIDEO_BUF_SIZE) ;               /* 申请视频buf */

    if (!framebuf)  /* 只要最后这个视频buf申请失败, 前面的申请失不失败都不重要, 总之就是失败了 */
    {
        printf("memory error!\r\n");
        res = 0XFF;
    }

    while (res == 0)
    {
        res = f_open(favi, (char *)pname, FA_READ);

        if (res == 0)
        {
            pbuf = framebuf;
            res = f_read(favi, pbuf, AVI_VIDEO_BUF_SIZE, &nr);  /* 开始读取 */

            if (res)
            {
                printf("fread error:%d\r\n", res);
                break;
            }

            /* 开始avi解析 */
            res = avi_init(pbuf, AVI_VIDEO_BUF_SIZE);   /* avi解析 */

            if (res)
            {
                printf("avi err:%d\r\n", res);
                break;
            }

            video_info_show(&g_avix);
            btim_tim7_int_init(g_avix.SecPerFrame / 100 - 1, 8400 - 1);     /* 10Khz计数频率,加1是100us */
            offset = avi_srarch_id(pbuf, AVI_VIDEO_BUF_SIZE, "movi");       /* 寻找movi ID */
            avi_get_streaminfo(pbuf + offset + 4);  /* 获取流信息 */
            f_lseek(favi, offset + 12);             /* 跳过标志ID,读地址偏移到流数据开始处 */
            res = mjpegdec_init((lcddev.width - g_avix.Width) / 2, 110 + (lcddev.height - 110 - g_avix.Height) / 2);    /* JPG解码初始化 */

            if (g_avix.SampleRate)  /* 有音频信息,才初始化 */
            {
                i2s_init(0,2,0,1);  /* 飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度 */
                i2s_samplerate_set(g_avix.SampleRate);      /* 设置采样率 */
                i2s_tx_dma_init(p_avi_i2s_buf[0], p_avi_i2s_buf[1], g_avix.AudioBufSize / 2);     /* 配置DMA */
                i2s_tx_callback = audio_i2s_dma_callback;   /* 回调函数指向I2S_DMA_Callback */
                g_avi_i2s_playbuf = 0;
                i2ssavebuf = 0;
                i2s_play_start();   /* 开启I2S播放 */
            }

            while (1)   /* 播放循环 */
            {
                if (g_avix.StreamID == AVI_VIDS_FLAG)       /* 视频流 */
                {
                    pbuf = framebuf;
                    f_read(favi, pbuf, g_avix.StreamSize + 8, &nr);   /* 读入整帧+下一数据流ID信息 */
                    res = mjpegdec_decode(pbuf, g_avix.StreamSize);

                    if (res)
                    {
                        printf("decode error!\r\n");
                    }

                    while (g_avi_frameup == 0);   /* 待时间到达(在TIM6的中断里面设置为1) */

                    g_avi_frameup = 0;            /* 标志清零 */
                    g_avi_frame++;
                }
                else     /* 音频流 */
                {
                    video_time_show(favi, &g_avix);   /* 显示当前播放时间 */
                    i2ssavebuf++;

                    if (i2ssavebuf > 3)
                    {
                        i2ssavebuf = 0;
                    }

                    do
                    {
                        nr = g_avi_i2s_playbuf;

                        if (nr)
                        {
                            nr--;
                        }
                        else
                        {
                            nr = 3;
                        }
                    } while (i2ssavebuf == nr); /* 碰撞等待. */

                    f_read(favi, p_avi_i2s_buf[i2ssavebuf], g_avix.StreamSize + 8, &nr); /* 填充p_avi_i2s_buf */
                    pbuf = p_avi_i2s_buf[i2ssavebuf];
                }

                key = key_scan(0);

                if (key == KEY0_PRES || key == KEY2_PRES)   /* KEY0/KEY2按下,播放下一个/上一个视频 */
                {
                    res = key;
                    break;
                }
                else if (key == KEY1_PRES || key == WKUP_PRES)
                {
                    i2s_play_stop();    /* 关闭音频 */
                    video_seek(favi, &g_avix, framebuf);
                    pbuf = framebuf;
                    i2s_play_start();   /* 开启DMA播放 */
                }

                if (avi_get_streaminfo(pbuf + g_avix.StreamSize))     /* 读取下一帧 流标志 */
                {
                    printf("frame error \r\n");
                    res = KEY0_PRES;
                    break;
                }
            }

            i2s_play_stop();                                    /* 关闭音频 */
            TIM7->CR1 &= ~(1 << 0);                             /* 关闭定时器7 */
            lcd_set_window(0, 0, lcddev.width, lcddev.height);  /* 恢复窗口 */
            mjpegdec_free();                                    /* 释放内存 */
            f_close(favi);                                      /* 关闭文件 */
        }
    }

    /* 释放内存 */
    for(i = 0; i < AVI_AUDIO_BUF_NUM; i++)
    {
        myfree(SRAMIN, p_avi_i2s_buf[i]);
    }
    
    myfree(SRAMIN, framebuf);
    myfree(SRAMIN, favi);

    return res;
}

/**
 * @brief       AVI文件查找
 * @param       favi    : AVI文件
 * @param       aviinfo : AVI信息结构体
 * @param       mbuf    : 数据缓冲区
 * @retval      执行结果
 *   @arg       0    , 成功
 *   @arg       其他 , 错误
 */
uint8_t video_seek(FIL *favi, AVI_INFO *aviinfo, uint8_t *mbuf)
{
    uint32_t fpos = favi->fptr;
    uint8_t *pbuf;
    uint16_t offset;
    uint32_t br;
    uint32_t delta;
    uint32_t totsec;
    uint8_t key;

    totsec = (aviinfo->SecPerFrame / 1000) * aviinfo->TotalFrame;
    totsec /= 1000;     /* 秒钟数 */
    delta = (favi->obj.objsize / totsec) * 5;   /* 每次前进5秒钟的数据量 */

    while (1)
    {
        key = key_scan(1);

        if (key == WKUP_PRES)       /* 快进 */
        {
            if (fpos < favi->obj.objsize)
            {
                fpos += delta;
            }

            if (fpos > (favi->obj.objsize - AVI_VIDEO_BUF_SIZE))
            {
                fpos = favi->obj.objsize - AVI_VIDEO_BUF_SIZE;
            }
        }
        else if (key == KEY1_PRES)     /* 快退 */
        {
            if (fpos > delta)
            {
                fpos -= delta;
            }
            else
            {
                fpos = 0;
            }
        }
        else
        {
            break;
        }

        f_lseek(favi, fpos);
        f_read(favi, mbuf, AVI_VIDEO_BUF_SIZE, &br);    /* 读入整帧+下一数据流ID信息 */
        pbuf = mbuf;

        if (fpos == 0)   /* 从0开始,得先寻找movi ID */
        {
            offset = avi_srarch_id(pbuf, AVI_VIDEO_BUF_SIZE, "movi");
        }
        else
        {
            offset = 0;
        }

        offset += avi_srarch_id(pbuf + offset, AVI_VIDEO_BUF_SIZE, g_avix.VideoFLAG); /* 寻找视频帧 */
        avi_get_streaminfo(pbuf + offset);  /* 获取流信息 */
        f_lseek(favi, fpos + offset + 8);   /* 跳过标志ID,读地址偏移到流数据开始处 */

        if (g_avix.StreamID == AVI_VIDS_FLAG)
        {
            f_read(favi, mbuf, g_avix.StreamSize + 8, &br);   /* 读入整帧 */
            mjpegdec_decode(mbuf, g_avix.StreamSize);         /* 显示视频帧 */
        }
        else
        {
            printf("error flag");
        }

        video_time_show(favi, &g_avix);
    }

    return 0;
}























