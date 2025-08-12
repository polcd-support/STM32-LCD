/**
 ****************************************************************************************************
 * @file        flacplay.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       flac解码 代码
 *              1, 支持16/24位单声道/立体声flac的解码
 *              2, 最高支持192K/16bit或96K/24bit的flac解码
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

#include "flacplay.h"
#include "audioplay.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "./BSP/KEY/key.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/ES8388/es8388.h"


__flacctrl *flacctrl;   /* flac解码控制结构体 */


/**
 * @brief       分析FLAC文件
 * @param       fx              : 文件指针
 * @param       fc              : flac解码容器
 * @retval      0, 成功
 *              其他, 错误代码
 */
uint8_t flac_init(FIL *fx, __flacctrl *fctrl, FLACContext *fc)
{
    FLAC_Tag *flactag;
    MD_Block_Head *flacblkh;
    uint8_t *buf;
    uint8_t endofmetadata = 0;      /* 最后一个metadata标记 */
    int blocklength;
    uint32_t br;
    uint8_t res;

    buf = mymalloc(SRAMIN, 512);    /* 申请512字节内存 */

    if (!buf)return 1;              /* 内存申请失败 */

    f_lseek(fx, 0);                 /* 偏移到文件头 */
    f_read(fx, buf, 4, &br);        /* 读取4字节 */
    flactag = (FLAC_Tag *)buf;      /* 强制转换为flac tag标签 */

    if (strncmp("fLaC", (char *)flactag->id, 4) != 0)
    {
        myfree(SRAMIN, buf);        /* 释放内存 */
        return 2;                   /* 非flac文件 */
    }

    while (!endofmetadata)
    {
        f_read(fx, buf, 4, &br);

        if (br < 4)break;

        flacblkh = (MD_Block_Head *)buf;
        endofmetadata = flacblkh->head & 0X80;  /* 判断是不是最后一个block? */
        blocklength = ((uint32_t)flacblkh->size[0] << 16) | ((uint16_t)flacblkh->size[1] << 8) | (flacblkh->size[2]); /* 得到块大小 */

        if ((flacblkh->head & 0x7f) == 0)       /* head最低7位为0,则表示是STREAMINFO块 */
        {
            res = f_read(fx, buf, blocklength, &br);

            if (res != FR_OK)break;

            fc->min_blocksize = ((uint16_t)buf[0] << 8) | buf[1];   /* 最小块大小 */
            fc->max_blocksize = ((uint16_t)buf[2] << 8) | buf[3];   /* 最大块大小 */
            fc->min_framesize = ((uint32_t)buf[4] << 16) | ((uint16_t)buf[5] << 8) | buf[6];                    /* 最小帧大小 */
            fc->max_framesize = ((uint32_t)buf[7] << 16) | ((uint16_t)buf[8] << 8) | buf[9];                    /* 最大帧大小 */
            fc->samplerate = ((uint32_t)buf[10] << 12) | ((uint16_t)buf[11] << 4) | ((buf[12] & 0xf0) >> 4);    /* 采样率 */
            fc->channels = ((buf[12] & 0x0e) >> 1) + 1;             /* 音频通道数 */
            fc->bps = ((((uint16_t)buf[12] & 0x01) << 4) | ((buf[13] & 0xf0) >> 4)) + 1;                        /* 采样位数16?24?32? */
            fc->totalsamples = ((uint32_t)buf[14] << 24) | ((uint32_t)buf[15] << 16) | ((uint16_t)buf[16] << 8) | buf[17];  /* 一个声道的总采样数 */
            fctrl->samplerate = fc->samplerate;
            fctrl->totsec = (fc->totalsamples / fc->samplerate);    /* 得到总时间 */
        }
        else    /* 忽略其他帧的处理 */
        {
            if (f_lseek(fx, fx->fptr + blocklength) != FR_OK)
            {
                myfree(SRAMIN, buf);
                return 3;
            }
        }
    }

    myfree(SRAMIN, buf); /* 释放内存 */

    if (fctrl->totsec)
    {
        fctrl->outsamples = fc->max_blocksize * 2;  /* PCM输出数据量(*2,表示2个声道的数据量) */
        fctrl->bps = fc->bps;                       /* 采样位数(16/24/32) */
        fctrl->datastart = fx->fptr;                /* FLAC数据帧开始的地址 */
        fctrl->bitrate = ((fx->obj.objsize - fctrl->datastart) * 8) / fctrl->totsec; /* 得到FLAC的位速 */
    }
    else return 4;  /* 总时间为0?有问题的flac文件 */

    return 0;
}

volatile uint8_t flactransferend = 0;   /* i2s传输完成标志 */
volatile uint8_t flacwitchbuf = 0;      /* i2sbufx指示标志 */

/**
 * @brief       FLAC DMA发送回调函数
 * @param       无
 * @retval      无
 */
void flac_i2s_dma_tx_callback(void)
{
    uint16_t i;
    uint16_t size;

    if (DMA1_Stream4->CR & (1 << 19))
    {
        flacwitchbuf = 0;

        if ((g_audiodev.status & 0X01) == 0) /* 暂停了,填充0 */
        {
            if (flacctrl->bps == 24)size = flacctrl->outsamples * 4;
            else size = flacctrl->outsamples * 2;

            for (i = 0; i < size; i++)g_audiodev.saibuf1[i] = 0;
        }
    }
    else
    {
        flacwitchbuf = 1;

        if ((g_audiodev.status & 0X01) == 0) /* 暂停了,填充0 */
        {
            if (flacctrl->bps == 24)size = flacctrl->outsamples * 4;
            else size = flacctrl->outsamples * 2;

            for (i = 0; i < size; i++)g_audiodev.saibuf2[i] = 0;
        }
    }

    flactransferend = 1;
}

/**
 * @brief       得到当前播放时间
 * @param       fx              : 文件指针
 * @param       flacctrl        : flac播放控制器
 * @retval      无
 */
void flac_get_curtime(FIL *fx, __flacctrl *flacctrl)
{
    long long fpos = 0;

    if (fx->fptr > flacctrl->datastart)fpos = fx->fptr - flacctrl->datastart;               /* 得到当前文件播放到的地方 */

    flacctrl->cursec = fpos * flacctrl->totsec / (fx->obj.objsize - flacctrl->datastart);   /* 当前播放到第多少秒了? */
}

/**
 * @brief       flac文件快进快退函数
 * @param       pos             : 需要定位到的文件位置
 * @retval      当前文件位置(即定位后的结果)
 */
uint32_t flac_file_seek(uint32_t pos)
{
    if (pos > g_audiodev.file->obj.objsize)
    {
        pos = g_audiodev.file->obj.objsize;
    }

    f_lseek(g_audiodev.file, pos);
    return g_audiodev.file->fptr;
}

/**
 * @brief       播放一曲FLAC音乐
 * @param       pname           : FLAC文件路径
 * @retval      0,正常播放完成
 *              [b7]:0,正常状态;1,错误状态
 *              [b6:0]:b7=0时,表示操作码
 *                     b7=1时,表示有错误(这里不判定具体错误,0X80~0XFF,都算是错误)
 */
uint8_t flac_play_song(uint8_t *fname)
{
    FLACContext *fc = 0;
    int bytesleft;
    int consumed;
    uint8_t res = 0;
    uint32_t br = 0;
    uint8_t *buffer = 0;
    uint8_t *decbuf0 = 0;
    uint8_t *decbuf1 = 0;
    uint8_t *p8 = 0;
    uint32_t flac_fptr = 0;

    fc = mymalloc(SRAMIN, sizeof(FLACContext));
    flacctrl = mymalloc(SRAMIN, sizeof(__flacctrl));
    g_audiodev.file = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    g_audiodev.file_seek = flac_file_seek;

    if (!fc || !g_audiodev.file || !flacctrl)res = 1;   /* 内存申请错误 */
    else
    {
        memset(fc, 0, sizeof(FLACContext));             /* fc所有内容清零 */
        res = f_open(g_audiodev.file, (char *)fname, FA_READ);  /* 读取文件错误 */

        if (res == FR_OK)
        {
            res = flac_init(g_audiodev.file, flacctrl, fc);     /* flac解码初始化 */

            if (fc->min_blocksize == fc->max_blocksize && fc->max_blocksize != 0) /* 必须min_blocksize等于max_blocksize */
            {
                if (fc->bps == 24)  /* 24位音频数据 */
                {
                    g_audiodev.saibuf1 = mymalloc(SRAMIN, fc->max_blocksize * 8);
                    g_audiodev.saibuf2 = mymalloc(SRAMIN, fc->max_blocksize * 8);
                }
                else    /* 16位音频数据 */
                {
                    g_audiodev.saibuf1 = mymalloc(SRAMIN, fc->max_blocksize * 4);
                    g_audiodev.saibuf2 = mymalloc(SRAMIN, fc->max_blocksize * 4);
                }
                
                if(fc->max_framesize == 0)  /* 对帧大小不确定的flac, 设置默认帧大小(内存充足可以适当设大一点) */
                {
                    fc->max_framesize = 0X4000;             /* 申请解码帧缓存 */
                    
                }
                
                buffer = mymalloc(SRAMDTCM, fc->max_framesize);  /* 申请解码帧缓存 */
                decbuf0 = mymalloc(SRAMDTCM, fc->max_blocksize * 4);
                decbuf1 = mymalloc(SRAMDTCM, fc->max_blocksize * 4);
            }
            else
            {
                res += 1; /* 不支持的音频格式 */
            }
        }
    }

    if (buffer && g_audiodev.saibuf1 && g_audiodev.saibuf2 && decbuf0 && decbuf1 && res == 0)
    {
        printf("\r\n  Blocksize: %d .. %d\r\n", fc->min_blocksize, fc->max_blocksize);
        printf("  Framesize: %d .. %d\r\n", fc->min_framesize, fc->max_framesize);
        printf("  Samplerate: %d\r\n", fc->samplerate);
        printf("  Channels: %d\r\n", fc->channels);
        printf("  Bits per sample: %d\r\n", fc->bps);
        printf("  Metadata length: %d\r\n", flacctrl->datastart);
        printf("  Total Samples: %lu\r\n", fc->totalsamples);
        printf("  Duration: %d s\r\n", flacctrl->totsec);
        printf("  Bitrate: %d kbps\r\n", flacctrl->bitrate);

        if (flacctrl->bps == 24)    /* 24位音频数据 */
        {
            es8388_sai_cfg(2, 2);   /* 飞利浦标准,24位数据长度（同样的配置WM8978可以正常，ES8388 24bit异常，有待解决！） */
            sai1_saia_init(0,1,6);      /* 飞利浦标准,主机发送,时钟低电平有效,24位扩展帧长度 */
            sai1_samplerate_set(flacctrl->samplerate);  /* 设置采样率 */
            sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, flacctrl->outsamples , 2);  /* 配置TX DMA */
            
            memset(g_audiodev.saibuf1, 0, fc->max_blocksize * 8);
            memset(g_audiodev.saibuf2, 0, fc->max_blocksize * 8);
        }
        else    /* 16位音频数据 */
        {
            es8388_sai_cfg(2, 0);   /* 飞利浦标准,16位数据长度 */
            sai1_saia_init(0,1,4);      /* 飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度 */
            sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, flacctrl->outsamples,1);      /* 配置TX DMA */
            
            memset(g_audiodev.saibuf1, 0, fc->max_blocksize * 4);
            memset(g_audiodev.saibuf2, 0, fc->max_blocksize * 4);
        }

        sai1_samplerate_set(fc->samplerate); /* 设置采样率 */
        sai1_tx_callback = flac_i2s_dma_tx_callback;                 /* 回调函数指向flac_i2s_dma_tx_callback */
        f_read(g_audiodev.file, buffer, fc->max_framesize, &br);    /* 读取最大帧长数据 */
        
        bytesleft = br;
        audio_start();                      /* 开始播放 */
        fc->decoded0 = (int *)decbuf0;      /* 解码数组0 */
        fc->decoded1 = (int *)decbuf1;      /* 解码数组1 */
        flac_fptr = g_audiodev.file->fptr;  /* 记录当前的文件位置 */

        while (bytesleft)
        {
            while (flactransferend == 0) /* 等待传输完成 */
            {
                delay_ms(1000 / OS_TICKS_PER_SEC);
            };

            if (flac_fptr != g_audiodev.file->fptr) /* 说明外部有进行文件快进/快退操作 */
            {
                if (g_audiodev.file->fptr < flacctrl->datastart) /* 在数据开始之前?? */
                {
                    f_lseek(g_audiodev.file, flacctrl->datastart); /* 偏移到数据开始的地方 */
                }

                f_read(g_audiodev.file, buffer, fc->max_framesize, &br); /* 读取一个最大帧的数据量 */
                bytesleft = flac_seek_frame(buffer, br, fc);    /* 查找帧 */

                if (bytesleft >= 0) /* 找到正确的帧头 */
                {
                    f_lseek(g_audiodev.file, g_audiodev.file->fptr - fc->max_framesize + bytesleft);
                    f_read(g_audiodev.file, buffer, fc->max_framesize, &br);
                }
                else printf("flac seek error:%d\r\n", bytesleft);

                bytesleft = br;
            }

            flactransferend = 0;

            if (flacwitchbuf == 0)p8 = g_audiodev.saibuf1;
            else p8 = g_audiodev.saibuf2;

            if (fc->bps == 24)res = flac_decode_frame24(fc, buffer, bytesleft, (s32 *)p8);
            else res = flac_decode_frame16(fc, buffer, bytesleft, (s16 *)p8);

            if (res != 0) /* 解码出错了 */
            {
                res = AP_ERR;
                break;
            }

            consumed = fc->gb.index / 8;
            memmove(buffer, &buffer[consumed], bytesleft - consumed);
            bytesleft -= consumed;
            res = f_read(g_audiodev.file, &buffer[bytesleft], fc->max_framesize - bytesleft, &br);

            if (res) /* 读数据出错了 */
            {
                res = AP_ERR;
                break;
            }

            if (br > 0)
            {
                bytesleft += br;
            }

            flac_fptr = g_audiodev.file->fptr;      /* 记录当前的文件位置 */

            while (g_audiodev.status & (1 << 1))    /* 正常播放中 */
            {
                flac_get_curtime(g_audiodev.file, flacctrl);    /* 得到总时间和当前播放的时间 */
                g_audiodev.totsec = flacctrl->totsec;           /* 参数传递 */
                g_audiodev.cursec = flacctrl->cursec;
                g_audiodev.bitrate = flacctrl->bitrate;
                g_audiodev.samplerate = flacctrl->samplerate;
                g_audiodev.bps = flacctrl->bps;

                if (g_audiodev.status & 0X01)break;     /* 没有按下暂停 */
                else delay_ms(1000 / OS_TICKS_PER_SEC);
            }

            if ((g_audiodev.status & (1 << 1)) == 0)    /* 请求结束播放/播放完成 */
            {
                break;
            }
        }

        audio_stop();
    }
    else res = AP_ERR;

    f_close(g_audiodev.file);
    myfree(SRAMIN, fc);
    myfree(SRAMIN, flacctrl);
    myfree(SRAMIN, g_audiodev.file);
    myfree(SRAMIN, g_audiodev.saibuf1);
    myfree(SRAMIN, g_audiodev.saibuf2);
    myfree(SRAMDTCM, buffer);
    myfree(SRAMDTCM, decbuf0);
    myfree(SRAMDTCM, decbuf1);
    return res;
}









































