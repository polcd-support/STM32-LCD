/**
 ****************************************************************************************************
 * @file        mp3play.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       mp3解码 代码
 *              1, 支持16位单声道/立体声MP3的解码
 *              2, 支持CBR/VBR格式MP3解码
 *              3, 支持ID3V1和ID3V2标签解析
 *              4, 支持所有比特率(MP3最高是320Kbps)解码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
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
 * V1.0 20221127
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "mp3play.h"
#include "audioplay.h"
#include "string.h"
#include "./SYSTEM/sys/sys.h"
#include "./FATFS/source/ff.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LED/led.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/ES8388/es8388.h"


__mp3ctrl *mp3ctrl;                     /* mp3控制结构体 */
volatile uint8_t mp3transferend = 0;    /* i2s传输完成标志 */
volatile uint8_t mp3witchbuf = 0;       /* i2sbufx指示标志 */

/**
 * @brief       MP3 DMA发送回调函数
 * @param       无
 * @retval      无
 */
void mp3_sai_dma_tx_callback(void)
{
    uint16_t i;

    if (DMA2_Stream3->CR & (1 << 19))
    {
        mp3witchbuf = 0;

        if ((g_audiodev.status & 0X01) == 0) /* 暂停了,填充0 */
        {
            for (i = 0; i < 2304 * 2; i++)g_audiodev.saibuf1[i] = 0;
        }
    }
    else
    {
        mp3witchbuf = 1;

        if ((g_audiodev.status & 0X01) == 0) /* 暂停了,填充0 */
        {
            for (i = 0; i < 2304 * 2; i++)g_audiodev.saibuf2[i] = 0;
        }
    }

    mp3transferend = 1;
}

/**
 * @brief       填充PCM数据到DAC
 * @param       buf             : PCM数据首地址
 * @param       size            : pcm数据量(16位为单位)
 * @param       nch             : 声道数(1,单声道,2立体声)
 * @retval      无
 */
void mp3_fill_buffer(uint16_t *buf, uint16_t size, uint8_t nch)
{
    uint16_t i;
    uint16_t *p;

    while (mp3transferend == 0) /* 等待传输完成 */
    {
        delay_ms(1000 / OS_TICKS_PER_SEC);
    };

    mp3transferend = 0;

    if (mp3witchbuf == 0)
    {
        p = (uint16_t *)g_audiodev.saibuf1;
    }
    else
    {
        p = (uint16_t *)g_audiodev.saibuf2;
    }

    if (nch == 2)
    {
        for (i = 0; i < size; i++)
        {
            p[i] = buf[i];
        }
    }
    else    /* 单声道 */
    {
        for (i = 0; i < size; i++)
        {
            p[2 * i] = buf[i];
            p[2 * i + 1] = buf[i];
        }
    }
}

/**
 * @brief       解析ID3V1
 * @param       buf             : 输入数据缓存区(大小固定是128字节)
 * @param       pctrl           : MP3控制器
 * @retval      0, 正常
 *              其他, 失败
 */
uint8_t mp3_id3v1_decode(uint8_t *buf, __mp3ctrl *pctrl)
{
    ID3V1_Tag *id3v1tag;
    id3v1tag = (ID3V1_Tag *)buf;

    if (strncmp("TAG", (char *)id3v1tag->id, 3) == 0) /* 是MP3 ID3V1 TAG */
    {
        if (id3v1tag->title[0])strncpy((char *)pctrl->title, (char *)id3v1tag->title, 30);

        if (id3v1tag->artist[0])strncpy((char *)pctrl->artist, (char *)id3v1tag->artist, 30);
    }
    else return 1;

    return 0;
}

/**
 * @brief       解析ID3V2
 * @param       buf             : 输入数据缓存区(大小固定是128字节)
 * @param       size            : 数据大小
 * @param       pctrl           : MP3控制器
 * @retval      0, 正常
 *              其他, 失败
 */
uint8_t mp3_id3v2_decode(uint8_t *buf, uint32_t size, __mp3ctrl *pctrl)
{
    ID3V2_TagHead *taghead;
    ID3V23_FrameHead *framehead;
    uint32_t t;
    uint32_t tagsize;       /* tag大小 */
    uint32_t frame_size;    /* 帧大小 */
    taghead = (ID3V2_TagHead *)buf;

    if (strncmp("ID3", (const char *)taghead->id, 3) == 0) /* 存在ID3? */
    {
        tagsize = ((uint32_t)taghead->size[0] << 21) | ((uint32_t)taghead->size[1] << 14) | ((uint16_t)taghead->size[2] << 7) | taghead->size[3]; /* 得到tag 大小 */
        pctrl->datastart = tagsize;         /* 得到mp3数据开始的偏移量 */

        if (tagsize > size)tagsize = size;  /* tagsize大于输入bufsize的时候,只处理输入size大小的数据 */

        if (taghead->mversion < 3)
        {
            printf("not supported mversion!\r\n");
            return 1;
        }

        t = 10;

        while (t < tagsize)
        {
            framehead = (ID3V23_FrameHead *)(buf + t);
            frame_size = ((uint32_t)framehead->size[0] << 24) | ((uint32_t)framehead->size[1] << 16) | ((uint32_t)framehead->size[2] << 8) | framehead->size[3]; /* 得到帧大小 */

            if (strncmp("TT2", (char *)framehead->id, 3) == 0 || strncmp("TIT2", (char *)framehead->id, 4) == 0) /* 找到歌曲标题帧,不支持unicode格式!! */
            {
                strncpy((char *)pctrl->title, (char *)(buf + t + sizeof(ID3V23_FrameHead) + 1), AUDIO_MIN(frame_size - 1, MP3_TITSIZE_MAX - 1));
            }

            if (strncmp("TP1", (char *)framehead->id, 3) == 0 || strncmp("TPE1", (char *)framehead->id, 4) == 0) /* 找到歌曲艺术家帧 */
            {
                strncpy((char *)pctrl->artist, (char *)(buf + t + sizeof(ID3V23_FrameHead) + 1), AUDIO_MIN(frame_size - 1, MP3_ARTSIZE_MAX - 1));
            }

            t += frame_size + sizeof(ID3V23_FrameHead);
        }
    }
    else pctrl->datastart = 0; /* 不存在ID3,mp3数据是从0开始 */

    return 0;
}

/**
 * @brief       获取MP3基本信息
 * @param       pname           : MP3文件路径
 * @param       pctrl           : MP3控制器
 * @retval      0, 成功
 *              其他, 失败
 */
uint8_t mp3_get_info(uint8_t *pname, __mp3ctrl *pctrl)
{
    HMP3Decoder decoder;
    MP3FrameInfo frame_info;
    MP3_FrameXing *fxing;
    MP3_FrameVBRI *fvbri;
    FIL *fmp3;
    uint8_t *buf;
    uint32_t br;
    uint8_t res;
    int offset = 0;
    uint32_t p;
    short samples_per_frame;    /* 一帧的采样个数 */
    uint32_t totframes;         /* 总帧数 */

    fmp3 = mymalloc(SRAMIN, sizeof(FIL));
    buf = mymalloc(SRAMIN, 5 * 1024);   /* 申请5K内存 */

    if (fmp3 && buf) /* 内存申请成功 */
    {
        f_open(fmp3, (const TCHAR *)pname, FA_READ); /* 打开文件 */
        res = f_read(fmp3, (char *)buf, 5 * 1024, &br);

        if (res == 0) /* 读取文件成功,开始解析ID3V2/ID3V1以及获取MP3信息 */
        {
            mp3_id3v2_decode(buf, br, pctrl);           /* 解析ID3V2数据 */
            f_lseek(fmp3, fmp3->obj.objsize - 128);     /* 偏移到倒数128的位置 */
            f_read(fmp3, (char *)buf, 128, &br);        /* 读取128字节 */
            mp3_id3v1_decode(buf, pctrl);               /* 解析ID3V1数据 */
            decoder = MP3InitDecoder();                 /* MP3解码申请内存 */
            f_lseek(fmp3, pctrl->datastart);            /* 偏移到数据开始的地方 */
            f_read(fmp3, (char *)buf, 5 * 1024, &br);   /* 读取5K字节mp3数据 */
            offset = MP3FindSyncWord(buf, br);          /* 查找帧同步信息 */

            if (offset >= 0 && MP3GetNextFrameInfo(decoder, &frame_info, &buf[offset]) == 0) /* 找到帧同步信息了,且下一阵信息获取正常 */
            {
                p = offset + 4 + 32;
                fvbri = (MP3_FrameVBRI *)(buf + p);

                if (strncmp("VBRI", (char *)fvbri->id, 4) == 0) /* 存在VBRI帧(VBR格式) */
                {
                    if (frame_info.version == MPEG1)samples_per_frame = 1152; /* MPEG1,layer3每帧采样数等于1152 */
                    else samples_per_frame = 576; /* MPEG2/MPEG2.5,layer3每帧采样数等于576 */

                    totframes = ((uint32_t)fvbri->frames[0] << 24) | ((uint32_t)fvbri->frames[1] << 16) | ((uint16_t)fvbri->frames[2] << 8) | fvbri->frames[3]; /* 得到总帧数 */
                    pctrl->totsec = totframes * samples_per_frame / frame_info.samprate; /* 得到文件总长度 */
                }
                else    /* 不是VBRI帧,尝试是不是Xing帧(VBR格式) */
                {
                    if (frame_info.version == MPEG1)    /* MPEG1 */
                    {
                        p = frame_info.nChans == 2 ? 32 : 17;
                        samples_per_frame = 1152;       /* MPEG1,layer3每帧采样数等于1152 */
                    }
                    else
                    {
                        p = frame_info.nChans == 2 ? 17 : 9;
                        samples_per_frame = 576;        /* MPEG2/MPEG2.5,layer3每帧采样数等于576 */
                    }

                    p += offset + 4;
                    fxing = (MP3_FrameXing *)(buf + p);

                    if (strncmp("Xing", (char *)fxing->id, 4) == 0 || strncmp("Info", (char *)fxing->id, 4) == 0) /* 是Xng帧 */
                    {
                        if (fxing->flags[3] & 0X01)     /* 存在总frame字段 */
                        {
                            totframes = ((uint32_t)fxing->frames[0] << 24) | ((uint32_t)fxing->frames[1] << 16) | ((uint16_t)fxing->frames[2] << 8) | fxing->frames[3]; /* 得到总帧数 */
                            pctrl->totsec = totframes * samples_per_frame / frame_info.samprate; /* 得到文件总长度 */
                        }
                        else    /* 不存在总frames字段 */
                        {
                            pctrl->totsec = fmp3->obj.objsize / (frame_info.bitrate / 8);
                        }
                    }
                    else        /* CBR格式,直接计算总播放时间 */
                    {
                        pctrl->totsec = fmp3->obj.objsize / (frame_info.bitrate / 8);
                    }
                }

                pctrl->bitrate = frame_info.bitrate;        /* 得到当前帧的码率 */
                mp3ctrl->samplerate = frame_info.samprate;  /* 得到采样率. */

                if (frame_info.nChans == 2)mp3ctrl->outsamples = frame_info.outputSamps; /* 输出PCM数据量大小 */
                else mp3ctrl->outsamples = frame_info.outputSamps * 2; /* 输出PCM数据量大小,对于单声道MP3,直接*2,补齐为双声道输出 */
            }
            else res = 0XFE; /* 未找到同步帧 */

            MP3FreeDecoder(decoder);/* 释放内存 */
        }

        f_close(fmp3);
    }
    else res = 0XFF;

    myfree(SRAMIN, fmp3);
    myfree(SRAMIN, buf);
    return res;
}

/**
 * @brief       得到当前播放时间
 * @param       fx              : 文件指针
 * @param       mp3x            : mp3播放控制器
 * @retval      无
 */
void mp3_get_curtime(FIL *fx, __mp3ctrl *mp3x)
{
    uint32_t fpos = 0;

    if (fx->fptr > mp3x->datastart)fpos = fx->fptr - mp3x->datastart;           /* 得到当前文件播放到的地方 */

    mp3x->cursec = fpos * mp3x->totsec / (fx->obj.objsize - mp3x->datastart);   /* 当前播放到第多少秒了? */
}

/**
 * @brief       mp3文件快进快退函数
 * @param       pos             : 需要定位到的文件位置
 * @retval      当前文件位置(即定位后的结果)
 */
uint32_t mp3_file_seek(uint32_t pos)
{
    if (pos > g_audiodev.file->obj.objsize)
    {
        pos = g_audiodev.file->obj.objsize;
    }

    f_lseek(g_audiodev.file, pos);
    return g_audiodev.file->fptr;
}

/**
 * @brief       播放一曲MP3音乐
 * @param       pname           : MP3文件路径
 * @retval      0,正常播放完成
 *              [b7]:0,正常状态;1,错误状态
 *              [b6:0]:b7=0时,表示操作码
 *                     b7=1时,表示有错误(这里不判定具体错误,0X80~0XFF,都算是错误)
 */
uint8_t mp3_play_song(uint8_t *fname)
{
    HMP3Decoder mp3decoder;
    MP3FrameInfo mp3frameinfo;
    uint8_t res;
    uint8_t *buffer;    /* 输入buffer */
    uint8_t *readptr;   /* MP3解码读指针 */
    int offset = 0;     /* 偏移量 */
    int outofdata = 0;  /* 超出数据范围 */
    int bytesleft = 0;  /* buffer还剩余的有效数据 */
    uint32_t br = 0;
    int err = 0;

    mp3ctrl = mymalloc(SRAMIN, sizeof(__mp3ctrl));
    buffer = mymalloc(SRAMIN, MP3_FILE_BUF_SZ); /* 申请解码buf大小 */
    g_audiodev.file = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    g_audiodev.saibuf1 = mymalloc(SRAMIN, 2304 * 2);
    g_audiodev.saibuf2 = mymalloc(SRAMIN, 2304 * 2);
    g_audiodev.tbuf = mymalloc(SRAMIN, 2304 * 2);
    g_audiodev.file_seek = mp3_file_seek;

    if (!mp3ctrl || !buffer || !g_audiodev.file || !g_audiodev.saibuf1 || !g_audiodev.saibuf2 || !g_audiodev.tbuf) /* 内存申请失败 */
    {
        myfree(SRAMIN, mp3ctrl);
        myfree(SRAMIN, buffer);
        myfree(SRAMIN, g_audiodev.file);
        myfree(SRAMIN, g_audiodev.saibuf1);
        myfree(SRAMIN, g_audiodev.saibuf2);
        myfree(SRAMIN, g_audiodev.tbuf);
        return AP_ERR;  /* 错误 */
    }

    memset(g_audiodev.saibuf1, 0, 2304 * 2);    /* 数据清零 */
    memset(g_audiodev.saibuf2, 0, 2304 * 2);    /* 数据清零 */
    memset(mp3ctrl, 0, sizeof(__mp3ctrl));      /* 数据清零 */
    res = mp3_get_info(fname, mp3ctrl);

    if (res == 0)
    {
        printf("     title:%s\r\n", mp3ctrl->title);
        printf("    artist:%s\r\n", mp3ctrl->artist);
        printf("   bitrate:%dbps\r\n", mp3ctrl->bitrate);
        printf("samplerate:%d\r\n", mp3ctrl->samplerate);
        printf("  totalsec:%d\r\n", mp3ctrl->totsec);

        es8388_sai_cfg(0, 3);   /* 飞利浦标准,16位数据长度 */
        sai1_saia_init(0,1,4);      /* 飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度 */
        sai1_samplerate_set(mp3ctrl->samplerate);    /* 设置采样率 */
        sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, mp3ctrl->outsamples,1);    /* 配置TX DMA */
        sai1_tx_callback = mp3_sai_dma_tx_callback;  /* 回调函数指mp3_sai_dma_tx_callback */

        mp3decoder = MP3InitDecoder();              /* MP3解码申请内存 */
        res = f_open(g_audiodev.file, (char *)fname, FA_READ);  /* 打开文件 */
    }

    if (res == 0 && mp3decoder != 0) /* 打开文件成功 */
    {
        f_lseek(g_audiodev.file, mp3ctrl->datastart);   /* 跳过文件头中tag信息 */
        audio_start();          /* 开始播放 */

        while (res == 0)
        {
            readptr = buffer;   /* MP3读指针指向buffer */
            offset = 0;         /* 偏移量为0 */
            outofdata = 0;      /* 数据正常 */
            bytesleft = 0;
            res = f_read(g_audiodev.file, buffer, MP3_FILE_BUF_SZ, &br); /* 一次读取MP3_FILE_BUF_SZ字节 */

            if (res) /* 读数据出错了 */
            {
                res = AP_ERR;
                break;
            }

            if (br == 0)        /* 读数为0,说明解码完成了 */
            {
                res = AP_OK;    /* 播放完成 */
                break;
            }

            bytesleft += br;    /* buffer里面有多少有效MP3数据? */
            err = 0;

            while (!outofdata)  /* 没有出现数据异常(即可否找到帧同步字符) */
            {
                offset = MP3FindSyncWord(readptr, bytesleft); /* 在readptr位置,开始查找同步字符 */

                if (offset < 0) /* 没有找到同步字符,跳出帧解码循环 */
                {
                    outofdata = 1; /* 没找到帧同步字符 */
                }
                else    /* 找到同步字符了 */
                {
                    readptr += offset;      /* MP3读指针偏移到同步字符处 */
                    bytesleft -= offset;    /* buffer里面的有效数据个数,必须减去偏移量 */
                    err = MP3Decode(mp3decoder, &readptr, &bytesleft, (short *)g_audiodev.tbuf, 0); /* 解码一帧MP3数据 */

                    if (err != 0)
                    {
                        printf("decode error:%d\r\n", err);
                        break;
                    }
                    else
                    {
                        MP3GetLastFrameInfo(mp3decoder, &mp3frameinfo); /* 得到刚刚解码的MP3帧信息 */

                        if (mp3ctrl->bitrate != mp3frameinfo.bitrate)   /* 更新码率 */
                        {
                            mp3ctrl->bitrate = mp3frameinfo.bitrate;
                        }

                        mp3_fill_buffer((uint16_t *)g_audiodev.tbuf, mp3frameinfo.outputSamps, mp3frameinfo.nChans); /* 填充pcm数据 */
                    }

                    if (bytesleft < MAINBUF_SIZE * 2) /* 当数组内容小于2倍MAINBUF_SIZE的时候,必须补充新的数据进来 */
                    {
                        memmove(buffer, readptr, bytesleft); /* 移动readptr所指向的数据到buffer里面,数据量大小为:bytesleft */
                        f_read(g_audiodev.file, buffer + bytesleft, MP3_FILE_BUF_SZ - bytesleft, &br); /* 补充余下的数据 */

                        if (br < MP3_FILE_BUF_SZ - bytesleft)
                        {
                            memset(buffer + bytesleft + br, 0, MP3_FILE_BUF_SZ - bytesleft - br);
                        }

                        bytesleft = MP3_FILE_BUF_SZ;
                        readptr = buffer;
                    }

                    while (g_audiodev.status & (1 << 1)) /* 正常播放中 */
                    {
                        delay_ms(1000 / OS_TICKS_PER_SEC);
                        mp3_get_curtime(g_audiodev.file, mp3ctrl);
                        g_audiodev.totsec = mp3ctrl->totsec;    /* 参数传递 */
                        g_audiodev.cursec = mp3ctrl->cursec;
                        g_audiodev.bitrate = mp3ctrl->bitrate;
                        g_audiodev.samplerate = mp3ctrl->samplerate;
                        g_audiodev.bps = 16;                /* MP3仅支持16位 */

                        if (g_audiodev.status & 0X01)break; /* 没有按下暂停 */
                    }

                    if ((g_audiodev.status & (1 << 1)) == 0)/* 请求结束播放/播放完成 */
                    {
                        res = AP_NEXT;  /* 跳出上上级循环 */
                        outofdata = 1;  /* 跳出上一级循环 */
                        break;
                    }
                }
            }
        }

        audio_stop();   /* 关闭音频输出 */
    }
    else res = AP_ERR;  /* 错误 */

    f_close(g_audiodev.file);
    MP3FreeDecoder(mp3decoder); /* 释放内存 */
    myfree(SRAMIN, mp3ctrl);
    myfree(SRAMIN, buffer);
    myfree(SRAMIN, g_audiodev.file);
    myfree(SRAMIN, g_audiodev.saibuf1);
    myfree(SRAMIN, g_audiodev.saibuf2);
    myfree(SRAMIN, g_audiodev.tbuf);
    return res;
}








