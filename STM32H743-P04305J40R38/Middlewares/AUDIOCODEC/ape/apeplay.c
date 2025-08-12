/**
 ****************************************************************************************************
 * @file        apeplay.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       ape���� ����
 *              1, ֧��16λ������/������ape�Ľ���
 *              2, ���֧��96K��APE��ʽ(LV1��LV2)
 *              3, LV1~LV3,��48K��������������,LV4,LV5��
 *              4, ��ĳЩape�ļ�,���ܲ�֧��,����Monkey's Audio�������ת��һ��,������������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221127
 * ��һ�η���
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


__apectrl *apectrl;	/* APE���ſ��ƽṹ�� */

/* apedecoder.c������Ҫ������ */
extern filter_int *filterbuf64;         /* ��Ҫ2816�ֽ� */

volatile uint8_t apetransferend = 0;    /* i2s������ɱ�־ */
volatile uint8_t apewitchbuf = 0;       /* i2sbufxָʾ��־ */


/**
 * @brief       APE DMA���ͻص�����
 * @param       ��
 * @retval      ��
 */
void ape_i2s_dma_tx_callback(void)
{
    uint16_t i;

    if (DMA1_Stream4->CR & (1 << 19))
    {
        apewitchbuf = 0;

        if ((g_audiodev.status & 0X01) == 0) /* ��ͣ��,���0 */
        {
            for (i = 0; i < APE_BLOCKS_PER_LOOP * 4; i++)g_audiodev.saibuf1[i] = 0;
        }
    }
    else
    {
        apewitchbuf = 1;

        if ((g_audiodev.status & 0X01) == 0) /* ��ͣ��,���0 */
        {
            for (i = 0; i < APE_BLOCKS_PER_LOOP * 4; i++)g_audiodev.saibuf2[i] = 0;
        }
    }

    apetransferend = 1;
}

/**
 * @brief       ���PCM���ݵ�DAC
 * @param       buf             : PCM�����׵�ַ
 * @param       size            : pcm������(32λΪ��λ)
 * @retval      ��
 */
void ape_fill_buffer(uint32_t *buf, uint16_t size)
{
    uint16_t i;
    uint32_t *p;

    while (apetransferend == 0)delay_ms(1000 / OS_TICKS_PER_SEC); /* �ȴ�������� */

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
 * @brief       �õ���ǰ����ʱ��
 * @param       fx              : �ļ�ָ��
 * @param       apectrl         : apectrl���ſ�����
 * @retval      ��
 */
void ape_get_curtime(FIL *fx, __apectrl *apectrl)
{
    long long fpos = 0;

    if (fx->fptr > apectrl->datastart)fpos = fx->fptr - apectrl->datastart;             /* �õ���ǰ�ļ����ŵ��ĵط� */

    apectrl->cursec = fpos * apectrl->totsec / (fx->obj.objsize - apectrl->datastart);  /* ��ǰ���ŵ��ڶ�������? */
}

/**
 * @brief       ape �ļ�������˺���
 * @param       pos             : ��Ҫ��λ�����ļ�λ��
 * @retval      ��ǰ�ļ�λ��(����λ��Ľ��)
 */
uint32_t ape_file_seek(uint32_t pos)
{
    return g_audiodev.file->fptr;/* ape�ļ���֧�ֿ������,ֱ�ӷ��ص�ǰ����λ�� */
}

/**
 * @brief       ����һ��APE����
 * @param       pname           : APE �ļ�·��
 * @retval      0,�����������
 *              [b7]:0,����״̬;1,����״̬
 *              [b6:0]:b7=0ʱ,��ʾ������
 *                     b7=1ʱ,��ʾ�д���(���ﲻ�ж��������,0X80~0XFF,�����Ǵ���)
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

    if (filterbuf64 && apectrl && apex && decoded0 && decoded1 && g_audiodev.file && g_audiodev.saibuf1 && g_audiodev.saibuf2 && buffer) /* �����ڴ����붼OK */
    {
        memset(apex, 0, sizeof(struct ape_ctx_t));
        memset(apectrl, 0, sizeof(__apectrl));
        memset(g_audiodev.saibuf1, 0, APE_BLOCKS_PER_LOOP * 4);
        memset(g_audiodev.saibuf2, 0, APE_BLOCKS_PER_LOOP * 4);
        f_open(g_audiodev.file, (char *)fname, FA_READ);    /* ���ļ� */
        res = ape_parseheader(g_audiodev.file, apex);       /* ����ape�ļ�ͷ */

        if (res == 0)
        {
            if ((apex->compressiontype > 3000) || (apex->fileversion < APE_MIN_VERSION) || (apex->fileversion > APE_MAX_VERSION || apex->bps != 16))
            {
                res = AP_ERR; /* ѹ���ʲ�֧��/�汾��֧��/����16λ��Ƶ��ʽ */
            }
            else
            {
                apectrl->bps = apex->bps;   /* �õ��������(ape,���ǽ�֧��16λ) */
                apectrl->samplerate = apex->samplerate;     /* �õ�������,ape֧��48Khz���µĲ�����,�ڸ߾Ϳ��ܿ���... */

                if (apex->totalframes > 1)totalsamples = apex->finalframeblocks + apex->blocksperframe * (apex->totalframes - 1);
                else totalsamples = apex->finalframeblocks;

                apectrl->totsec = totalsamples / apectrl->samplerate;   /* �õ��ļ���ʱ�� */
                apectrl->bitrate = (g_audiodev.file->obj.objsize - apex->firstframe) * 8 / apectrl->totsec; /* �õ�λ�� */
                apectrl->outsamples = APE_BLOCKS_PER_LOOP * 2;  /* PCM���������(16λΪ��λ) */
                apectrl->datastart = apex->firstframe;      /* �õ���һ֡�ĵ�ַ */
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

        es8388_sai_cfg(2, 0);   /* �����ֱ�׼,16λ���ݳ��� */
        sai1_saia_init(0,1,4);      /* �����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,16λ��չ֡���� */
        sai1_samplerate_set(apex->samplerate);       /* ���ò����� */
        sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, APE_BLOCKS_PER_LOOP * 2,1);    /* ����TX DMA */
        sai1_tx_callback = ape_i2s_dma_tx_callback;  /* �ص�����ָape_i2s_dma_tx_callback */

        currentframe = 0;
        f_lseek(g_audiodev.file, apex->firstframe);
        res = f_read(g_audiodev.file, buffer, APE_FILE_BUF_SZ, (uint32_t *)&bytesinbuffer);
        firstbyte = 3;          /* Take account of the little-endian 32-bit byte ordering */
        readptr = buffer;
        audio_start();

        while (currentframe < apex->totalframes && res == 0) /* ����δ�����֡? */
        {
            /* ����һ֡�����ж��ٸ�blocks? */
            if (currentframe == (apex->totalframes - 1))nblocks = apex->finalframeblocks;
            else nblocks = apex->blocksperframe;

            apex->currentframeblocks = nblocks;
            /* ��ʼ��֡���� */
            init_frame_decoder(apex, readptr, &firstbyte, &bytesconsumed);
            readptr += bytesconsumed;
            bytesinbuffer -= bytesconsumed;

            while (nblocks > 0) /* ��ʼ֡���� */
            {
                blockstodecode = AUDIO_MIN(APE_BLOCKS_PER_LOOP, nblocks); /* ���һ��Ҫ�����blocks���� */
                res = decode_chunk(apex, readptr, &firstbyte, &bytesconsumed, decoded0, decoded1, blockstodecode);

                if (res != 0)
                {
                    printf("frame decode err\r\n");
                    res = AP_ERR;
                    break;
                }

                ape_fill_buffer((uint32_t *)decoded1, APE_BLOCKS_PER_LOOP);
                readptr += bytesconsumed;           /* ����ָ��ƫ�Ƶ�������λ�� */
                bytesinbuffer -= bytesconsumed;     /* buffer��������������� */

                if (bytesconsumed > 4 * APE_BLOCKS_PER_LOOP) /* ���ִ����� */
                {
                    nblocks = 0;
                    res = AP_ERR;
                    printf("bytesconsumed:%d\r\n", bytesconsumed);
                }

                if (bytesinbuffer < 4 * APE_BLOCKS_PER_LOOP) /* ��Ҫ������������ */
                {
                    memmove(buffer, readptr, bytesinbuffer);
                    res = f_read(g_audiodev.file, buffer + bytesinbuffer, APE_FILE_BUF_SZ - bytesinbuffer, (uint32_t *)&n);

                    if (res) /* ������ */
                    {
                        res = AP_ERR;
                        break;
                    }

                    bytesinbuffer += n;
                    readptr = buffer;
                }

                nblocks -= blockstodecode; /* block�����ݼ� */

                while (g_audiodev.status & (1 << 1)) /* ���������� */
                {
                    ape_get_curtime(g_audiodev.file, apectrl);  /* �õ���ʱ��͵�ǰ���ŵ�ʱ�� */
                    g_audiodev.totsec = apectrl->totsec;        /* �������� */
                    g_audiodev.cursec = apectrl->cursec;
                    g_audiodev.bitrate = apectrl->bitrate;
                    g_audiodev.samplerate = apectrl->samplerate;
                    g_audiodev.bps = apectrl->bps;
                    delay_ms(1000 / OS_TICKS_PER_SEC);

                    if (g_audiodev.status & 0X01)break;         /* û�а�����ͣ */
                }

                if ((g_audiodev.status & (1 << 1)) == 0)        /* �����������/������� */
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
























