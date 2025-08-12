/**
 ****************************************************************************************************
 * @file        camera.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-1
 * @brief       APP-¼���� ����
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
 * V1.0 20221201
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "recorder.h"
#include "calendar.h"
#include "audioplay.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/SAI/sai.h"
#include "settings.h"


uint8_t *sairecbuf1;     /* I2S ���ջ���ָ��1 */
uint8_t *sairecbuf2;     /* I2S ���ջ���ָ��2 */

/**
 * REC¼��FIFO�������.
 * ����FATFS�ļ�д��ʱ��Ĳ�ȷ����,���ֱ���ڽ����ж�����д�ļ�,���ܵ���ĳ��д��ʱ�����
 * �Ӷ��������ݶ�ʧ,�ʼ���FIFO����,�Խ��������
 */
volatile uint8_t g_sai_recfifo_rdpos = 0;           /* ¼��FIFO ��λ�� */
volatile uint8_t g_sai_recfifo_wrpos = 0;           /* ¼��FIFO дλ�� */
uint8_t * p_sai_recfifo_buf[SAI_RX_FIFO_SIZE];  /* ¼������FIFO����ָ�� */


uint32_t g_wav_size;        /* wav���ݴ�С(�ֽ���,�������ļ�ͷ!!) */

uint8_t g_rec_sta = 0;      /**
                             * ¼��״̬
                             * [7]:0,û�п���¼��;1,�Ѿ�����¼��;
                             * [6:1]:����
                             * [0]:0,����¼��;1,��ͣ¼��;
                             */
                             
uint8_t *g_vubuf;           /* vu��buf,ֱ��ָ��sairecbuf1/sairecbuf2 */


#define RECORDER_TITLE_COLOR    0XFFFF      /* ¼����������ɫ */
#define RECORDER_TITLE_BKCOLOR  0X0000      /* ¼�������ⱳ��ɫ */

#define RECORDER_VU_BKCOLOR     0X39C7      /* VU Meter����ɫ */
#define RECORDER_VU_L1COLOR     0X07FF      /* VU Meter L1ɫ */
#define RECORDER_VU_L2COLOR     0xFFE0      /* VU Meter L2ɫ */
#define RECORDER_VU_L3COLOR     0xF800      /* VU Meter L3ɫ */

#define RECORDER_TIME_COLOR     0X07FF      /* ʱ����ɫ */
#define RECORDER_MAIN_BKCOLOR   0X18E3      /* ������ɫ */

/* ������Ƕ����ɫ */
#define RECORDER_INWIN_FONT_COLOR   0X736C  /* 0XAD53 */


uint8_t *const RECORDER_DEMO_PIC = "1:/SYSTEM/APP/RECORDER/Demo.bmp";       /* demoͼƬ·�� */
uint8_t *const RECORDER_RECR_PIC = "1:/SYSTEM/APP/RECORDER/RecR.bmp";       /* ¼�� �ɿ� */
uint8_t *const RECORDER_RECP_PIC = "1:/SYSTEM/APP/RECORDER/RecP.bmp";       /* ¼�� ���� */
uint8_t *const RECORDER_PAUSER_PIC = "1:/SYSTEM/APP/RECORDER/PauseR.bmp";   /* ��ͣ �ɿ� */
uint8_t *const RECORDER_PAUSEP_PIC = "1:/SYSTEM/APP/RECORDER/PauseP.bmp";   /* ��ͣ ���� */
uint8_t *const RECORDER_STOPR_PIC = "1:/SYSTEM/APP/RECORDER/StopR.bmp";     /* ֹͣ �ɿ� */
uint8_t *const RECORDER_STOPP_PIC = "1:/SYSTEM/APP/RECORDER/StopP.bmp";     /* ֹͣ ���� */

/* ¼������ѡ���б� */
uint8_t *const recorder_setsel_tbl[GUI_LANGUAGE_NUM][2] =
{
    {"����������", "MIC��������",},
    {"������O��", "MIC�����O��",},
    {"Samplerate set", "MIC gain set",},
};

/* ¼����������ʾ��Ϣ�� */
uint8_t *const recorder_sampleratemsg_tbl[5] = {"8KHz", "16Khz", "32Khz", "44.1Khz", "48Khz",};

/* ¼�������ʱ� */
const uint16_t recorder_samplerate_tbl[5] = {8000, 16000, 32000, 44100, 48000};

/* ¼����ʾ��Ϣ */
uint8_t *const recorder_remind_tbl[3][GUI_LANGUAGE_NUM] =
{
    "�Ƿ񱣴��¼���ļ�?", "�Ƿ񱣴�ԓ����ļ�?", "Do you want to save?",
    {"����ֹͣ¼��!", "Ո��ֹͣ���!", "Please stop REC first!",},
    {"�ڴ治��!!", "�ȴ治��!!", "Out of memory!",},
};

/* 00������ѡ������ */
uint8_t *const recorder_modesel_tbl[GUI_LANGUAGE_NUM] =
{
    "¼������", "����O��", "Recorder Set",
};


/**
 * @brief       ��ȡһ��¼��FIFO
 * @param       buf:  ���ݻ������׵�ַ
 * @retval      0, û�����ݿɶ�;
 *              1, ������1�����ݿ�;
 */
uint8_t rec_sai_fifo_read(uint8_t **buf)
{
    if (g_sai_recfifo_rdpos == g_sai_recfifo_wrpos)     /* ��λ��  =  дλ��, ˵��û�����ݿɶ� */
    {
        return 0;
    }
    
    g_sai_recfifo_rdpos++;          /* ��λ�ü�1 */

    if (g_sai_recfifo_rdpos >= SAI_RX_FIFO_SIZE)    /* ��λ�ó�������FIFO��, �������¿�ʼ */
    {
        g_sai_recfifo_rdpos = 0;    /* ���� */
    }
    
    *buf = p_sai_recfifo_buf[g_sai_recfifo_rdpos];      /* ���ض�ӦFIFO BUF�ĵ�ַ */

    return 1;
}

/**
 * @brief       дһ��¼��FIFO
 * @param       buf:  ���ݻ������׵�ַ
 * @retval      0, д��ɹ�;
 *              1, д��ʧ��;
 */
uint8_t rec_sai_fifo_write(uint8_t *buf)
{
    uint16_t i;
    uint8_t temp = g_sai_recfifo_wrpos; /* ��¼��ǰдλ�� */
    g_sai_recfifo_wrpos++;              /* дλ�ü�1 */

    if (g_sai_recfifo_wrpos >= SAI_RX_FIFO_SIZE)    /* дλ�ó�������FIFO��, �������¿�ʼ */
    {
        g_sai_recfifo_wrpos = 0;        /* ���� */
    }
    
    if (g_sai_recfifo_wrpos == g_sai_recfifo_rdpos)     /* дλ��  =  ��λ��, ˵��û��λ�ÿ�д�� */
    {
        g_sai_recfifo_wrpos = temp;     /* ��ԭԭ����дλ��,�˴�д��ʧ�� */
        return 1;
    }

    for (i = 0; i < SAI_RX_DMA_BUF_SIZE; i++)       /* ѭ��д���� */
    {
        p_sai_recfifo_buf[g_sai_recfifo_wrpos][i] = buf[i];
    }

    return 0;
}

/**
 * @brief       ¼�� SAI RX DMA�����жϷ�����
 *   @note      ���ж�����д������
 * @param       ��
 * @retval      ��
 */
void rec_sai_dma_rx_callback(void)
{
    if (DMA2_Stream5->CR & (1 << 19))
    {
        g_vubuf = sairecbuf1;    /* g_vubufָ��sairecbuf1 */

        if (g_rec_sta == 0X80)      /* ¼��ģʽ,����ͣ,��д�ļ� */
        {
            rec_sai_fifo_write(sairecbuf1);  /* sairecbuf1 д��FIFO */
        }
    }
    else
    {
        g_vubuf = sairecbuf2;    /* g_vubufָ��sairecbuf2 */

        if (g_rec_sta == 0X80)      /* ¼��ģʽ,����ͣ,��д�ļ� */
        {
            rec_sai_fifo_write(sairecbuf2);  /* sairecbuf1 д��FIFO */
        }
    }

}
const uint16_t saiplaybuf[2] = {0X0000, 0X0000}; /* 2��16λ����,����¼��ʱI2S Master����.ѭ������0 */

/**
 * @brief       ����PCM¼��ģʽ
 * @param       ��
 * @retval      ��
 */
void recorder_enter_rec_mode(void)
{
    es8388_adda_cfg(0, 1);          /* ����ADC */
    es8388_input_cfg(0);            /* ��������ͨ��(ͨ��1,MIC����ͨ��) */
    es8388_mic_gain(8);             /* MIC��������Ϊ��� */
    es8388_alc_ctrl(3, 4, 4);       /* ����������ALC����,�����¼������ */
    es8388_output_cfg(0, 0);        /* �ر�ͨ��1��2����� */
    es8388_spkvol_set(0);           /* �ر�����. */
    es8388_sai_cfg(0, 3);           /* �����ֱ�׼,16λ���ݳ��� */

    sai1_saia_init(0, 1, 4);        /* SAI1 Block A,������,16λ���� */
    sai1_saib_init(3, 1, 4);        /* SAI1 Block B��ģʽ����,16λ */
    
    sai1_samplerate_set(es8388set.mvol);    /* ���ò����� */
    
    sai1_tx_dma_init((uint8_t *)&saiplaybuf[0], (uint8_t *)&saiplaybuf[1],1,1); /* ����TX DMA,16λ */
    sai1_rx_dma_init(sairecbuf1, sairecbuf2, SAI_RX_DMA_BUF_SIZE / 2,1);        /* ����RX DMA */
    
    sai1_rx_callback = rec_sai_dma_rx_callback; /* ��ʼ���ص�����ָsai_rx_callback */
    
    SAI1_TX_DMASx->CR &= ~(1 << 4); /* �ر� TX DMA��������ж�(���ﲻ���ж�������) */
    
    sai1_play_start();              /* ��ʼSAI���ݷ���(����) */
    sai1_rec_start();               /* ��ʼSAI���ݽ���(�ӻ�) */
}

/**
 * @brief       ֹͣ¼��ģʽ
 * @param       ��
 * @retval      ��
 */
void recorder_stop_rec_mode(void)
{
    sai1_play_stop();           /* ֹͣSAI */
    sai1_rec_stop();
    
    es8388_adda_cfg(0, 0);      /* �ر�DAC&ADC */
    es8388_output_cfg(0, 0);    /* �ر�ͨ��1��2����� */
    app_es8388_volset(0);       /* �ر�ES8388������� */
}

/**
 * @brief       ����¼������Ƶ��
 * @param       wavhead         : wav�ļ�ͷ�ṹ��
 * @param       samplerate      : 8K~192K.
 * @retval      ��
 */
void recoder_set_samplerate(__WaveHeader *wavhead, uint16_t samplerate)
{
    sai1_play_stop();
    sai1_samplerate_set(samplerate);
    wavhead->fmt.SampleRate = samplerate;   /* ������,��λ:Hz */
    wavhead->fmt.ByteRate = wavhead->fmt.SampleRate * 4; /* �ֽ�����=������*ͨ����*(ADCλ��/8) */
    sai1_play_start();       /* ��ʼI2S���ݷ���(����) */
    sai1_play_start();        /* ��ʼI2S���ݽ���(�ӻ�) */
}

/**
 * @brief       ��ʼ��WAVͷ
 * @param       wavhead         : wav�ļ�ͷ�ṹ��
 * @retval      ��
 */
void recorder_wav_init(__WaveHeader *wavhead)
{
    wavhead->riff.ChunkID = 0X46464952; /* "RIFF" */
    wavhead->riff.ChunkSize = 0;        /* ��δȷ��,�����Ҫ���� */
    wavhead->riff.Format = 0X45564157;  /* "WAVE" */
    wavhead->fmt.ChunkID = 0X20746D66;  /* "fmt " */
    wavhead->fmt.ChunkSize = 16;        /* ��СΪ16���ֽ� */
    wavhead->fmt.AudioFormat = 0X01;    /* 0X01,��ʾPCM;0X01,��ʾIMA ADPCM */
    wavhead->fmt.NumOfChannels = 2;     /* ˫���� */
    wavhead->fmt.SampleRate = 0;        /* ������,��λ:Hz,������ȷ�� */
    wavhead->fmt.ByteRate = 0;          /* �ֽ�����=������*ͨ����*(ADCλ��/8),����ȷ�� */
    wavhead->fmt.BlockAlign = 4;        /* ���С=ͨ����*(ADCλ��/8) */
    wavhead->fmt.BitsPerSample = 16;    /* 16λPCM */
    wavhead->data.ChunkID = 0X61746164; /* "data" */
    wavhead->data.ChunkSize = 0;        /* ���ݴ�С,����Ҫ���� */
}

/* ��ƽ��ֵ�� */
const uint16_t vu_val_tbl[10] = {1200, 2400, 3600, 4800, 6000, 8000, 11000, 16000, 21000, 28000};

/**
 * @brief       ���źŵ�ƽ�õ�vu����ֵ
 * @param       signallevel     : �źŵ�ƽ
 * @retval      vuֵ
 */
uint8_t recorder_vu_get(uint16_t signallevel)
{
    uint8_t i;

    for (i = 10; i > 0; i--)
    {
        if (signallevel >= vu_val_tbl[i - 1])break;
    }

    return i;

}

/**
 * @brief       ��ʾVU Meter
 * @param       x,y             : ����
 * @param       level           : 0~10;
 * @retval      ��
 */
void recorder_vu_meter(uint16_t x, uint16_t y, uint8_t level)
{
    uint8_t i;
    uint16_t vucolor = RECORDER_VU_L1COLOR;

    if (level > 10)return ;

    if (level == 0)
    {
        gui_fill_rectangle(x, y, 218, 10, RECORDER_VU_BKCOLOR); /* ��䱳��ɫ */
        return;
    }

    for (i = 0; i < level; i++)
    {
        if (i == 9)vucolor = RECORDER_VU_L3COLOR;
        else if (i > 5)vucolor = RECORDER_VU_L2COLOR;

        gui_fill_rectangle(x + 22 * i, y, 20, 10, vucolor);     /* ��䱳��ɫ */
    }

    if (level < 10)gui_fill_rectangle(x + level * 22, y, 218 - level * 22, 10, RECORDER_VU_BKCOLOR);    /* ��䱳��ɫ */
}

/**
 * @brief       ��ʾ¼��ʱ��(��ʾ�ߴ�Ϊ:150*60)
 * @param       x,y             : ����
 * @param       tsec            : ������
 * @retval      ��
 */
void recorder_show_time(uint16_t x, uint16_t y, uint32_t tsec)
{
    uint8_t min;

    if (tsec >= 60 * 100)min = 99;
    else min = tsec / 60;

    gui_phy.back_color = RECORDER_MAIN_BKCOLOR;
    gui_show_num(x, y, 2, RECORDER_TIME_COLOR, 60, min, 0X80);  /* XX */
    gui_show_ptchar(x + 60, y, lcddev.width, lcddev.height, 0, RECORDER_TIME_COLOR, 60, ':', 0);    /* ":" */
    gui_show_num(x + 90, y, 2, RECORDER_TIME_COLOR, 60, tsec % 60, 0X80);   /* XX */
}

/**
 * @brief       ��ʾ����
 * @param       x,y             : ����
 * @param       name            : ����
 * @retval      ��
 */
void recorder_show_name(uint16_t x, uint16_t y, uint8_t *name)
{
    gui_fill_rectangle(x - 1, y - 1, lcddev.width, 13, RECORDER_MAIN_BKCOLOR);  /* ��䱳��ɫ */
    gui_show_ptstrwhiterim(x, y, lcddev.width, y + 12, 0, BLACK, WHITE, 12, name);
}

/**
 * @brief       ��ʾ������
 * @param       x,y             : ����(��Ҫ��0��ʼ)
 * @param       samplerate      : ������
 * @retval      ��
 */
void recorder_show_samplerate(uint16_t x, uint16_t y, uint16_t samplerate)
{
    uint8_t *buf = 0;
    float temp;
    temp = (float)samplerate / 1000;
    buf = gui_memin_malloc(60); /* �����ڴ� */

    if (buf == 0)return;

    if (samplerate % 1000)sprintf((char *)buf, "%.1fKHz", temp);    /* ��С���� */
    else sprintf((char *)buf, "%dKHz", samplerate / 1000);

    gui_fill_rectangle(x, y, 42, 12, RECORDER_MAIN_BKCOLOR);        /* ��䱳��ɫ */
    gui_show_string(buf, x, y, 42, 12, 12, RECORDER_INWIN_FONT_COLOR);  /* ��ʾagc */
    gui_memin_free(buf);        /* �ͷ��ڴ� */
}

/**
 * @brief       ����¼����������UI
 * @param       ��
 * @retval      ��
 */
void recorder_load_ui(void)
{
    gui_fill_rectangle(0, 0, lcddev.width, gui_phy.tbheight, RECORDER_TITLE_BKCOLOR);           /* ��䱳��ɫ */
    gui_show_strmid(0, 0, lcddev.width, gui_phy.tbheight, RECORDER_TITLE_COLOR, gui_phy.tbfsize, (uint8_t *)APP_MFUNS_CAPTION_TBL[11][gui_phy.language]); /* ��ʾ���� */
    gui_fill_rectangle(0, gui_phy.tbheight, lcddev.width, lcddev.height - gui_phy.tbheight, RECORDER_MAIN_BKCOLOR);             /* ����ɫ */
    minibmp_decode((uint8_t *)RECORDER_DEMO_PIC, (lcddev.width - 100) / 2, 100 + (lcddev.height - 320) / 2, 100, 100, 0, 0);    /* ����100*100��ͼƬDEMO */
    recorder_vu_meter((lcddev.width - 218) / 2, (lcddev.height - 320) / 2 + 200 + 5, 0);        /* ��ʾvu meter */
    app_gui_tcbar(0, lcddev.height - gui_phy.tbheight, lcddev.width, gui_phy.tbheight, 0x01);   /* �Ϸֽ��� */
}

/**
 * @brief       ͨ��ʱ���ȡ�ļ���
 *  @note       ������SD��/U�̱���,��֧��FLASH DISK����
 *  @note       ��ϳ�:����"0:PAINT/PAINT20120321210633.wav"/"2:PAINT/PAINT20120321210633.wav"���ļ���
 * @param       pname           : ��·��������
 * @retval      ��
 */
void recorder_new_pathname(uint8_t *pname)
{
    calendar_get_time(&calendar);
    calendar_get_date(&calendar);

    if (gui_phy.memdevflag & (1 << 0))sprintf((char *)pname, "0:RECORDER/REC%04d%02d%02d%02d%02d%02d.wav", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);        /* ��ѡ������SD�� */
    else if (gui_phy.memdevflag & (1 << 3))sprintf((char *)pname, "3:RECORDER/REC%04d%02d%02d%02d%02d%02d.wav", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);   /* SD��������,�򱣴���U�� */
}

/**
 * @brief       ��ʾAGC��С
 * @param       x,y             : ����
 * @param       agc             : ����ֵ 0~8,��Ӧ0dB~24dB,3dB/Step
 * @retval      ��
 */
void recorder_show_agc(uint16_t x, uint16_t y, uint8_t agc)
{
    uint8_t *buf;
    float temp;
    buf = gui_memin_malloc(60); /* �����ڴ� */

    if (buf == 0)return;

    temp = agc * 3;
    sprintf((char *)buf, "%.2fdB", temp);
    gui_phy.back_color = APP_WIN_BACK_COLOR;                /* ���ñ���ɫΪ��ɫ */
    gui_fill_rectangle(x, y, 48, 12, APP_WIN_BACK_COLOR);   /* ��䱳��ɫ */
    gui_show_string(buf, x, y, 48, 12, 12, RECORDER_INWIN_FONT_COLOR);  /* ��ʾagc */
    gui_memin_free(buf);/* �ͷ��ڴ� */
}

/**
 * @brief       agc���ý���.�̶��ߴ�:180*122
 * @param       x,y             : ����
 * @param       agc             : �Զ�����ָ��,��Χ:0~63,��Ӧ-12dB~35.25dB,0.75dB/Step
 * @param       caption         : ��������
 * @retval      0,�ɹ�����;
 *              ����,����������
 */
uint8_t recorder_agc_set(uint16_t x, uint16_t y, uint8_t *agc, uint8_t *caption)
{
    uint8_t rval = 0, res;
    _window_obj *twin = 0;      /* ���� */
    _btn_obj *rbtn = 0;         /* ȡ����ť */
    _btn_obj *okbtn = 0;        /* ȷ����ť */
    _progressbar_obj *agcprgb;  /* AGC���ý����� */
    uint8_t tempagc = *agc;

    twin = window_creat(x, y, 180, 122, 0, 1 | 1 << 5, 16);     /* �������� */
    agcprgb = progressbar_creat(x + 10, y + 52, 160, 15, 0X20); /* ���������� */

    if (agcprgb == NULL)rval = 1;

    okbtn = btn_creat(x + 20, y + 82, 60, 30, 0, 0x02);         /* ������ť */
    rbtn = btn_creat(x + 20 + 60 + 20, y + 82, 60, 30, 0, 0x02);/* ������ť */

    if (twin == NULL || rbtn == NULL || okbtn == NULL || rval)rval = 1;
    else
    {
        /* ���ڵ����ֺͱ���ɫ */
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;
        
        /* ���ذ�ť����ɫ */
        rbtn->bkctbl[0] = 0X8452;   /* �߿���ɫ */
        rbtn->bkctbl[1] = 0XAD97;   /* ��һ�е���ɫ */
        rbtn->bkctbl[2] = 0XAD97;   /* �ϰ벿����ɫ */
        rbtn->bkctbl[3] = 0X8452;   /* �°벿����ɫ */
        okbtn->bkctbl[0] = 0X8452;  /* �߿���ɫ */
        okbtn->bkctbl[1] = 0XAD97;  /* ��һ�е���ɫ */
        okbtn->bkctbl[2] = 0XAD97;  /* �ϰ벿����ɫ */
        okbtn->bkctbl[3] = 0X8452;  /* �°벿����ɫ */
        
        agcprgb->totallen = 8;      /* ���AGCΪ8 */
        agcprgb->curpos = tempagc;  /* ��ǰ�ߴ� */
        rbtn->caption = (uint8_t *)GUI_CANCEL_CAPTION_TBL[gui_phy.language];    /* ����Ϊȡ�� */
        okbtn->caption = (uint8_t *)GUI_OK_CAPTION_TBL[gui_phy.language];       /* ����Ϊȷ�� */
        window_draw(twin);          /* �������� */
        btn_draw(rbtn);             /* ����ť */
        btn_draw(okbtn);            /* ����ť */
        progressbar_draw_progressbar(agcprgb);
        gui_show_string("AGC:", x + 10, y + 38, 24, 12, 12, RECORDER_INWIN_FONT_COLOR); /* ��ʾSIZE */
        recorder_show_agc(x + 10 + 24, y + 38, tempagc);

        while (rval == 0)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* �õ�������ֵ */
            delay_ms(1000 / OS_TICKS_PER_SEC);      /* ��ʱһ��ʱ�ӽ��� */

            if (system_task_return)
            {
                delay_ms(10);
                if (tpad_scan(1)) 
                {
                    break;  /* TPAD����,�ٴ�ȷ��,�ų����� */
                }
                else system_task_return = 0;
            }

            res = btn_check(rbtn, &in_obj);         /* ȡ����ť��� */

            if (res && ((rbtn->sta & 0X80) == 0))   /* ����Ч���� */
            {
                rval = 1;
                break;/* �˳� */
            }

            res = btn_check(okbtn, &in_obj);        /* ȷ�ϰ�ť��� */

            if (res && ((okbtn->sta & 0X80) == 0))  /* ����Ч���� */
            {
                rval = 0XFF;
                break;/* ȷ���� */
            }

            res = progressbar_check(agcprgb, &in_obj);

            if (res && (tempagc != agcprgb->curpos))    /* �������Ķ��� */
            {
                tempagc = agcprgb->curpos;      /* �������µĽ�� */
                recorder_show_agc(x + 10 + 24, y + 38, tempagc);
                es8388_mic_gain(tempagc);       /* �������� */
            }
        }
    }

    window_delete(twin);        /* ɾ������ */
    btn_delete(rbtn);           /* ɾ����ť */
    progressbar_delete(agcprgb);/* ɾ�������� */
    system_task_return = 0;

    if (rval == 0XFF)
    {
        *agc = tempagc;
        return 0;
    }

    return rval;
}

/**
 * @brief       ¼����
 *  @note       ����¼���ļ�,��������SD��RECORDER�ļ�����.
 * @param       ��
 * @retval      ��
 */
uint8_t recorder_play(void)
{
    uint8_t res;
    uint8_t rval = 0;
    __WaveHeader *wavhead = 0;
    FIL * f_rec;
    uint8_t *pdatabuf;
    uint8_t *pname = 0;
    _btn_obj *rbtn = 0;         /* ȡ����ť */
    _btn_obj *mbtn = 0;         /* ѡ�ť */
    _btn_obj *recbtn = 0;       /* ¼����ť */
    _btn_obj *stopbtn = 0;      /* ֹͣ¼����ť */

    uint16_t *pset_bkctbl = 0;  /* ����ʱ����ɫָ�� */
    uint32_t recsec = 0;        /* ¼��ʱ�� */

    uint8_t timecnt = 0;
    uint8_t vulevel = 0;
    short *tempvubuf;
    short tempval;
    uint16_t maxval = 0;
    uint16_t i;

    uint8_t recspd = 1;     /* ¼��������ѡ��,Ĭ��ѡ��16Khz¼�� */
    uint8_t recagc = 5;     /* Ĭ������Ϊ5*3dB */
    
    uint8_t *p;             /* ָ�� */
    uint8_t *buf;           /* ���� */
    uint32_t readlen;       /* �ܶ�ȡ���� */
    uint16_t bread;         /* ��ȡ�ĳ��� */

    /* �����ڴ� */
    for (i = 0; i < SAI_RX_FIFO_SIZE; i++)
    {
        p_sai_recfifo_buf[i] = gui_memin_malloc(SAI_RX_DMA_BUF_SIZE);   /* I2S¼��FIFO�ڴ����� */

        if (p_sai_recfifo_buf[i] == NULL)
        {
            break;  /* ����ʧ�� */
        }
    }

    sairecbuf1 = gui_memin_malloc(SAI_RX_DMA_BUF_SIZE);  /* I2S¼���ڴ�1���� */
    sairecbuf2 = gui_memin_malloc(SAI_RX_DMA_BUF_SIZE);  /* I2S¼���ڴ�2���� */
    
    f_rec = (FIL *)gui_memin_malloc(sizeof(FIL));       /* ����FIL�ֽڵ��ڴ����� */
    wavhead = (__WaveHeader *)gui_memin_malloc(sizeof(__WaveHeader)); /* ����__WaveHeader�ֽڵ��ڴ����� */
    pname = gui_memin_malloc(60);                       /* ����30���ֽ��ڴ�,����"0:RECORDER/REC20120321210633.wav" */
    pset_bkctbl = gui_memex_malloc(180 * 272 * 2);      /* Ϊ����ʱ�ı���ɫ�������ڴ� */
    buf = gui_memin_malloc(1024);                       /* ��buf����1024�ֽڻ��� */

    if (!sairecbuf2 || !f_rec || !wavhead || !buf || !pset_bkctbl)rval = 1;
    else
    {
        /* �������� */
        res = f_open(f_rec, (const TCHAR *)APP_ASCII_S6030, FA_READ); /* ���ļ��� */

        if (res == FR_OK)
        {
            asc2_s6030 = (uint8_t *)gui_memex_malloc(f_rec->obj.objsize);   /* Ϊ�����忪�ٻ����ַ */

            if (asc2_s6030 == 0)rval = 1;
            else
            {
                p = asc2_s6030;
                readlen = 0;

                while (readlen < f_rec->obj.objsize)    /* ѭ����ȡ */
                {
                    res = f_read(f_rec, buf, 1024, (UINT *)&bread); /* ����rec��������� */
                    readlen += bread;
                    my_mem_copy(p, buf, bread);
                    p += bread;

                    if (res)break;
                }
            }
        }

        if (res)rval = 1;

        recorder_load_ui(); /* װ�������� */
        rbtn = btn_creat(lcddev.width - 2 * gui_phy.tbfsize - 8 - 1, lcddev.height - gui_phy.tbheight, 2 * gui_phy.tbfsize + 8, gui_phy.tbheight - 1, 0, 0x03); /* �������ְ�ť */
        mbtn = btn_creat(0, lcddev.height - gui_phy.tbheight, 2 * gui_phy.tbfsize + 8, gui_phy.tbheight - 1, 0, 0x03);  /* �������ְ�ť */
        recbtn = btn_creat((lcddev.width - 96) / 3, (lcddev.height - 320) / 2 + 215 + 18, 48, 48, 0, 1);                /* ����ͼƬ��ť */
        stopbtn = btn_creat((lcddev.width - 96) * 2 / 3 + 48, (lcddev.height - 320) / 2 + 215 + 18, 48, 48, 0, 1);      /* ����ͼƬ��ť */

        if (!rbtn || !mbtn || !recbtn || !stopbtn)rval = 1; /* û���㹻�ڴ湻���� */
        else
        {
            rbtn->caption = (uint8_t *)GUI_BACK_CAPTION_TBL[gui_phy.language]; /* ���� */
            rbtn->font = gui_phy.tbfsize;   /* �����µ������С */
            rbtn->bcfdcolor = WHITE;        /* ����ʱ����ɫ */
            rbtn->bcfucolor = WHITE;        /* �ɿ�ʱ����ɫ */

            mbtn->caption = (uint8_t *)GUI_OPTION_CAPTION_TBL[gui_phy.language]; /* ���� */
            mbtn->font = gui_phy.tbfsize;   /* �����µ������С */
            mbtn->bcfdcolor = WHITE;        /* ����ʱ����ɫ */
            mbtn->bcfucolor = WHITE;        /* �ɿ�ʱ����ɫ */

            recbtn->picbtnpathu = (uint8_t *)RECORDER_RECR_PIC;
            recbtn->picbtnpathd = (uint8_t *)RECORDER_PAUSEP_PIC;
            recbtn->bcfucolor = 0X0001;     /* ����䱳�� */
            recbtn->bcfdcolor = 0X0001;     /* ����䱳�� */
            recbtn->sta = 0;

            stopbtn->picbtnpathu = (uint8_t *)RECORDER_STOPR_PIC;
            stopbtn->picbtnpathd = (uint8_t *)RECORDER_STOPP_PIC;
            stopbtn->bcfucolor = 0X0001;    /* ����䱳�� */
            stopbtn->bcfdcolor = 0X0001;    /* ����䱳�� */
            recbtn->sta = 0;
        }
    }

    if (rval == 0)
    {
        if (gui_phy.memdevflag & (1 << 0))f_mkdir("0:RECORDER");    /* ǿ�ƴ����ļ���,��¼������ */

        if (gui_phy.memdevflag & (1 << 3))f_mkdir("3:RECORDER");    /* ǿ�ƴ����ļ���,��¼������ */

        btn_draw(rbtn);
        btn_draw(mbtn);
        btn_draw(recbtn);
        recbtn->picbtnpathu = (uint8_t *)RECORDER_PAUSER_PIC;
        recbtn->picbtnpathd = (uint8_t *)RECORDER_RECP_PIC;
        btn_draw(stopbtn);

        if (g_audiodev.status & (1 << 7))   /* ��ǰ�ڷŸ�??����ֹͣ */
        {
            audio_stop_req(&g_audiodev);    /* ֹͣ��Ƶ���� */
            audio_task_delete();            /* ɾ�����ֲ������� */
        }

        tempvubuf = 0;
        recsec = 0;
        g_rec_sta = 0;
        g_wav_size = 0;
        recorder_enter_rec_mode();          /* ����¼��ģʽ,������AGC */
        recoder_set_samplerate(wavhead, recorder_samplerate_tbl[recspd]); /* ���ò����� */
        recorder_show_samplerate((lcddev.width - 218) / 2, (lcddev.height - 320) / 2 + 200 + 5 - 15, recorder_samplerate_tbl[recspd]); /* ��ʾ������ */
        es8388_mic_gain(recagc);            /* �������� */
        recorder_show_time((lcddev.width - 150) / 2, 40 + (lcddev.height - 320) / 2, recsec); /* ��ʾʱ�� */

        while (rval == 0)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* �õ�������ֵ */
            

            if (rec_sai_fifo_read(&pdatabuf) && (g_rec_sta & 0X80)) /* ��ȡһ������,����������,д���ļ� */
            {
                res = f_write(f_rec, pdatabuf, SAI_RX_DMA_BUF_SIZE, (UINT *)&bread);    /* д���ļ� */

                if (res)
                {
                    printf("write error:%d\r\n", res);
                }

                g_wav_size += SAI_RX_DMA_BUF_SIZE;
            }
            else
            {
                timecnt++;

                if ((timecnt % 20) == 0)
                {
                    tempvubuf = (short *)g_vubuf;

                    for (i = 0; i < 512; i++)   /* ȡǰ512��������������ֵ */
                    {
                        tempval = tempvubuf[i];

                        if (tempval < 0)tempval = -tempval;

                        if (maxval < tempval)maxval = tempval;  /* ȡ���ֵ */
                    }

                    tempval = recorder_vu_get(maxval);

                    if (tempval > vulevel)vulevel = tempval;
                    else if (vulevel)vulevel--;

                    recorder_vu_meter((lcddev.width - 218) / 2, (lcddev.height - 320) / 2 + 200 + 5, vulevel); /* ��ʾvu meter */
                    maxval = 0;
                }

                delay_ms(1000 / OS_TICKS_PER_SEC);  /* ��ʱһ��ʱ�ӽ��� */

                if (recsec != (g_wav_size / wavhead->fmt.ByteRate)) /* ¼��ʱ����ʾ */
                {
                    recsec = g_wav_size / wavhead->fmt.ByteRate;    /* ¼��ʱ�� */
                    recorder_show_time((lcddev.width - 150) / 2, 40 + (lcddev.height - 320) / 2, recsec); /* ��ʾʱ�� */
                }
            }
            
            if (system_task_return)
            {
                delay_ms(10);
                if (tpad_scan(1)) 
                {
                    break;  /* TPAD����,�ٴ�ȷ��,�ų����� */
                }
                else system_task_return = 0;
            }

            res = btn_check(rbtn, &in_obj);         /* ��鷵�ذ�ť */

            if (res && ((rbtn->sta & (1 << 7)) == 0) && (rbtn->sta & (1 << 6)))break; /* ���ذ�ť */

            res = btn_check(mbtn, &in_obj);         /* ������ð�ť */

            if (res && ((mbtn->sta & (1 << 7)) == 0) && (mbtn->sta & (1 << 6)))
            {
                app_read_bkcolor((lcddev.width - 180) / 2, (lcddev.height - 272) / 2, 180, 272, pset_bkctbl); /* ��ȡ����ɫ */
                res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 152) / 2, 180, 72 + 40 * 2, (uint8_t **)recorder_setsel_tbl[gui_phy.language], 2, (uint8_t *)&rval, 0X90, (uint8_t *)recorder_modesel_tbl[gui_phy.language]); /* 2��ѡ�� */

                if (res == 0)
                {
                    app_recover_bkcolor((lcddev.width - 180) / 2, (lcddev.height - 272) / 2, 180, 272, pset_bkctbl); /* �ָ�����ɫ */

                    if (rval == 0)  /* ���ò����� */
                    {
                        if (g_rec_sta & 0X80) /* ����¼��?��ʾ�û������ֹ¼�������ò����� */
                        {
                            window_msg_box((lcddev.width - 180) / 2, (lcddev.height - 80) / 2, 180, 80, (uint8_t *)recorder_remind_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
                            delay_ms(1500);
                        }
                        else
                        {
                            res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 272) / 2, 180, 72 + 40 * 5, (uint8_t **)recorder_sampleratemsg_tbl, 5, (uint8_t *)&recspd, 0X90, (uint8_t *)recorder_setsel_tbl[gui_phy.language][0]); /* ��ѡ */

                            if (res == 0)recoder_set_samplerate(wavhead, recorder_samplerate_tbl[recspd]); /* ���ò����� */
                        }
                    }
                    else    /* ����AGC���� */
                    {
                        res = recorder_agc_set((lcddev.width - 180) / 2, (lcddev.height - 122) / 2, &recagc, (uint8_t *)recorder_setsel_tbl[gui_phy.language][1]); /* ����AGC */
                        es8388_mic_gain(recagc); /* �������� */
                    }
                }

                rval = 0;   /* �ָ�rval��ֵ */
                app_recover_bkcolor((lcddev.width - 180) / 2, (lcddev.height - 272) / 2, 180, 272, pset_bkctbl); /* �ָ�����ɫ */
                recorder_show_samplerate((lcddev.width - 218) / 2, (lcddev.height - 320) / 2 + 200 + 5 - 15, recorder_samplerate_tbl[recspd]); /* ��ʾ������ */
            }

            res = btn_check(recbtn, &in_obj);   /* ���¼����ť */

            if (res && ((recbtn->sta & (1 << 7)) == 0) && (recbtn->sta & (1 << 6)))
            {
                if (g_rec_sta & 0X01)     /* ԭ������ͣ,����¼�� */
                {
                    g_rec_sta &= 0XFE;    /* ȡ����ͣ */
                    recbtn->picbtnpathu = (uint8_t *)RECORDER_RECR_PIC;
                    recbtn->picbtnpathd = (uint8_t *)RECORDER_PAUSEP_PIC;
                }
                else if (g_rec_sta & 0X80)     /* �Ѿ���¼����,��ͣ */
                {
                    g_rec_sta |= 0X01;    /* ��ͣ */
                    recbtn->picbtnpathu = (uint8_t *)RECORDER_PAUSER_PIC;
                    recbtn->picbtnpathd = (uint8_t *)RECORDER_RECP_PIC;
                }
                else    /* ��û��ʼ¼�� */
                {
                    g_rec_sta |= 0X80;    /* ��ʼ¼�� */
                    g_wav_size = 0;        /* �ļ���С����Ϊ0 */
                    recbtn->picbtnpathu = (uint8_t *)RECORDER_RECR_PIC;
                    recbtn->picbtnpathd = (uint8_t *)RECORDER_PAUSEP_PIC;
                    pname[0] = '\0';    /* ��ӽ����� */
                    recorder_new_pathname(pname);   /* �õ��µ����� */
                    recorder_show_name(2, gui_phy.tbheight + 4, pname); /* ��ʾ���� */
                    recorder_wav_init(wavhead);     /* ��ʼ��wav���� */
                    recoder_set_samplerate(wavhead, recorder_samplerate_tbl[recspd]); /* ���ò����� */
                    res = f_open(f_rec, (const TCHAR *)pname, FA_CREATE_ALWAYS | FA_WRITE);

                    if (res)    /* �ļ�����ʧ�� */
                    {
                        g_rec_sta = 0;    /* �����ļ�ʧ��,����¼�� */
                        rval = 0XFE;    /* ��ʾ�Ƿ����SD�� */
                    }
                    else res = f_write(f_rec, (const void *)wavhead, sizeof(__WaveHeader), (UINT *)&bread); /* д��ͷ���� */
                }
            }

            res = btn_check(stopbtn, &in_obj);  /* ���ֹͣ��ť */

            if (res && ((recbtn->sta & (1 << 7)) == 0) && (recbtn->sta & (1 << 6)))
            {
                if (g_rec_sta & 0X80) /* ��¼�� */
                {
                    wavhead->riff.ChunkSize = g_wav_size + 36; /* �����ļ��Ĵ�С-8; */
                    wavhead->data.ChunkSize = g_wav_size;      /* ���ݴ�С */
                    f_lseek(f_rec, 0);                      /* ƫ�Ƶ��ļ�ͷ */
                    f_write(f_rec, (const void *)wavhead, sizeof(__WaveHeader), (UINT *)&bread); /* д��ͷ���� */
                    f_close(f_rec);
                    g_wav_size = 0;

                    recbtn->picbtnpathu = (uint8_t *)RECORDER_RECR_PIC;
                    recbtn->picbtnpathd = (uint8_t *)RECORDER_PAUSEP_PIC;
                    btn_draw(recbtn);
                    recbtn->picbtnpathu = (uint8_t *)RECORDER_PAUSER_PIC;
                    recbtn->picbtnpathd = (uint8_t *)RECORDER_RECP_PIC;
                }

                g_rec_sta = 0;
                recsec = 0;
                recorder_show_name(2, gui_phy.tbheight + 4, "");    /* ��ʾ���� */
                recorder_show_time((lcddev.width - 150) / 2, 40 + (lcddev.height - 320) / 2, recsec); /* ��ʾʱ�� */
            }
        }
        
        recorder_stop_rec_mode();/* ֹͣ¼�� */
    }
    else
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)recorder_remind_tbl[2][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* �ڴ���� */
        delay_ms(2000);
    }

    if (rval == 0XFE)       /* �����ļ�ʧ����,��Ҫ��ʾ�Ƿ����SD�� */
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)APP_CREAT_ERR_MSG_TBL[gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* ��ʾSD���Ƿ���� */
        delay_ms(2000);     /* �ȴ�2���� */
    }

    if (g_rec_sta & 0X80)     /* �������¼��,����ʾ�������¼���ļ� */
    {
        res = window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, "", (uint8_t *)recorder_remind_tbl[0][gui_phy.language], 12, 0, 0X03, 0);

        if (res == 1)       /* ��Ҫ���� */
        {
            wavhead->riff.ChunkSize = g_wav_size + 36; /* �����ļ��Ĵ�С-8; */
            wavhead->data.ChunkSize = g_wav_size;      /* ���ݴ�С */
            f_lseek(f_rec, 0);                      /* ƫ�Ƶ��ļ�ͷ */
            f_write(f_rec, (const void *)wavhead, sizeof(__WaveHeader), (UINT *)&bread); /* д��ͷ���� */
            f_close(f_rec);
        }
    }

    /* �ͷ��ڴ� */
    for (i = 0; i < SAI_RX_FIFO_SIZE; i++)
    {
        gui_memin_free(p_sai_recfifo_buf[i]);   /* I2S¼��FIFO�ڴ��ͷ� */
    }
    
    gui_memin_free(sairecbuf1);
    gui_memin_free(sairecbuf2);
    gui_memin_free(f_rec);
    gui_memin_free(wavhead);
    gui_memin_free(pname);
    gui_memex_free(pset_bkctbl);
    gui_memex_free(asc2_s6030);
    gui_memin_free(buf);
    asc2_s6030 = 0; /* ���� */
    btn_delete(rbtn);
    btn_delete(mbtn);
    btn_delete(recbtn);
    btn_delete(stopbtn);
    return rval;
}



































