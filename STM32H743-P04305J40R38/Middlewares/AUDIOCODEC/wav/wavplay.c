/**
 ****************************************************************************************************
 * @file        wavplay.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-25
 * @brief       wav���� ����
 *              1, ֧��16λ/24λWAV�ļ�����
 *              2, ��߿���֧�ֵ�192K/24bit��WAV��ʽ
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
 * V1.0 20221125
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./AUDIOCODEC/wav/wavplay.h"
#include "./APP/audioplay.h"
#include "./SYSTEM/USART/usart.h" 
#include "./SYSTEM/delay/delay.h" 
#include "./MALLOC/malloc.h"
#include "./FATFS/source/ff.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/KEY/key.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/LED/led.h"


__wavctrl wavctrl;                      /* WAV���ƽṹ�� */
volatile uint8_t wavtransferend = 0;    /* sai������ɱ�־ */
volatile uint8_t wavwitchbuf = 0;       /* saibufxָʾ��־ */

/**
 * @brief       WAV������ʼ��
 * @param       fname : �ļ�·��+�ļ���
 * @param       wavx  : ��Ϣ��Žṹ��ָ��
 * @retval      0,���ļ��ɹ�
 *              1,���ļ�ʧ��
 *              2,��WAV�ļ�
 *              3,DATA����δ�ҵ�
 */
uint8_t wav_decode_init(uint8_t *fname, __wavctrl *wavx)
{
    FIL *ftemp;
    uint8_t *buf; 
    uint32_t br = 0;
    uint8_t res = 0;

    ChunkRIFF *riff;
    ChunkFMT *fmt;
    ChunkFACT *fact;
    ChunkDATA *data;
    
    ftemp = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    buf = mymalloc(SRAMIN, 512);

    if (ftemp && buf)    /* �ڴ�����ɹ� */
    {
        res = f_open(ftemp, (TCHAR*)fname, FA_READ);    /* ���ļ� */
        if (res==FR_OK)
        {
            f_read(ftemp, buf, 512, &br);               /* ��ȡ512�ֽ������� */
            riff = (ChunkRIFF *)buf;                    /* ��ȡRIFF�� */
            if (riff->Format == 0X45564157)             /* ��WAV�ļ� */
            {
                fmt = (ChunkFMT *)(buf + 12);           /* ��ȡFMT�� */
                fact = (ChunkFACT *)(buf + 12 + 8 + fmt->ChunkSize);                    /* ��ȡFACT�� */

                if (fact->ChunkID == 0X74636166 || fact->ChunkID == 0X5453494C)
                {
                    wavx->datastart = 12 + 8 + fmt->ChunkSize + 8 + fact->ChunkSize;    /* ����fact/LIST���ʱ��(δ����) */
                }
                else
                {
                    wavx->datastart = 12 + 8 + fmt->ChunkSize;
                }

                data = (ChunkDATA *)(buf + wavx->datastart);    /* ��ȡDATA�� */

                if (data->ChunkID == 0X61746164)                /* �����ɹ�! */
                {
                    wavx->audioformat = fmt->AudioFormat;       /* ��Ƶ��ʽ */
                    wavx->nchannels = fmt->NumOfChannels;       /* ͨ���� */
                    wavx->samplerate = fmt->SampleRate;         /* ������ */
                    wavx->bitrate = fmt->ByteRate * 8;          /* �õ�λ�� */
                    wavx->blockalign = fmt->BlockAlign;         /* ����� */
                    wavx->bps = fmt->BitsPerSample;             /* λ��,16/24/32λ */
                    
                    wavx->datasize = data->ChunkSize;           /* ���ݿ��С */
                    wavx->datastart = wavx->datastart + 8;      /* ��������ʼ�ĵط�. */
                     
                    printf("wavx->audioformat:%d\r\n", wavx->audioformat);
                    printf("wavx->nchannels:%d\r\n", wavx->nchannels);
                    printf("wavx->samplerate:%d\r\n", wavx->samplerate);
                    printf("wavx->bitrate:%d\r\n", wavx->bitrate);
                    printf("wavx->blockalign:%d\r\n", wavx->blockalign);
                    printf("wavx->bps:%d\r\n", wavx->bps);
                    printf("wavx->datasize:%d\r\n", wavx->datasize);
                    printf("wavx->datastart:%d\r\n", wavx->datastart);  
                }
                else
                {
                    res = 3;    /* data����δ�ҵ�. */
                }
            }
            else
            {
                res = 2;        /* ��wav�ļ� */
            }
        }
        else
        {
            res = 1;            /* ���ļ����� */
        }
    }

    f_close(ftemp);             /* �ر��ļ� */
    myfree(SRAMIN,ftemp);       /* �ͷ��ڴ� */
    myfree(SRAMIN,buf); 

    return 0;
}

/**
 * @brief       ���buf
 * @param       buf  : �����
 * @param       size : ���������
 * @param       bits : λ��(16/24)
 * @retval      ��ȡ�������ݳ���
 */
uint32_t wav_buffill(uint8_t *buf, uint16_t size, uint8_t bits)
{
    uint16_t readlen = 0;
    uint32_t bread;
    uint16_t i;
    uint32_t *p, *pbuf;

    if (bits == 24)                 /* 24bit��Ƶ,��Ҫ����һ�� */
    {
        readlen = (size / 4) * 3;   /* �˴�Ҫ��ȡ���ֽ��� */
        f_read(g_audiodev.file, g_audiodev.tbuf, readlen, (UINT *)&bread); /* ��ȡ���� */
        pbuf = (uint32_t *)buf;

        for (i = 0; i < size / 4; i++)
        {
            p = (uint32_t *)(g_audiodev.tbuf + i * 3);
            pbuf[i] = p[0];
        }

        bread = (bread * 4) / 3;    /* ����Ĵ�С */
    }
    else 
    {
        f_read(g_audiodev.file, buf, size, (UINT*)&bread);                /* 16bit��Ƶ,ֱ�Ӷ�ȡ���� */
        if (bread < size)           /* ����������,����0 */
        {
            for (i = bread; i < size - bread; i++)
            {
                buf[i] = 0;
            }
        }
    }
    
    return bread;
}

/**
 * @brief       WAV����ʱ,SAI DMA����ص�����
 * @param       ��
 * @retval      ��
 */
void wav_sai_dma_tx_callback(void) 
{
    uint16_t i;

    if (SAI1_TX_DMASx->CR & (1 << 19))
    {
        wavwitchbuf = 0;
        if ((g_audiodev.status & 0X01) == 0)
        {
            for (i = 0; i < WAV_SAI_TX_DMA_BUFSIZE; i++)    /* ��ͣ */
            {
                g_audiodev.saibuf1[i] = 0;                  /* ���0 */
            }
        }
    }
    else 
    {
        wavwitchbuf = 1;
        if ((g_audiodev.status & 0X01) == 0)
        {
            for (i = 0; i < WAV_SAI_TX_DMA_BUFSIZE; i++)    /* ��ͣ */
            {
                g_audiodev.saibuf2[i] = 0;                  /* ���0 */
            }
        }
    }
    wavtransferend = 1;
}

/**
 * @brief       ��ȡ��ǰ����ʱ��
 * @param       fname : �ļ�ָ��
 * @param       wavx  : wavx���ſ�����
 * @retval      ��
 */
void wav_get_curtime(FIL *fx, __wavctrl *wavx)
{
    long long fpos;

    wavx->totsec = wavx->datasize / (wavx->bitrate / 8);    /* �����ܳ���(��λ:��) */
    fpos = fx->fptr-wavx->datastart;                        /* �õ���ǰ�ļ����ŵ��ĵط� */
    wavx->cursec = fpos*wavx->totsec / wavx->datasize;      /* ��ǰ���ŵ��ڶ�������? */
}


/**
 * @brief       wav�ļ�������˺���
 * @param       pos:��Ҫ��λ�����ļ�λ��
 * @retval      ����ֵ:��ǰ�ļ�λ��(����λ��Ľ��)
 */
uint32_t wav_file_seek(uint32_t pos)
{
    uint8_t temp;
    
    if (pos > g_audiodev.file->obj.objsize)
    {
        pos = g_audiodev.file->obj.objsize;
    }
    
    if (pos < wavctrl.datastart) pos = wavctrl.datastart;
    
    if (wavctrl.bps == 16) temp = 8;    /* ������8�ı��� */
    
    if (wavctrl.bps == 24) temp = 12;   /* ������12�ı��� */
    
    if ((pos - wavctrl.datastart) % temp)
    {
        pos += temp - (pos - wavctrl.datastart) % temp;
    }
    
    f_lseek(g_audiodev.file,pos);
    
    return g_audiodev.file->fptr;
}

/**
 * @brief       ����ĳ��wav�ļ�
 * @param       fname : �ļ�·��+�ļ���
 * @retval      KEY0_PRES,����
 *              KEY1_PRES,���ļ�ʧ��
 *              ����,��WAV�ļ�
 */
uint8_t wav_play_song(uint8_t* fname)
{
    uint8_t key;
    uint8_t t = 0; 
    uint8_t res;  
    uint32_t fillnum; 
    
    g_audiodev.file = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    g_audiodev.saibuf1 = mymalloc(SRAMIN, WAV_SAI_TX_DMA_BUFSIZE);
    g_audiodev.saibuf2 = mymalloc(SRAMIN, WAV_SAI_TX_DMA_BUFSIZE);
    g_audiodev.tbuf = mymalloc(SRAMIN, WAV_SAI_TX_DMA_BUFSIZE);
    g_audiodev.file_seek = wav_file_seek;
    
    if (g_audiodev.file && g_audiodev.saibuf1 && g_audiodev.saibuf2 && g_audiodev.tbuf)
    { 
        res = wav_decode_init(fname, &wavctrl); /* �õ��ļ�����Ϣ */
        
        if (res == 0)   /* �����ļ��ɹ� */
        {
            if (wavctrl.bps == 16)
            {
                es8388_sai_cfg(0, 3);       /* �����ֱ�׼,16λ���ݳ��� */
                sai1_saia_init(0, 1, 4);    /* ����SAI,������,16λ���� */
                sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, WAV_SAI_TX_DMA_BUFSIZE / 2, 1);    /* TX DMA 16λ�� */
            }
            else if (wavctrl.bps == 24)
            {
                es8388_sai_cfg(0, 0);       /* �����ֱ�׼,24λ���ݳ��� */
                sai1_saia_init(0, 1, 6);    /* ����SAI,������,24λ���� */
                sai1_tx_dma_init(g_audiodev.saibuf1, g_audiodev.saibuf2, WAV_SAI_TX_DMA_BUFSIZE / 4, 2);    /* TX DMA 32λ�� */
           }

            sai1_samplerate_set(wavctrl.samplerate);     /* ���ò����� */
            sai1_tx_callback = wav_sai_dma_tx_callback;  /* �ص�����ָwav_sai_dma_callback */
            audio_stop();

            res = f_open(g_audiodev.file, (TCHAR*)fname, FA_READ);    /* ���ļ� */ 
           
            if (res == 0)
            {
                f_lseek(g_audiodev.file, wavctrl.datastart);          /* �����ļ�ͷ */
                fillnum = wav_buffill(g_audiodev.saibuf1, WAV_SAI_TX_DMA_BUFSIZE, wavctrl.bps);
                fillnum = wav_buffill(g_audiodev.saibuf2, WAV_SAI_TX_DMA_BUFSIZE, wavctrl.bps);
                audio_start();  

                while (res == 0)
                { 
                    while(wavtransferend == 0)             /* �ȴ�wav�������; */
                    {
                        delay_ms(1000 / OS_TICKS_PER_SEC);
                    }
                    
                    wavtransferend = 0;
                    
                    if (wavwitchbuf) fillnum = wav_buffill(g_audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);/* ���buf2 */
                    else fillnum = wav_buffill(g_audiodev.saibuf1,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);/* ���buf1 */
                    
                    while(g_audiodev.status&(1<<1))/* ���������� */
                    {
                        wav_get_curtime(g_audiodev.file,&wavctrl);/* �õ���ʱ��͵�ǰ���ŵ�ʱ�� */
                        g_audiodev.totsec = wavctrl.totsec;       /* �������� */
                        g_audiodev.cursec = wavctrl.cursec;
                        g_audiodev.bitrate = wavctrl.bitrate;
                        g_audiodev.samplerate = wavctrl.samplerate;
                        g_audiodev.bps = wavctrl.bps;
                        
                        if(g_audiodev.status & 0X01) break;/* û�а�����ͣ */
                        else delay_ms(1000/OS_TICKS_PER_SEC);
                    }
                    
                    if ((g_audiodev.status & (1 << 1)) == 0 || (fillnum != WAV_SAI_TX_DMA_BUFSIZE)) /* �����������/������� */
                    {  
                        break;
                    }
                }
                audio_stop(); 
            }
            else res = AP_ERR; 
        }
        else res = AP_ERR;
    }
    else res = AP_ERR;
    
    myfree(SRAMIN,g_audiodev.tbuf);     /* �ͷ��ڴ� */
    myfree(SRAMIN,g_audiodev.saibuf1);  /* �ͷ��ڴ� */
    myfree(SRAMIN,g_audiodev.saibuf2);  /* �ͷ��ڴ� */
    myfree(SRAMIN,g_audiodev.file);     /* �ͷ��ڴ� */
    return res;
    
}