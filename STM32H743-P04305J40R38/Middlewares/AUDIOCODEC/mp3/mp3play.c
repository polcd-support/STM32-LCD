/**
 ****************************************************************************************************
 * @file        mp3play.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       mp3���� ����
 *              1, ֧��16λ������/������MP3�Ľ���
 *              2, ֧��CBR/VBR��ʽMP3����
 *              3, ֧��ID3V1��ID3V2��ǩ����
 *              4, ֧�����б�����(MP3�����320Kbps)����
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


__mp3ctrl *mp3ctrl;                     /* mp3���ƽṹ�� */
volatile uint8_t mp3transferend = 0;    /* i2s������ɱ�־ */
volatile uint8_t mp3witchbuf = 0;       /* i2sbufxָʾ��־ */

/**
 * @brief       MP3 DMA���ͻص�����
 * @param       ��
 * @retval      ��
 */
void mp3_sai_dma_tx_callback(void)
{
    uint16_t i;

    if (DMA2_Stream3->CR & (1 << 19))
    {
        mp3witchbuf = 0;

        if ((g_audiodev.status & 0X01) == 0) /* ��ͣ��,���0 */
        {
            for (i = 0; i < 2304 * 2; i++)g_audiodev.saibuf1[i] = 0;
        }
    }
    else
    {
        mp3witchbuf = 1;

        if ((g_audiodev.status & 0X01) == 0) /* ��ͣ��,���0 */
        {
            for (i = 0; i < 2304 * 2; i++)g_audiodev.saibuf2[i] = 0;
        }
    }

    mp3transferend = 1;
}

/**
 * @brief       ���PCM���ݵ�DAC
 * @param       buf             : PCM�����׵�ַ
 * @param       size            : pcm������(16λΪ��λ)
 * @param       nch             : ������(1,������,2������)
 * @retval      ��
 */
void mp3_fill_buffer(uint16_t *buf, uint16_t size, uint8_t nch)
{
    uint16_t i;
    uint16_t *p;

    while (mp3transferend == 0) /* �ȴ�������� */
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
    else    /* ������ */
    {
        for (i = 0; i < size; i++)
        {
            p[2 * i] = buf[i];
            p[2 * i + 1] = buf[i];
        }
    }
}

/**
 * @brief       ����ID3V1
 * @param       buf             : �������ݻ�����(��С�̶���128�ֽ�)
 * @param       pctrl           : MP3������
 * @retval      0, ����
 *              ����, ʧ��
 */
uint8_t mp3_id3v1_decode(uint8_t *buf, __mp3ctrl *pctrl)
{
    ID3V1_Tag *id3v1tag;
    id3v1tag = (ID3V1_Tag *)buf;

    if (strncmp("TAG", (char *)id3v1tag->id, 3) == 0) /* ��MP3 ID3V1 TAG */
    {
        if (id3v1tag->title[0])strncpy((char *)pctrl->title, (char *)id3v1tag->title, 30);

        if (id3v1tag->artist[0])strncpy((char *)pctrl->artist, (char *)id3v1tag->artist, 30);
    }
    else return 1;

    return 0;
}

/**
 * @brief       ����ID3V2
 * @param       buf             : �������ݻ�����(��С�̶���128�ֽ�)
 * @param       size            : ���ݴ�С
 * @param       pctrl           : MP3������
 * @retval      0, ����
 *              ����, ʧ��
 */
uint8_t mp3_id3v2_decode(uint8_t *buf, uint32_t size, __mp3ctrl *pctrl)
{
    ID3V2_TagHead *taghead;
    ID3V23_FrameHead *framehead;
    uint32_t t;
    uint32_t tagsize;       /* tag��С */
    uint32_t frame_size;    /* ֡��С */
    taghead = (ID3V2_TagHead *)buf;

    if (strncmp("ID3", (const char *)taghead->id, 3) == 0) /* ����ID3? */
    {
        tagsize = ((uint32_t)taghead->size[0] << 21) | ((uint32_t)taghead->size[1] << 14) | ((uint16_t)taghead->size[2] << 7) | taghead->size[3]; /* �õ�tag ��С */
        pctrl->datastart = tagsize;         /* �õ�mp3���ݿ�ʼ��ƫ���� */

        if (tagsize > size)tagsize = size;  /* tagsize��������bufsize��ʱ��,ֻ��������size��С������ */

        if (taghead->mversion < 3)
        {
            printf("not supported mversion!\r\n");
            return 1;
        }

        t = 10;

        while (t < tagsize)
        {
            framehead = (ID3V23_FrameHead *)(buf + t);
            frame_size = ((uint32_t)framehead->size[0] << 24) | ((uint32_t)framehead->size[1] << 16) | ((uint32_t)framehead->size[2] << 8) | framehead->size[3]; /* �õ�֡��С */

            if (strncmp("TT2", (char *)framehead->id, 3) == 0 || strncmp("TIT2", (char *)framehead->id, 4) == 0) /* �ҵ���������֡,��֧��unicode��ʽ!! */
            {
                strncpy((char *)pctrl->title, (char *)(buf + t + sizeof(ID3V23_FrameHead) + 1), AUDIO_MIN(frame_size - 1, MP3_TITSIZE_MAX - 1));
            }

            if (strncmp("TP1", (char *)framehead->id, 3) == 0 || strncmp("TPE1", (char *)framehead->id, 4) == 0) /* �ҵ�����������֡ */
            {
                strncpy((char *)pctrl->artist, (char *)(buf + t + sizeof(ID3V23_FrameHead) + 1), AUDIO_MIN(frame_size - 1, MP3_ARTSIZE_MAX - 1));
            }

            t += frame_size + sizeof(ID3V23_FrameHead);
        }
    }
    else pctrl->datastart = 0; /* ������ID3,mp3�����Ǵ�0��ʼ */

    return 0;
}

/**
 * @brief       ��ȡMP3������Ϣ
 * @param       pname           : MP3�ļ�·��
 * @param       pctrl           : MP3������
 * @retval      0, �ɹ�
 *              ����, ʧ��
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
    short samples_per_frame;    /* һ֡�Ĳ������� */
    uint32_t totframes;         /* ��֡�� */

    fmp3 = mymalloc(SRAMIN, sizeof(FIL));
    buf = mymalloc(SRAMIN, 5 * 1024);   /* ����5K�ڴ� */

    if (fmp3 && buf) /* �ڴ�����ɹ� */
    {
        f_open(fmp3, (const TCHAR *)pname, FA_READ); /* ���ļ� */
        res = f_read(fmp3, (char *)buf, 5 * 1024, &br);

        if (res == 0) /* ��ȡ�ļ��ɹ�,��ʼ����ID3V2/ID3V1�Լ���ȡMP3��Ϣ */
        {
            mp3_id3v2_decode(buf, br, pctrl);           /* ����ID3V2���� */
            f_lseek(fmp3, fmp3->obj.objsize - 128);     /* ƫ�Ƶ�����128��λ�� */
            f_read(fmp3, (char *)buf, 128, &br);        /* ��ȡ128�ֽ� */
            mp3_id3v1_decode(buf, pctrl);               /* ����ID3V1���� */
            decoder = MP3InitDecoder();                 /* MP3���������ڴ� */
            f_lseek(fmp3, pctrl->datastart);            /* ƫ�Ƶ����ݿ�ʼ�ĵط� */
            f_read(fmp3, (char *)buf, 5 * 1024, &br);   /* ��ȡ5K�ֽ�mp3���� */
            offset = MP3FindSyncWord(buf, br);          /* ����֡ͬ����Ϣ */

            if (offset >= 0 && MP3GetNextFrameInfo(decoder, &frame_info, &buf[offset]) == 0) /* �ҵ�֡ͬ����Ϣ��,����һ����Ϣ��ȡ���� */
            {
                p = offset + 4 + 32;
                fvbri = (MP3_FrameVBRI *)(buf + p);

                if (strncmp("VBRI", (char *)fvbri->id, 4) == 0) /* ����VBRI֡(VBR��ʽ) */
                {
                    if (frame_info.version == MPEG1)samples_per_frame = 1152; /* MPEG1,layer3ÿ֡����������1152 */
                    else samples_per_frame = 576; /* MPEG2/MPEG2.5,layer3ÿ֡����������576 */

                    totframes = ((uint32_t)fvbri->frames[0] << 24) | ((uint32_t)fvbri->frames[1] << 16) | ((uint16_t)fvbri->frames[2] << 8) | fvbri->frames[3]; /* �õ���֡�� */
                    pctrl->totsec = totframes * samples_per_frame / frame_info.samprate; /* �õ��ļ��ܳ��� */
                }
                else    /* ����VBRI֡,�����ǲ���Xing֡(VBR��ʽ) */
                {
                    if (frame_info.version == MPEG1)    /* MPEG1 */
                    {
                        p = frame_info.nChans == 2 ? 32 : 17;
                        samples_per_frame = 1152;       /* MPEG1,layer3ÿ֡����������1152 */
                    }
                    else
                    {
                        p = frame_info.nChans == 2 ? 17 : 9;
                        samples_per_frame = 576;        /* MPEG2/MPEG2.5,layer3ÿ֡����������576 */
                    }

                    p += offset + 4;
                    fxing = (MP3_FrameXing *)(buf + p);

                    if (strncmp("Xing", (char *)fxing->id, 4) == 0 || strncmp("Info", (char *)fxing->id, 4) == 0) /* ��Xng֡ */
                    {
                        if (fxing->flags[3] & 0X01)     /* ������frame�ֶ� */
                        {
                            totframes = ((uint32_t)fxing->frames[0] << 24) | ((uint32_t)fxing->frames[1] << 16) | ((uint16_t)fxing->frames[2] << 8) | fxing->frames[3]; /* �õ���֡�� */
                            pctrl->totsec = totframes * samples_per_frame / frame_info.samprate; /* �õ��ļ��ܳ��� */
                        }
                        else    /* ��������frames�ֶ� */
                        {
                            pctrl->totsec = fmp3->obj.objsize / (frame_info.bitrate / 8);
                        }
                    }
                    else        /* CBR��ʽ,ֱ�Ӽ����ܲ���ʱ�� */
                    {
                        pctrl->totsec = fmp3->obj.objsize / (frame_info.bitrate / 8);
                    }
                }

                pctrl->bitrate = frame_info.bitrate;        /* �õ���ǰ֡������ */
                mp3ctrl->samplerate = frame_info.samprate;  /* �õ�������. */

                if (frame_info.nChans == 2)mp3ctrl->outsamples = frame_info.outputSamps; /* ���PCM��������С */
                else mp3ctrl->outsamples = frame_info.outputSamps * 2; /* ���PCM��������С,���ڵ�����MP3,ֱ��*2,����Ϊ˫������� */
            }
            else res = 0XFE; /* δ�ҵ�ͬ��֡ */

            MP3FreeDecoder(decoder);/* �ͷ��ڴ� */
        }

        f_close(fmp3);
    }
    else res = 0XFF;

    myfree(SRAMIN, fmp3);
    myfree(SRAMIN, buf);
    return res;
}

/**
 * @brief       �õ���ǰ����ʱ��
 * @param       fx              : �ļ�ָ��
 * @param       mp3x            : mp3���ſ�����
 * @retval      ��
 */
void mp3_get_curtime(FIL *fx, __mp3ctrl *mp3x)
{
    uint32_t fpos = 0;

    if (fx->fptr > mp3x->datastart)fpos = fx->fptr - mp3x->datastart;           /* �õ���ǰ�ļ����ŵ��ĵط� */

    mp3x->cursec = fpos * mp3x->totsec / (fx->obj.objsize - mp3x->datastart);   /* ��ǰ���ŵ��ڶ�������? */
}

/**
 * @brief       mp3�ļ�������˺���
 * @param       pos             : ��Ҫ��λ�����ļ�λ��
 * @retval      ��ǰ�ļ�λ��(����λ��Ľ��)
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
 * @brief       ����һ��MP3����
 * @param       pname           : MP3�ļ�·��
 * @retval      0,�����������
 *              [b7]:0,����״̬;1,����״̬
 *              [b6:0]:b7=0ʱ,��ʾ������
 *                     b7=1ʱ,��ʾ�д���(���ﲻ�ж��������,0X80~0XFF,�����Ǵ���)
 */
uint8_t mp3_play_song(uint8_t *fname)
{
    HMP3Decoder mp3decoder;
    MP3FrameInfo mp3frameinfo;
    uint8_t res;
    uint8_t *buffer;    /* ����buffer */
    uint8_t *readptr;   /* MP3�����ָ�� */
    int offset = 0;     /* ƫ���� */
    int outofdata = 0;  /* �������ݷ�Χ */
    int bytesleft = 0;  /* buffer��ʣ�����Ч���� */
    uint32_t br = 0;
    int err = 0;

    mp3ctrl = mymalloc(SRAMIN, sizeof(__mp3ctrl));
    buffer = mymalloc(SRAMIN, MP3_FILE_BUF_SZ); /* �������buf��С */
    g_audiodev.file = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    g_audiodev.saibuf1 = mymalloc(SRAMIN, 2304 * 2);
    g_audiodev.saibuf2 = mymalloc(SRAMIN, 2304 * 2);
    g_audiodev.tbuf = mymalloc(SRAMIN, 2304 * 2);
    g_audiodev.file_seek = mp3_file_seek;

    if (!mp3ctrl || !buffer || !g_audiodev.file || !g_audiodev.saibuf1 || !g_audiodev.saibuf2 || !g_audiodev.tbuf) /* �ڴ�����ʧ�� */
    {
        myfree(SRAMIN, mp3ctrl);
        myfree(SRAMIN, buffer);
        myfree(SRAMIN, g_audiodev.file);
        myfree(SRAMIN, g_audiodev.saibuf1);
        myfree(SRAMIN, g_audiodev.saibuf2);
        myfree(SRAMIN, g_audiodev.tbuf);
        return AP_ERR;  /* ���� */
    }

    memset(g_audiodev.saibuf1, 0, 2304 * 2);    /* �������� */
    memset(g_audiodev.saibuf2, 0, 2304 * 2);    /* �������� */
    memset(mp3ctrl, 0, sizeof(__mp3ctrl));      /* �������� */
    res = mp3_get_info(fname, mp3ctrl);

    if (res == 0)
    {
        printf("     title:%s\r\n", mp3ctrl->title);
        printf("    artist:%s\r\n", mp3ctrl->artist);
        printf("   bitrate:%dbps\r\n", mp3ctrl->bitrate);
        printf("samplerate:%d\r\n", mp3ctrl->samplerate);
        printf("  totalsec:%d\r\n", mp3ctrl->totsec);

        es8388_sai_cfg(0, 3);   /* �����ֱ�׼,16λ���ݳ��� */
        sai1_saia_init(0,1,4);      /* �����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,16λ��չ֡���� */
        sai1_samplerate_set(mp3ctrl->samplerate);    /* ���ò����� */
        sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, mp3ctrl->outsamples,1);    /* ����TX DMA */
        sai1_tx_callback = mp3_sai_dma_tx_callback;  /* �ص�����ָmp3_sai_dma_tx_callback */

        mp3decoder = MP3InitDecoder();              /* MP3���������ڴ� */
        res = f_open(g_audiodev.file, (char *)fname, FA_READ);  /* ���ļ� */
    }

    if (res == 0 && mp3decoder != 0) /* ���ļ��ɹ� */
    {
        f_lseek(g_audiodev.file, mp3ctrl->datastart);   /* �����ļ�ͷ��tag��Ϣ */
        audio_start();          /* ��ʼ���� */

        while (res == 0)
        {
            readptr = buffer;   /* MP3��ָ��ָ��buffer */
            offset = 0;         /* ƫ����Ϊ0 */
            outofdata = 0;      /* �������� */
            bytesleft = 0;
            res = f_read(g_audiodev.file, buffer, MP3_FILE_BUF_SZ, &br); /* һ�ζ�ȡMP3_FILE_BUF_SZ�ֽ� */

            if (res) /* �����ݳ����� */
            {
                res = AP_ERR;
                break;
            }

            if (br == 0)        /* ����Ϊ0,˵����������� */
            {
                res = AP_OK;    /* ������� */
                break;
            }

            bytesleft += br;    /* buffer�����ж�����ЧMP3����? */
            err = 0;

            while (!outofdata)  /* û�г��������쳣(���ɷ��ҵ�֡ͬ���ַ�) */
            {
                offset = MP3FindSyncWord(readptr, bytesleft); /* ��readptrλ��,��ʼ����ͬ���ַ� */

                if (offset < 0) /* û���ҵ�ͬ���ַ�,����֡����ѭ�� */
                {
                    outofdata = 1; /* û�ҵ�֡ͬ���ַ� */
                }
                else    /* �ҵ�ͬ���ַ��� */
                {
                    readptr += offset;      /* MP3��ָ��ƫ�Ƶ�ͬ���ַ��� */
                    bytesleft -= offset;    /* buffer�������Ч���ݸ���,�����ȥƫ���� */
                    err = MP3Decode(mp3decoder, &readptr, &bytesleft, (short *)g_audiodev.tbuf, 0); /* ����һ֡MP3���� */

                    if (err != 0)
                    {
                        printf("decode error:%d\r\n", err);
                        break;
                    }
                    else
                    {
                        MP3GetLastFrameInfo(mp3decoder, &mp3frameinfo); /* �õ��ոս����MP3֡��Ϣ */

                        if (mp3ctrl->bitrate != mp3frameinfo.bitrate)   /* �������� */
                        {
                            mp3ctrl->bitrate = mp3frameinfo.bitrate;
                        }

                        mp3_fill_buffer((uint16_t *)g_audiodev.tbuf, mp3frameinfo.outputSamps, mp3frameinfo.nChans); /* ���pcm���� */
                    }

                    if (bytesleft < MAINBUF_SIZE * 2) /* ����������С��2��MAINBUF_SIZE��ʱ��,���벹���µ����ݽ��� */
                    {
                        memmove(buffer, readptr, bytesleft); /* �ƶ�readptr��ָ������ݵ�buffer����,��������СΪ:bytesleft */
                        f_read(g_audiodev.file, buffer + bytesleft, MP3_FILE_BUF_SZ - bytesleft, &br); /* �������µ����� */

                        if (br < MP3_FILE_BUF_SZ - bytesleft)
                        {
                            memset(buffer + bytesleft + br, 0, MP3_FILE_BUF_SZ - bytesleft - br);
                        }

                        bytesleft = MP3_FILE_BUF_SZ;
                        readptr = buffer;
                    }

                    while (g_audiodev.status & (1 << 1)) /* ���������� */
                    {
                        delay_ms(1000 / OS_TICKS_PER_SEC);
                        mp3_get_curtime(g_audiodev.file, mp3ctrl);
                        g_audiodev.totsec = mp3ctrl->totsec;    /* �������� */
                        g_audiodev.cursec = mp3ctrl->cursec;
                        g_audiodev.bitrate = mp3ctrl->bitrate;
                        g_audiodev.samplerate = mp3ctrl->samplerate;
                        g_audiodev.bps = 16;                /* MP3��֧��16λ */

                        if (g_audiodev.status & 0X01)break; /* û�а�����ͣ */
                    }

                    if ((g_audiodev.status & (1 << 1)) == 0)/* �����������/������� */
                    {
                        res = AP_NEXT;  /* �������ϼ�ѭ�� */
                        outofdata = 1;  /* ������һ��ѭ�� */
                        break;
                    }
                }
            }
        }

        audio_stop();   /* �ر���Ƶ��� */
    }
    else res = AP_ERR;  /* ���� */

    f_close(g_audiodev.file);
    MP3FreeDecoder(mp3decoder); /* �ͷ��ڴ� */
    myfree(SRAMIN, mp3ctrl);
    myfree(SRAMIN, buffer);
    myfree(SRAMIN, g_audiodev.file);
    myfree(SRAMIN, g_audiodev.saibuf1);
    myfree(SRAMIN, g_audiodev.saibuf2);
    myfree(SRAMIN, g_audiodev.tbuf);
    return res;
}








