#include "appplay_spdifrx.h"  
#include "audioplay.h"
#include "./BSP/SAI/sai.h"


uint32_t *spdif_audiobuf[2];                    /* SPDIF��Ƶ���ݽ��ջ�����,��2��(˫����) */
uint32_t *spdif_vubuf = 0;

#define SPDIF_BACK_COLOR        0X2945          /* �����汳��ɫ */

#define SPDIF_VU_BKCOLOR        0X39C7          /* VU Meter����ɫ */
#define SPDIF_VU_L1COLOR        0X07FF          /* VU Meter L1ɫ */
#define SPDIF_VU_L2COLOR        0xFFE0          /* VU Meter L2ɫ */
#define SPDIF_VU_L3COLOR        0xF800          /* VU Meter L3ɫ */
#define SPDIF_VU_LEVEL          12              /* �ܼ���,������ż�� */

uint8_t* const SPDIF_RECORD_PIC[3]=
{ 
    "1:/SYSTEM/APP/APPS/SPDIF/record_180.bmp",  /* demoͼƬ·�� */
    "1:/SYSTEM/APP/APPS/SPDIF/record_220.bmp",  /* demoͼƬ·�� */
    "1:/SYSTEM/APP/APPS/SPDIF/record_320.bmp",  /* demoͼƬ·�� */
};
 
/**
 * @brief       ��ʾ������
 * @param       y:y����(x�����Զ�����)
 * @param       samplerate:��Ƶ������(��λ:Hz)
 * @param       fsr:��������ʾ������
 * @retval      ��
 */
void spdif_show_samplerate(uint16_t y,uint32_t samplerate,uint8_t fsr)
{  
    uint8_t *buf;
    uint16_t x;
    uint16_t lenth;
    float rate = (float)samplerate / 1000; 
    buf = mymalloc(SRAMIN,100);                                         /* �����ڴ� */
    
    if (buf)                                                            /* ����ɹ� */
    {
        if (samplerate)
        {
            if (samplerate % 1000) sprintf((char*)buf,"%2.1fKHz",rate); /* ��ӡ������ */
            else sprintf((char*)buf,"%dKHz",samplerate / 1000);         /* ��ӡ������ */
        }
        else sprintf((char*)buf,"Detecting...");                        /* ��ʾ����� */
        
        lenth = strlen((char*)buf);
        lenth = fsr * lenth / 2;
        
        x = (lcddev.width - lenth) / 2; 
        gui_fill_rectangle((lcddev.width - fsr * 6) / 2,y,fsr * 6,fsr,SPDIF_BACK_COLOR); 
        gui_show_string(buf,x,y,200,fsr,fsr,WHITE);                     /* ��ʾ������ */
    }
    
    myfree(SRAMIN,buf);                                                 /* �ͷ��ڴ� */
}

/**
 * @brief       SAI DMA��������жϻص�����
 * @param       ��
 * @retval      ��
 */
void sai_dma_tx_callback(void)
{ 
    if (DMA2_Stream3->CR & (1 << 19))       /* buf1�ѿ�,��������buf0 */
    { 
        spdif_vubuf = spdif_audiobuf[0];    /* ����buf0 */
        
    }
    else                                    /* buf0�ѿ�,��������buf1 */
    {
        spdif_vubuf = spdif_audiobuf[1];    /* ����buf1 */
    }
}

/**
 * @brief       SPDIF RX����ʱ�Ļص�����
 * @param       ��
 * @retval      ��
 */
void spdif_rx_stopplay_callback(void)
{
    sai1_play_stop();
    SPDIFRX->IFCR |= 1 << 5;                /* ���ͬ����ɱ�־ */
    spdif_dev.samplerate = 0;
    memset((uint8_t*)spdif_audiobuf[0],0,SPDIF_DBUF_SIZE * 4);   
    memset((uint8_t*)spdif_audiobuf[1],0,SPDIF_DBUF_SIZE * 4);
}

/* ��ƽ��ֵ�� */
const uint16_t spdif_vu_val_tbl[SPDIF_VU_LEVEL] = {600,1200,2400,3600,4800,6000,8000,11000,13000,16000,21000,28000};

/**
 * @brief       ���źŵ�ƽ�õ�vu����ֵ
 * @param       signallevel:�źŵ�ƽ
 * @retval      ����ֵ:vuֵ
 */
uint8_t spdif_vu_get(uint16_t signallevel)
{
    uint8_t i;
    
    for (i = SPDIF_VU_LEVEL;i > 0;i --)
    {
        if (signallevel >= spdif_vu_val_tbl[i - 1]) break;
    }
    
    return i; 
}

/**
 * @brief       ��ʾVU Meter
 * @param       width:���,�߶�=���/2,���=���/8
 * @param       x,y:����
 * @param       level:0~10
 * @retval      ��
 */
void spdif_vu_meter(uint8_t width,uint16_t x,uint16_t y,uint8_t level)
{
    uint8_t i; 
    uint16_t pitch = 0; /* 2��level֮��ļ�϶ */
    uint16_t height = 0;
    uint16_t vucolor = SPDIF_VU_L1COLOR;
    
    if (level > SPDIF_VU_LEVEL) return ;
    
    pitch = width / 8;
    height = pitch * (SPDIF_VU_LEVEL - 1) + width * SPDIF_VU_LEVEL / 2;
    
    if (level == 0)
    {
        gui_fill_rectangle(x,y,width,height,SPDIF_VU_BKCOLOR);  /* ��䱳��ɫ */
        return;
    }
    
    for (i = 0;i < level;i++)
    {
        if (i == (SPDIF_VU_LEVEL - 1)) vucolor = SPDIF_VU_L3COLOR;
        else if (i > SPDIF_VU_LEVEL / 2) vucolor = SPDIF_VU_L2COLOR;
        
        gui_fill_rectangle(x,y + height - width / 2 - (width / 2 + pitch) * i,width,width / 2,vucolor); /* ��䱳��ɫ */
    }
    
    if (level < 10)gui_fill_rectangle(x,y,width,height - level * (width / 2 + pitch),SPDIF_VU_BKCOLOR); /* ��䱳��ɫ */
} 

/**
 * @brief       SPDIF RXӦ�ã�ͨ��SPDIF RX�ӿ�,���չ�����Ƶ����,�����������
 * @param       ��
 * @retval      ��
 */
uint8_t appplay_spdifrx(uint8_t* caption)
{  
    uint8_t res;
    uint16_t lastvolpos; 
    uint32_t lastsamplerate = 0;
    
    uint16_t i;
    uint8_t timecnt = 0;
    uint8_t vulevel[2];                 /* �������� */
    short tempval;
    uint16_t maxval = 0;
    
    
    uint8_t fsr;                        /* ����������,12/16/24 */
    uint16_t sry;                       /* �����ʵ�y���� */
    
    uint16_t bmpx,bmpy,bmpw;            /* ��ƬͼƬ��x,y����͸߶� */
    uint8_t bmpidx;                     /* ��ƬͼƬ���� */
    
    uint16_t vpbh,vpbw,vpbx,vpby;       /* �����������ĸ߶�/���/x,y���� */
    uint16_t vbmpx,vbmpy;               /* ����ͼ���x,y���� */
    
    uint16_t vux,vuy,vuwidth,vuheight;  /* vu x,y����,vu���Ŀ��/�߶� */
    
    uint16_t ydis1,ydis2,ydis3;         /* 3�������� */
    uint16_t vuoffx;                    /* vu x�����ƫ�ƺ��볪Ƭͼ��ļ�϶ */
    uint16_t vbmpoffx,vbmpdis;          /* ����ͼ���xƫ�ƺ��������������ļ�϶ */

    _progressbar_obj*volprgb;           /* ���������� */
    
    if (lcddev.width == 240)
    {
        fsr = 12;
        bmpidx = 0;
        bmpw = 180;
        vuoffx = 4; 
        vuwidth = 16;
        vbmpoffx = 20;
        vbmpdis = 1;
        ydis1 = 30;ydis2 = 12;ydis3 = 20;
        vpbh = 12;
    }
    else if (lcddev.width == 272)
    {
        fsr = 16;
        bmpidx = 1;
        bmpw = 220;
        vuoffx = 3; 
        vuwidth = 16;
        vbmpoffx = 26;
        vbmpdis = 2;
        ydis1 = 70;ydis2 = 10;ydis3 = 54;
        vpbh = 16;
    }
    else if (lcddev.width == 320)
    {
        fsr = 16;
        bmpidx = 1;
        bmpw = 220;
        vuoffx = 10; 
        vuwidth = 24;
        vbmpoffx = 30;
        vbmpdis = 2;
        ydis1 = 70;ydis2 = 10;ydis3 = 54;
        vpbh = 16;
    }
    else if (lcddev.width == 480)
    {
        fsr = 24;
        bmpidx = 2;
        bmpw = 320;
        vuoffx = 20; 
        vuwidth = 40;
        vbmpoffx = 50;
        vbmpdis = 2;
        ydis1 = 130;ydis2 = 18;ydis3 = 130;
        vpbh = 16;
    }
    else if (lcddev.width == 600)
    {
        fsr = 24;
        bmpidx = 2;
        bmpw = 320;
        vuoffx = 40; 
        vuwidth = 40;
        vbmpoffx = 80;
        vbmpdis = 2;
        ydis1 = 220;ydis2 = 20;ydis3 = 172;
        vpbh = 16;
    }
    else if (lcddev.width == 800)
    {
        fsr = 24;
        bmpidx = 2;
        bmpw = 320;
        vuoffx = 80; 
        vuwidth = 50;
        vbmpoffx = 100;
        vbmpdis = 2;
        ydis1 = 300;ydis2 = 30;ydis3 = 250;
        vpbh = 16;
    }
    
    bmpx = (lcddev.width - bmpw) / 2;
    bmpy = ydis1 + gui_phy.tbheight;
    sry = bmpy + bmpw + ydis2;
    
    vbmpx = vbmpoffx;
    vbmpy = sry + fsr + ydis3;
    
    vpbw = lcddev.width - vbmpoffx * 2 - 16 - vbmpdis;
    vpbx = vbmpx + 16 + vbmpdis;
    
    if (vpbh > 16)vpby = vbmpy - (vpbh - 16) / 2;
    else vpby = vbmpy + (16 - vpbh) / 2;
     
    vuheight = (vuwidth * (SPDIF_VU_LEVEL - 1)) / 8 + vuwidth * SPDIF_VU_LEVEL / 2; 
    vux = vuoffx;
    
    if (vuheight > bmpw) vuy = bmpy - (vuheight - bmpw) / 2;
    else vuy = bmpy + (bmpw - vuheight) / 2;
     
    vulevel[0] = 0;
    vulevel[1] = 0;
    volprgb = progressbar_creat(vpbx,vpby,vpbw,vpbh,0X20);  /* ������С������ */
    spdif_audiobuf[0] = gui_memin_malloc(SPDIF_DBUF_SIZE * 4);
    spdif_audiobuf[1] = gui_memin_malloc(SPDIF_DBUF_SIZE * 4);
    
    if (!spdif_audiobuf[1] || !volprgb)
    {
        if (volprgb) progressbar_delete(volprgb);   /* ɾ�������� */
        gui_memin_free(spdif_audiobuf[0]);          /* �ͷ��ڴ� */
        return 1;                                   /* ����ʧ��! */
    }
    
    memset((uint8_t*)spdif_audiobuf[0],0,SPDIF_DBUF_SIZE * 4);
    memset((uint8_t*)spdif_audiobuf[1],0,SPDIF_DBUF_SIZE * 4);
    
    if (g_audiodev.status & (1 << 7))               /* ��ǰ�ڷŸ�?? */
    {
        audio_stop_req(&g_audiodev);                /* ֹͣ��Ƶ���� */
        audio_task_delete();                        /* ɾ�����ֲ�������. */
    }
    
    lcd_clear(SPDIF_BACK_COLOR);
    app_gui_tcbar(0,0,lcddev.width,gui_phy.tbheight,0x02);                              /* �·ֽ��� */
    gui_show_strmid(0,0,lcddev.width,gui_phy.tbheight,WHITE,gui_phy.tbfsize,caption);   /* ��ʾ���� */
    
    SCB_CleanInvalidateDCache();                                                        /* �����MCU��,�ر�D cache */
    minibmp_decode((uint8_t*)SPDIF_RECORD_PIC[bmpidx],bmpx,bmpy,bmpw,bmpw,0,0);         /* ��������ͼ�� */
    SCB_CleanInvalidateDCache();                                                        /* �����MCU��,ʹ��D cache */
    minibmp_decode((uint8_t*)APP_VOL_PIC,vbmpx,vbmpy,16,16,0,0);                        /* ��������ͼ�� */
    
    volprgb->totallen = 63;
    
    if (es8388set.mvol <= 63) volprgb->curpos = es8388set.mvol;
    else    /* ���������  */
    {
        es8388set.mvol = 0;
        volprgb->curpos = 0;
    }
    
    lastvolpos = volprgb->curpos;           /* �趨�����λ�� */
    volprgb->inbkcolora = AUDIO_INFO_COLOR; /* Ĭ��ɫ */
    volprgb->inbkcolorb = AUDIO_INFO_COLOR; /* Ĭ��ɫ */
    volprgb->infcolora = 0X75D;             /* Ĭ��ɫ */
    volprgb->infcolorb = 0X596;             /* Ĭ��ɫ */
    progressbar_draw_progressbar(volprgb);  /* �������� */
    spdif_show_samplerate(sry,0,fsr);       /* ��ʾdetecting... */
    app_es8388_volset(es8388set.mvol);      /* ���������� */
    spdif_rx_init();                        /* SPDIF��ʼ�� */
    es8388_adda_cfg(1, 0);                  /* ����DAC�ر�ADC */
    es8388_output_cfg(1, 1);                /* DACѡ��ͨ����� */
    spdif_rx_dma_init((uint32_t*)spdif_audiobuf[0],(uint32_t*)spdif_audiobuf[1],SPDIF_DBUF_SIZE,2); /* ����SPDIF RX DMA,32λ */
    spdif_rx_stop_callback = spdif_rx_stopplay_callback;                                            /* SPDIF ��������ʱ�Ļص�����  */
    
    while(1)
    { 
        tp_dev.scan(0);    
        in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);      /* �õ�������ֵ */
        
        if (system_task_return) break;              /* TPAD���� */
        
        res = progressbar_check(volprgb,&in_obj);   /* ������������� */
        
        if (res && lastvolpos != volprgb->curpos)   /* ��������,��λ�ñ仯��.ִ���������� */
        {
            lastvolpos = volprgb->curpos;
            
            if (volprgb->curpos) es8388set.mvol = volprgb->curpos;/* �������� */
            else es8388set.mvol = 0;
            
            app_es8388_volset(es8388set.mvol);	    
        }
        
        if(spdif_dev.consta == 0)                           /* δ���� */
        {
            if (spdif_rx_wait_sync())                       /* �ȴ�ͬ�� */
            {
                spdif_dev.samplerate = spdif_rx_get_samplerate();   /* ��ò����� */
                
                if(spdif_dev.samplerate)                            /* ��������Ч,����WM8978��SAI�� */
                {
                    es8388_sai_cfg(0, 0);                           /* �����ֱ�׼,24λ���ݳ��� */
                    sai1_saia_init(0, 1, 6);                        /* ����SAI,������,24λ���� */
                    sai1_samplerate_set(spdif_dev.samplerate);      /* ���ò����� */
                    spdif_rx_start();                               /* ��SPDIF */
                    sai1_tx_dma_init((uint8_t*)spdif_audiobuf[0],(uint8_t*)spdif_audiobuf[1],SPDIF_DBUF_SIZE,2); /* ����TX DMA,32λ */
                    sai1_tx_callback = sai_dma_tx_callback;
                    sai1_play_start();                              /* ����DMA */
                }
                else spdif_rx_stop();                               /* �����ʴ���,ֹͣSPDIF����  */
            }
        }
        
        if (lastsamplerate != spdif_dev.samplerate)                 /* �����ʸı��� */
        {
            lastsamplerate = spdif_dev.samplerate;
            spdif_vu_meter(vuwidth,vux,vuy,0);                          /* ��ʾvu meter; */
            spdif_vu_meter(vuwidth,lcddev.width - vux - vuwidth,vuy,0); /* ��ʾvu meter; */
            spdif_show_samplerate(sry,spdif_dev.samplerate,fsr);        /* ��ʾ������. */
        }
        
        delay_ms(1000 / OS_TICKS_PER_SEC);                              /* ��ʱһ��ʱ�ӽ��� */
        timecnt++; 
        
        if ((timecnt % 20) == 0 && spdif_dev.samplerate)
        {
            for (i = 0;i < 512;i++)                     /* ȡǰ512��������������ֵ */
            {
                tempval = 0.0039061*spdif_vubuf[i*2];   /* ת��Ϊshort����  */
                
                if (tempval < 0)tempval =- tempval;
                
                if (maxval<tempval)maxval = tempval;    /* ȡ���ֵ */
            }
            
            tempval = spdif_vu_get(maxval);
            
            if (tempval > vulevel[0]) vulevel[0] = tempval;
            else if (vulevel[0]) vulevel[0]--;
            
            spdif_vu_meter(vuwidth,vux,vuy,vulevel[0]); /* ��ʾvu meter; */
            maxval = 0;

            for (i = 0;i < 512;i++)                             /* ȡǰ512��������������ֵ */
            
            {
                tempval = 0.0039061 * spdif_vubuf[i * 2 + 1];   /* ת��Ϊshort����  */
                if (tempval < 0) tempval =- tempval;
                if (maxval < tempval) maxval = tempval;         /* ȡ���ֵ */
            }
            
            tempval = spdif_vu_get(maxval);
            
            if (tempval > vulevel[1]) vulevel[1] = tempval;
            else if (vulevel[1]) vulevel[1]--;
            
            spdif_vu_meter(vuwidth,lcddev.width - vux - vuwidth,vuy,vulevel[1]);    /* ��ʾvu meter; */
            maxval = 0;
        } 
    }
    
    spdif_rx_stop();                    /* ֹͣSPDIF���� */
    sai1_play_stop();                   /* �ر���Ƶ */
    spdif_rx_mode(SPDIF_RX_IDLE);       /* SPDIFRX IDLE״̬ */
    es8388_adda_cfg(0, 0);              /* �ر�DAC&ADC */
    es8388_output_cfg(0, 0);            /* �ر�DAC��� */
    app_es8388_volset(0);               /* �ر�ES8388������� */
    gui_memin_free(spdif_audiobuf[0]);  /* �ͷ��ڴ� */
    gui_memin_free(spdif_audiobuf[1]);
    progressbar_delete(volprgb);        /* ɾ�������� */
    return 0;
}
