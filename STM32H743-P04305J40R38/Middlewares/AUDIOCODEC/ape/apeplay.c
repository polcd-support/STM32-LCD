/**
 ****************************************************************************************************
 * @file        apeplay.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       ape解码 代码
 *              1, 支持16位单声道/立体声ape的解码
 *              2, 最高支持96K的APE格式(LV1和LV2)
 *              3, LV1~LV3,在48K及以下流畅播放,LV4,LV5大卡
 *              4, 对某些ape文件,可能不支持,请用Monkey's Audio软件进行转换一下,即可正常播放
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

#include <apeplay.h>
#include "string.h"
#include "./SYSTEM/sys/sys.h"
#include "./FATFS/source/ff.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "./BSP/KEY/key.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/ES8388/es8388.h"
#include "audioplay.h"


__apectrl *apectrl;	/* APE播放控制结构体 */

/* apedecoder.c里面需要的数组 */
extern filter_int *filterbuf64;         /* 需要2816字节 */

volatile uint8_t apetransferend = 0;    /* i2s传输完成标志 */
volatile uint8_t apewitchbuf = 0;       /* i2sbufx指示标志 */


/**
 * @brief       APE DMA发送回调函数
 * @param       无
 * @retval      无
 */
void ape_i2s_dma_tx_callback(void)
{
    uint16_t i;

    if (DMA1_Stream4->CR & (1 << 19))
    {
        apewitchbuf = 0;

        if ((g_audiodev.status & 0X01) == 0) /* 暂停了,填充0 */
        {
            for (i = 0; i < APE_BLOCKS_PER_LOOP * 4; i++)g_audiodev.saibuf1[i] = 0;
        }
    }
    else
    {
        apewitchbuf = 1;

        if ((g_audiodev.status & 0X01) == 0) /* 暂停了,填充0 */
        {
            for (i = 0; i < APE_BLOCKS_PER_LOOP * 4; i++)g_audiodev.saibuf2[i] = 0;
        }
    }

    apetransferend = 1;
}

/**
 * @brief       填充PCM数据到DAC
 * @param       buf             : PCM数据首地址
 * @param       size            : pcm数据量(32位为单位)
 * @retval      无
 */
void ape_fill_buffer(uint32_t *buf, uint16_t size)
{
    uint16_t i;
    uint32_t *p;

    while (apetransferend == 0)delay_ms(1000 / OS_TICKS_PER_SEC); /* 等待传输完成 */

    apetransferend = 0;

    if (apewitchbuf == 0)
    {
        p = (uint32_t *)g_audiodev.saibuf1;
    }
    else
    {
        p = (uint32_t *)g_audiodev.saibuf2;
    }

    for (i = 0; i < size; i++)p[i] = buf[i];
}

/**
 * @brief       得到当前播放时间
 * @param       fx              : 文件指针
 * @param       apectrl         : apectrl播放控制器
 * @retval      无
 */
void ape_get_curtime(FIL *fx, __apectrl *apectrl)
{
    long long fpos = 0;

    if (fx->fptr > apectrl->datastart)fpos = fx->fptr - apectrl->datastart;             /* 得到当前文件播放到的地方 */

    apectrl->cursec = fpos * apectrl->totsec / (fx->obj.objsize - apectrl->datastart);  /* 当前播放到第多少秒了? */
}

/**
 * @brief       ape 文件快进快退函数
 * @param       pos             : 需要定位到的文件位置
 * @retval      当前文件位置(即定位后的结果)
 */
uint32_t ape_file_seek(uint32_t pos)
{
    return g_audiodev.file->fptr;/* ape文件不支持快进快退,直接返回当前播放位置 */
}

/**
 * @brief       播放一曲APE音乐
 * @param       pname           : APE 文件路径
 * @retval      0,正常播放完成
 *              [b7]:0,正常状态;1,错误状态
 *              [b6:0]:b7=0时,表示操作码
 *                     b7=1时,表示有错误(这里不判定具体错误,0X80~0XFF,都算是错误)
 */
uint8_t ape_play_song(uint8_t *fname)
{
    struct ape_ctx_t *apex;

    int currentframe;
    int nblocks;
    int bytesconsumed;
    int bytesinbuffer;
    int blockstodecode;
    int firstbyte;
    int n;

    uint8_t res = AP_ERR;
    uint8_t *readptr;
    uint8_t *buffer;
    int *decoded0;
    int *decoded1;
    uint32_t totalsamples;

    filterbuf64 = mymalloc(SRAMIN, 2816);
    apectrl = mymalloc(SRAMIN, sizeof(__apectrl));
    apex = mymalloc(SRAMIN, sizeof(struct ape_ctx_t));
    decoded0 = mymalloc(SRAMDTCM, APE_BLOCKS_PER_LOOP * 4);
    decoded1 = mymalloc(SRAMDTCM, APE_BLOCKS_PER_LOOP * 4);
    g_audiodev.file = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    g_audiodev.saibuf1 = mymalloc(SRAMIN, APE_BLOCKS_PER_LOOP * 4);
    g_audiodev.saibuf2 = mymalloc(SRAMIN, APE_BLOCKS_PER_LOOP * 4);
    g_audiodev.file_seek = ape_file_seek;
    buffer = mymalloc(SRAMIN, APE_FILE_BUF_SZ);

    if (filterbuf64 && apectrl && apex && decoded0 && decoded1 && g_audiodev.file && g_audiodev.saibuf1 && g_audiodev.saibuf2 && buffer) /* 所有内存申请都OK */
    {
        memset(apex, 0, sizeof(struct ape_ctx_t));
        memset(apectrl, 0, sizeof(__apectrl));
        memset(g_audiodev.saibuf1, 0, APE_BLOCKS_PER_LOOP * 4);
        memset(g_audiodev.saibuf2, 0, APE_BLOCKS_PER_LOOP * 4);
        f_open(g_audiodev.file, (char *)fname, FA_READ);    /* 打开文件 */
        res = ape_parseheader(g_audiodev.file, apex);       /* 分析ape文件头 */

        if (res == 0)
        {
            if ((apex->compressiontype > 3000) || (apex->fileversion < APE_MIN_VERSION) || (apex->fileversion > APE_MAX_VERSION || apex->bps != 16))
            {
                res = AP_ERR; /* 压缩率不支持/版本不支持/不是16位音频格式 */
            }
            else
            {
                apectrl->bps = apex->bps;   /* 得到采样深度(ape,我们仅支持16位) */
                apectrl->samplerate = apex->samplerate;     /* 得到采样率,ape支持48Khz以下的采样率,在高就可能卡了... */

                if (apex->totalframes > 1)totalsamples = apex->finalframeblocks + apex->blocksperframe * (apex->totalframes - 1);
                else totalsamples = apex->finalframeblocks;

                apectrl->totsec = totalsamples / apectrl->samplerate;   /* 得到文件总时长 */
                apectrl->bitrate = (g_audiodev.file->obj.objsize - apex->firstframe) * 8 / apectrl->totsec; /* 得到位速 */
                apectrl->outsamples = APE_BLOCKS_PER_LOOP * 2;  /* PCM输出数据量(16位为单位) */
                apectrl->datastart = apex->firstframe;      /* 得到第一帧的地址 */
            }
        }
    }

    if (res == 0)
    {
        printf("  Samplerate: %d\r\n", apectrl->samplerate);
        printf("  Bits per sample: %d\r\n", apectrl->bps);
        printf("  First frame pos: %d\r\n", apectrl->datastart);
        printf("  Duration: %d s\r\n", apectrl->totsec);
        printf("  Bitrate: %d kbps\r\n", apectrl->bitrate);
        //ape_dumpinfo(apex);

        es8388_sai_cfg(2, 0);   /* 飞利浦标准,16位数据长度 */
        sai1_saia_init(0,1,4);      /* 飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度 */
        sai1_samplerate_set(apex->samplerate);       /* 设置采样率 */
        sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, APE_BLOCKS_PER_LOOP * 2,1);    /* 配置TX DMA */
        sai1_tx_callback = ape_i2s_dma_tx_callback;  /* 回调函数指ape_i2s_dma_tx_callback */

        currentframe = 0;
        f_lseek(g_audiodev.file, apex->firstframe);
        res = f_read(g_audiodev.file, buffer, APE_FILE_BUF_SZ, (uint32_t *)&bytesinbuffer);
        firstbyte = 3;          /* Take account of the little-endian 32-bit byte ordering */
        readptr = buffer;
        audio_start();

        while (currentframe < apex->totalframes && res == 0) /* 还有未解码的帧? */
        {
            /* 计算一帧里面有多少个blocks? */
            if (currentframe == (apex->totalframes - 1))nblocks = apex->finalframeblocks;
            else nblocks = apex->blocksperframe;

            apex->currentframeblocks = nblocks;
            /* 初始化帧解码 */
            init_frame_decoder(apex, readptr, &firstbyte, &bytesconsumed);
            readptr += bytesconsumed;
            bytesinbuffer -= bytesconsumed;

            while (nblocks > 0) /* 开始帧解码 */
            {
                blockstodecode = AUDIO_MIN(APE_BLOCKS_PER_LOOP, nblocks); /* 获得一次要解码的blocks个数 */
                res = decode_chunk(apex, readptr, &firstbyte, &bytesconsumed, decoded0, decoded1, blockstodecode);

                if (res != 0)
                {
                    printf("frame decode err\r\n");
                    res = AP_ERR;
                    break;
                }

                ape_fill_buffer((uint32_t *)decoded1, APE_BLOCKS_PER_LOOP);
                readptr += bytesconsumed;           /* 解码指针偏移到新数据位置 */
                bytesinbuffer -= bytesconsumed;     /* buffer里面的数据量减少 */

                if (bytesconsumed > 4 * APE_BLOCKS_PER_LOOP) /* 出现错误了 */
                {
                    nblocks = 0;
                    res = AP_ERR;
                    printf("bytesconsumed:%d\r\n", bytesconsumed);
                }

                if (bytesinbuffer < 4 * APE_BLOCKS_PER_LOOP) /* 需要补充新数据了 */
                {
                    memmove(buffer, readptr, bytesinbuffer);
                    res = f_read(g_audiodev.file, buffer + bytesinbuffer, APE_FILE_BUF_SZ - bytesinbuffer, (uint32_t *)&n);

                    if (res) /* 出错了 */
                    {
                        res = AP_ERR;
                        break;
                    }

                    bytesinbuffer += n;
                    readptr = buffer;
                }

                nblocks -= blockstodecode; /* block计数递减 */

                while (g_audiodev.status & (1 << 1)) /* 正常播放中 */
                {
                    ape_get_curtime(g_audiodev.file, apectrl);  /* 得到总时间和当前播放的时间 */
                    g_audiodev.totsec = apectrl->totsec;        /* 参数传递 */
                    g_audiodev.cursec = apectrl->cursec;
                    g_audiodev.bitrate = apectrl->bitrate;
                    g_audiodev.samplerate = apectrl->samplerate;
                    g_audiodev.bps = apectrl->bps;
                    delay_ms(1000 / OS_TICKS_PER_SEC);

                    if (g_audiodev.status & 0X01)break;         /* 没有按下暂停 */
                }

                if ((g_audiodev.status & (1 << 1)) == 0)        /* 请求结束播放/播放完成 */
                {
                    nblocks = 0;
                    res = AP_PREV;
                    break;
                }
            }

            currentframe++;
        }

        audio_stop();
    }

    f_close(g_audiodev.file);
    myfree(SRAMIN, filterbuf64);
    myfree(SRAMIN, apectrl);
    myfree(SRAMIN, apex->seektable);
    myfree(SRAMIN, apex);
    myfree(SRAMDTCM, decoded0);
    myfree(SRAMDTCM, decoded1);
    myfree(SRAMIN, g_audiodev.file);
    myfree(SRAMIN, g_audiodev.saibuf1);
    myfree(SRAMIN, g_audiodev.saibuf2);
    myfree(SRAMIN, buffer);
    return res;
}
























