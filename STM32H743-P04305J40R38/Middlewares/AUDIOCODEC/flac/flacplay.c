/**
 ****************************************************************************************************
 * @file        flacplay.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       flac���� ����
 *              1, ֧��16/24λ������/������flac�Ľ���
 *              2, ���֧��192K/16bit��96K/24bit��flac����
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

#include "flacplay.h"
#include "audioplay.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "./BSP/KEY/key.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/ES8388/es8388.h"


__flacctrl *flacctrl;   /* flac������ƽṹ�� */


/**
 * @brief       ����FLAC�ļ�
 * @param       fx              : �ļ�ָ��
 * @param       fc              : flac��������
 * @retval      0, �ɹ�
 *              ����, �������
 */
uint8_t flac_init(FIL *fx, __flacctrl *fctrl, FLACContext *fc)
{
    FLAC_Tag *flactag;
    MD_Block_Head *flacblkh;
    uint8_t *buf;
    uint8_t endofmetadata = 0;      /* ���һ��metadata��� */
    int blocklength;
    uint32_t br;
    uint8_t res;

    buf = mymalloc(SRAMIN, 512);    /* ����512�ֽ��ڴ� */

    if (!buf)return 1;              /* �ڴ�����ʧ�� */

    f_lseek(fx, 0);                 /* ƫ�Ƶ��ļ�ͷ */
    f_read(fx, buf, 4, &br);        /* ��ȡ4�ֽ� */
    flactag = (FLAC_Tag *)buf;      /* ǿ��ת��Ϊflac tag��ǩ */

    if (strncmp("fLaC", (char *)flactag->id, 4) != 0)
    {
        myfree(SRAMIN, buf);        /* �ͷ��ڴ� */
        return 2;                   /* ��flac�ļ� */
    }

    while (!endofmetadata)
    {
        f_read(fx, buf, 4, &br);

        if (br < 4)break;

        flacblkh = (MD_Block_Head *)buf;
        endofmetadata = flacblkh->head & 0X80;  /* �ж��ǲ������һ��block? */
        blocklength = ((uint32_t)flacblkh->size[0] << 16) | ((uint16_t)flacblkh->size[1] << 8) | (flacblkh->size[2]); /* �õ����С */

        if ((flacblkh->head & 0x7f) == 0)       /* head���7λΪ0,���ʾ��STREAMINFO�� */
        {
            res = f_read(fx, buf, blocklength, &br);

            if (res != FR_OK)break;

            fc->min_blocksize = ((uint16_t)buf[0] << 8) | buf[1];   /* ��С���С */
            fc->max_blocksize = ((uint16_t)buf[2] << 8) | buf[3];   /* �����С */
            fc->min_framesize = ((uint32_t)buf[4] << 16) | ((uint16_t)buf[5] << 8) | buf[6];                    /* ��С֡��С */
            fc->max_framesize = ((uint32_t)buf[7] << 16) | ((uint16_t)buf[8] << 8) | buf[9];                    /* ���֡��С */
            fc->samplerate = ((uint32_t)buf[10] << 12) | ((uint16_t)buf[11] << 4) | ((buf[12] & 0xf0) >> 4);    /* ������ */
            fc->channels = ((buf[12] & 0x0e) >> 1) + 1;             /* ��Ƶͨ���� */
            fc->bps = ((((uint16_t)buf[12] & 0x01) << 4) | ((buf[13] & 0xf0) >> 4)) + 1;                        /* ����λ��16?24?32? */
            fc->totalsamples = ((uint32_t)buf[14] << 24) | ((uint32_t)buf[15] << 16) | ((uint16_t)buf[16] << 8) | buf[17];  /* һ���������ܲ����� */
            fctrl->samplerate = fc->samplerate;
            fctrl->totsec = (fc->totalsamples / fc->samplerate);    /* �õ���ʱ�� */
        }
        else    /* ��������֡�Ĵ��� */
        {
            if (f_lseek(fx, fx->fptr + blocklength) != FR_OK)
            {
                myfree(SRAMIN, buf);
                return 3;
            }
        }
    }

    myfree(SRAMIN, buf); /* �ͷ��ڴ� */

    if (fctrl->totsec)
    {
        fctrl->outsamples = fc->max_blocksize * 2;  /* PCM���������(*2,��ʾ2��������������) */
        fctrl->bps = fc->bps;                       /* ����λ��(16/24/32) */
        fctrl->datastart = fx->fptr;                /* FLAC����֡��ʼ�ĵ�ַ */
        fctrl->bitrate = ((fx->obj.objsize - fctrl->datastart) * 8) / fctrl->totsec; /* �õ�FLAC��λ�� */
    }
    else return 4;  /* ��ʱ��Ϊ0?�������flac�ļ� */

    return 0;
}

volatile uint8_t flactransferend = 0;   /* i2s������ɱ�־ */
volatile uint8_t flacwitchbuf = 0;      /* i2sbufxָʾ��־ */

/**
 * @brief       FLAC DMA���ͻص�����
 * @param       ��
 * @retval      ��
 */
void flac_i2s_dma_tx_callback(void)
{
    uint16_t i;
    uint16_t size;

    if (DMA1_Stream4->CR & (1 << 19))
    {
        flacwitchbuf = 0;

        if ((g_audiodev.status & 0X01) == 0) /* ��ͣ��,���0 */
        {
            if (flacctrl->bps == 24)size = flacctrl->outsamples * 4;
            else size = flacctrl->outsamples * 2;

            for (i = 0; i < size; i++)g_audiodev.saibuf1[i] = 0;
        }
    }
    else
    {
        flacwitchbuf = 1;

        if ((g_audiodev.status & 0X01) == 0) /* ��ͣ��,���0 */
        {
            if (flacctrl->bps == 24)size = flacctrl->outsamples * 4;
            else size = flacctrl->outsamples * 2;

            for (i = 0; i < size; i++)g_audiodev.saibuf2[i] = 0;
        }
    }

    flactransferend = 1;
}

/**
 * @brief       �õ���ǰ����ʱ��
 * @param       fx              : �ļ�ָ��
 * @param       flacctrl        : flac���ſ�����
 * @retval      ��
 */
void flac_get_curtime(FIL *fx, __flacctrl *flacctrl)
{
    long long fpos = 0;

    if (fx->fptr > flacctrl->datastart)fpos = fx->fptr - flacctrl->datastart;               /* �õ���ǰ�ļ����ŵ��ĵط� */

    flacctrl->cursec = fpos * flacctrl->totsec / (fx->obj.objsize - flacctrl->datastart);   /* ��ǰ���ŵ��ڶ�������? */
}

/**
 * @brief       flac�ļ�������˺���
 * @param       pos             : ��Ҫ��λ�����ļ�λ��
 * @retval      ��ǰ�ļ�λ��(����λ��Ľ��)
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
 * @brief       ����һ��FLAC����
 * @param       pname           : FLAC�ļ�·��
 * @retval      0,�����������
 *              [b7]:0,����״̬;1,����״̬
 *              [b6:0]:b7=0ʱ,��ʾ������
 *                     b7=1ʱ,��ʾ�д���(���ﲻ�ж��������,0X80~0XFF,�����Ǵ���)
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

    if (!fc || !g_audiodev.file || !flacctrl)res = 1;   /* �ڴ�������� */
    else
    {
        memset(fc, 0, sizeof(FLACContext));             /* fc������������ */
        res = f_open(g_audiodev.file, (char *)fname, FA_READ);  /* ��ȡ�ļ����� */

        if (res == FR_OK)
        {
            res = flac_init(g_audiodev.file, flacctrl, fc);     /* flac�����ʼ�� */

            if (fc->min_blocksize == fc->max_blocksize && fc->max_blocksize != 0) /* ����min_blocksize����max_blocksize */
            {
                if (fc->bps == 24)  /* 24λ��Ƶ���� */
                {
                    g_audiodev.saibuf1 = mymalloc(SRAMIN, fc->max_blocksize * 8);
                    g_audiodev.saibuf2 = mymalloc(SRAMIN, fc->max_blocksize * 8);
                }
                else    /* 16λ��Ƶ���� */
                {
                    g_audiodev.saibuf1 = mymalloc(SRAMIN, fc->max_blocksize * 4);
                    g_audiodev.saibuf2 = mymalloc(SRAMIN, fc->max_blocksize * 4);
                }
                
                if(fc->max_framesize == 0)  /* ��֡��С��ȷ����flac, ����Ĭ��֡��С(�ڴ��������ʵ����һ��) */
                {
                    fc->max_framesize = 0X4000;             /* �������֡���� */
                    
                }
                
                buffer = mymalloc(SRAMDTCM, fc->max_framesize);  /* �������֡���� */
                decbuf0 = mymalloc(SRAMDTCM, fc->max_blocksize * 4);
                decbuf1 = mymalloc(SRAMDTCM, fc->max_blocksize * 4);
            }
            else
            {
                res += 1; /* ��֧�ֵ���Ƶ��ʽ */
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

        if (flacctrl->bps == 24)    /* 24λ��Ƶ���� */
        {
            es8388_sai_cfg(2, 2);   /* �����ֱ�׼,24λ���ݳ��ȣ�ͬ��������WM8978����������ES8388 24bit�쳣���д�������� */
            sai1_saia_init(0,1,6);      /* �����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,24λ��չ֡���� */
            sai1_samplerate_set(flacctrl->samplerate);  /* ���ò����� */
            sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, flacctrl->outsamples , 2);  /* ����TX DMA */
            
            memset(g_audiodev.saibuf1, 0, fc->max_blocksize * 8);
            memset(g_audiodev.saibuf2, 0, fc->max_blocksize * 8);
        }
        else    /* 16λ��Ƶ���� */
        {
            es8388_sai_cfg(2, 0);   /* �����ֱ�׼,16λ���ݳ��� */
            sai1_saia_init(0,1,4);      /* �����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,16λ��չ֡���� */
            sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, flacctrl->outsamples,1);      /* ����TX DMA */
            
            memset(g_audiodev.saibuf1, 0, fc->max_blocksize * 4);
            memset(g_audiodev.saibuf2, 0, fc->max_blocksize * 4);
        }

        sai1_samplerate_set(fc->samplerate); /* ���ò����� */
        sai1_tx_callback = flac_i2s_dma_tx_callback;                 /* �ص�����ָ��flac_i2s_dma_tx_callback */
        f_read(g_audiodev.file, buffer, fc->max_framesize, &br);    /* ��ȡ���֡������ */
        
        bytesleft = br;
        audio_start();                      /* ��ʼ���� */
        fc->decoded0 = (int *)decbuf0;      /* ��������0 */
        fc->decoded1 = (int *)decbuf1;      /* ��������1 */
        flac_fptr = g_audiodev.file->fptr;  /* ��¼��ǰ���ļ�λ�� */

        while (bytesleft)
        {
            while (flactransferend == 0) /* �ȴ�������� */
            {
                delay_ms(1000 / OS_TICKS_PER_SEC);
            };

            if (flac_fptr != g_audiodev.file->fptr) /* ˵���ⲿ�н����ļ����/���˲��� */
            {
                if (g_audiodev.file->fptr < flacctrl->datastart) /* �����ݿ�ʼ֮ǰ?? */
                {
                    f_lseek(g_audiodev.file, flacctrl->datastart); /* ƫ�Ƶ����ݿ�ʼ�ĵط� */
                }

                f_read(g_audiodev.file, buffer, fc->max_framesize, &br); /* ��ȡһ�����֡�������� */
                bytesleft = flac_seek_frame(buffer, br, fc);    /* ����֡ */

                if (bytesleft >= 0) /* �ҵ���ȷ��֡ͷ */
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

            if (res != 0) /* ��������� */
            {
                res = AP_ERR;
                break;
            }

            consumed = fc->gb.index / 8;
            memmove(buffer, &buffer[consumed], bytesleft - consumed);
            bytesleft -= consumed;
            res = f_read(g_audiodev.file, &buffer[bytesleft], fc->max_framesize - bytesleft, &br);

            if (res) /* �����ݳ����� */
            {
                res = AP_ERR;
                break;
            }

            if (br > 0)
            {
                bytesleft += br;
            }

            flac_fptr = g_audiodev.file->fptr;      /* ��¼��ǰ���ļ�λ�� */

            while (g_audiodev.status & (1 << 1))    /* ���������� */
            {
                flac_get_curtime(g_audiodev.file, flacctrl);    /* �õ���ʱ��͵�ǰ���ŵ�ʱ�� */
                g_audiodev.totsec = flacctrl->totsec;           /* �������� */
                g_audiodev.cursec = flacctrl->cursec;
                g_audiodev.bitrate = flacctrl->bitrate;
                g_audiodev.samplerate = flacctrl->samplerate;
                g_audiodev.bps = flacctrl->bps;

                if (g_audiodev.status & 0X01)break;     /* û�а�����ͣ */
                else delay_ms(1000 / OS_TICKS_PER_SEC);
            }

            if ((g_audiodev.status & (1 << 1)) == 0)    /* �����������/������� */
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









































