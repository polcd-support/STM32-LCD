#include "appplay_spdifrx.h"  
#include "audioplay.h"
#include "./BSP/SAI/sai.h"


uint32_t *spdif_audiobuf[2];                    /* SPDIF音频数据接收缓冲区,共2个(双缓冲) */
uint32_t *spdif_vubuf = 0;

#define SPDIF_BACK_COLOR        0X2945          /* 主界面背景色 */

#define SPDIF_VU_BKCOLOR        0X39C7          /* VU Meter背景色 */
#define SPDIF_VU_L1COLOR        0X07FF          /* VU Meter L1色 */
#define SPDIF_VU_L2COLOR        0xFFE0          /* VU Meter L2色 */
#define SPDIF_VU_L3COLOR        0xF800          /* VU Meter L3色 */
#define SPDIF_VU_LEVEL          12              /* 总级数,必须是偶数 */

uint8_t* const SPDIF_RECORD_PIC[3]=
{ 
    "1:/SYSTEM/APP/APPS/SPDIF/record_180.bmp",  /* demo图片路径 */
    "1:/SYSTEM/APP/APPS/SPDIF/record_220.bmp",  /* demo图片路径 */
    "1:/SYSTEM/APP/APPS/SPDIF/record_320.bmp",  /* demo图片路径 */
};
 
/**
 * @brief       显示采样率
 * @param       y:y坐标(x坐标自动计算)
 * @param       samplerate:音频采样率(单位:Hz)
 * @param       fsr:采样率显示的字体
 * @retval      无
 */
void spdif_show_samplerate(uint16_t y,uint32_t samplerate,uint8_t fsr)
{  
    uint8_t *buf;
    uint16_t x;
    uint16_t lenth;
    float rate = (float)samplerate / 1000; 
    buf = mymalloc(SRAMIN,100);                                         /* 申请内存 */
    
    if (buf)                                                            /* 申请成功 */
    {
        if (samplerate)
        {
            if (samplerate % 1000) sprintf((char*)buf,"%2.1fKHz",rate); /* 打印采样率 */
            else sprintf((char*)buf,"%dKHz",samplerate / 1000);         /* 打印采样率 */
        }
        else sprintf((char*)buf,"Detecting...");                        /* 显示检查中 */
        
        lenth = strlen((char*)buf);
        lenth = fsr * lenth / 2;
        
        x = (lcddev.width - lenth) / 2; 
        gui_fill_rectangle((lcddev.width - fsr * 6) / 2,y,fsr * 6,fsr,SPDIF_BACK_COLOR); 
        gui_show_string(buf,x,y,200,fsr,fsr,WHITE);                     /* 显示采样率 */
    }
    
    myfree(SRAMIN,buf);                                                 /* 释放内存 */
}

/**
 * @brief       SAI DMA发送完成中断回调函数
 * @param       无
 * @retval      无
 */
void sai_dma_tx_callback(void)
{ 
    if (DMA2_Stream3->CR & (1 << 19))       /* buf1已空,正常处理buf0 */
    { 
        spdif_vubuf = spdif_audiobuf[0];    /* 处理buf0 */
        
    }
    else                                    /* buf0已空,正常处理buf1 */
    {
        spdif_vubuf = spdif_audiobuf[1];    /* 处理buf1 */
    }
}

/**
 * @brief       SPDIF RX结束时的回调函数
 * @param       无
 * @retval      无
 */
void spdif_rx_stopplay_callback(void)
{
    sai1_play_stop();
    SPDIFRX->IFCR |= 1 << 5;                /* 清除同步完成标志 */
    spdif_dev.samplerate = 0;
    memset((uint8_t*)spdif_audiobuf[0],0,SPDIF_DBUF_SIZE * 4);   
    memset((uint8_t*)spdif_audiobuf[1],0,SPDIF_DBUF_SIZE * 4);
}

/* 电平阀值表 */
const uint16_t spdif_vu_val_tbl[SPDIF_VU_LEVEL] = {600,1200,2400,3600,4800,6000,8000,11000,13000,16000,21000,28000};

/**
 * @brief       从信号电平得到vu表征值
 * @param       signallevel:信号电平
 * @retval      返回值:vu值
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
 * @brief       显示VU Meter
 * @param       width:宽度,高度=宽度/2,间隔=宽度/8
 * @param       x,y:坐标
 * @param       level:0~10
 * @retval      无
 */
void spdif_vu_meter(uint8_t width,uint16_t x,uint16_t y,uint8_t level)
{
    uint8_t i; 
    uint16_t pitch = 0; /* 2个level之间的间隙 */
    uint16_t height = 0;
    uint16_t vucolor = SPDIF_VU_L1COLOR;
    
    if (level > SPDIF_VU_LEVEL) return ;
    
    pitch = width / 8;
    height = pitch * (SPDIF_VU_LEVEL - 1) + width * SPDIF_VU_LEVEL / 2;
    
    if (level == 0)
    {
        gui_fill_rectangle(x,y,width,height,SPDIF_VU_BKCOLOR);  /* 填充背景色 */
        return;
    }
    
    for (i = 0;i < level;i++)
    {
        if (i == (SPDIF_VU_LEVEL - 1)) vucolor = SPDIF_VU_L3COLOR;
        else if (i > SPDIF_VU_LEVEL / 2) vucolor = SPDIF_VU_L2COLOR;
        
        gui_fill_rectangle(x,y + height - width / 2 - (width / 2 + pitch) * i,width,width / 2,vucolor); /* 填充背景色 */
    }
    
    if (level < 10)gui_fill_rectangle(x,y,width,height - level * (width / 2 + pitch),SPDIF_VU_BKCOLOR); /* 填充背景色 */
} 

/**
 * @brief       SPDIF RX应用（通过SPDIF RX接口,接收光纤音频数据,并解码出来）
 * @param       无
 * @retval      无
 */
uint8_t appplay_spdifrx(uint8_t* caption)
{  
    uint8_t res;
    uint16_t lastvolpos; 
    uint32_t lastsamplerate = 0;
    
    uint16_t i;
    uint8_t timecnt = 0;
    uint8_t vulevel[2];                 /* 两个声道 */
    short tempval;
    uint16_t maxval = 0;
    
    
    uint8_t fsr;                        /* 采样率字体,12/16/24 */
    uint16_t sry;                       /* 采样率的y坐标 */
    
    uint16_t bmpx,bmpy,bmpw;            /* 唱片图片的x,y坐标和高度 */
    uint8_t bmpidx;                     /* 唱片图片索引 */
    
    uint16_t vpbh,vpbw,vpbx,vpby;       /* 音量进度条的高度/宽度/x,y坐标 */
    uint16_t vbmpx,vbmpy;               /* 音量图标的x,y坐标 */
    
    uint16_t vux,vuy,vuwidth,vuheight;  /* vu x,y坐标,vu条的宽度/高度 */
    
    uint16_t ydis1,ydis2,ydis3;         /* 3个纵向间隔 */
    uint16_t vuoffx;                    /* vu x方向的偏移和与唱片图标的间隙 */
    uint16_t vbmpoffx,vbmpdis;          /* 音量图标的x偏移和与音量进度条的间隙 */

    _progressbar_obj*volprgb;           /* 音量进度条 */
    
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
    volprgb = progressbar_creat(vpbx,vpby,vpbw,vpbh,0X20);  /* 声音大小进度条 */
    spdif_audiobuf[0] = gui_memin_malloc(SPDIF_DBUF_SIZE * 4);
    spdif_audiobuf[1] = gui_memin_malloc(SPDIF_DBUF_SIZE * 4);
    
    if (!spdif_audiobuf[1] || !volprgb)
    {
        if (volprgb) progressbar_delete(volprgb);   /* 删除进度条 */
        gui_memin_free(spdif_audiobuf[0]);          /* 释放内存 */
        return 1;                                   /* 返回失败! */
    }
    
    memset((uint8_t*)spdif_audiobuf[0],0,SPDIF_DBUF_SIZE * 4);
    memset((uint8_t*)spdif_audiobuf[1],0,SPDIF_DBUF_SIZE * 4);
    
    if (g_audiodev.status & (1 << 7))               /* 当前在放歌?? */
    {
        audio_stop_req(&g_audiodev);                /* 停止音频播放 */
        audio_task_delete();                        /* 删除音乐播放任务. */
    }
    
    lcd_clear(SPDIF_BACK_COLOR);
    app_gui_tcbar(0,0,lcddev.width,gui_phy.tbheight,0x02);                              /* 下分界线 */
    gui_show_strmid(0,0,lcddev.width,gui_phy.tbheight,WHITE,gui_phy.tbfsize,caption);   /* 显示标题 */
    
    SCB_CleanInvalidateDCache();                                                        /* 如果是MCU屏,关闭D cache */
    minibmp_decode((uint8_t*)SPDIF_RECORD_PIC[bmpidx],bmpx,bmpy,bmpw,bmpw,0,0);         /* 解码音量图标 */
    SCB_CleanInvalidateDCache();                                                        /* 如果是MCU屏,使能D cache */
    minibmp_decode((uint8_t*)APP_VOL_PIC,vbmpx,vbmpy,16,16,0,0);                        /* 解码音量图标 */
    
    volprgb->totallen = 63;
    
    if (es8388set.mvol <= 63) volprgb->curpos = es8388set.mvol;
    else    /* 错误的数据  */
    {
        es8388set.mvol = 0;
        volprgb->curpos = 0;
    }
    
    lastvolpos = volprgb->curpos;           /* 设定最近的位置 */
    volprgb->inbkcolora = AUDIO_INFO_COLOR; /* 默认色 */
    volprgb->inbkcolorb = AUDIO_INFO_COLOR; /* 默认色 */
    volprgb->infcolora = 0X75D;             /* 默认色 */
    volprgb->infcolorb = 0X596;             /* 默认色 */
    progressbar_draw_progressbar(volprgb);  /* 画进度条 */
    spdif_show_samplerate(sry,0,fsr);       /* 显示detecting... */
    app_es8388_volset(es8388set.mvol);      /* 设置主音量 */
    spdif_rx_init();                        /* SPDIF初始化 */
    es8388_adda_cfg(1, 0);                  /* 开启DAC关闭ADC */
    es8388_output_cfg(1, 1);                /* DAC选择通道输出 */
    spdif_rx_dma_init((uint32_t*)spdif_audiobuf[0],(uint32_t*)spdif_audiobuf[1],SPDIF_DBUF_SIZE,2); /* 配置SPDIF RX DMA,32位 */
    spdif_rx_stop_callback = spdif_rx_stopplay_callback;                                            /* SPDIF 结束播放时的回调函数  */
    
    while(1)
    { 
        tp_dev.scan(0);    
        in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);      /* 得到按键键值 */
        
        if (system_task_return) break;              /* TPAD返回 */
        
        res = progressbar_check(volprgb,&in_obj);   /* 检查音量进度条 */
        
        if (res && lastvolpos != volprgb->curpos)   /* 被按下了,且位置变化了.执行音量调整 */
        {
            lastvolpos = volprgb->curpos;
            
            if (volprgb->curpos) es8388set.mvol = volprgb->curpos;/* 设置音量 */
            else es8388set.mvol = 0;
            
            app_es8388_volset(es8388set.mvol);	    
        }
        
        if(spdif_dev.consta == 0)                           /* 未连接 */
        {
            if (spdif_rx_wait_sync())                       /* 等待同步 */
            {
                spdif_dev.samplerate = spdif_rx_get_samplerate();   /* 获得采样率 */
                
                if(spdif_dev.samplerate)                            /* 采样率有效,配置WM8978和SAI等 */
                {
                    es8388_sai_cfg(0, 0);                           /* 飞利浦标准,24位数据长度 */
                    sai1_saia_init(0, 1, 6);                        /* 设置SAI,主发送,24位数据 */
                    sai1_samplerate_set(spdif_dev.samplerate);      /* 设置采样率 */
                    spdif_rx_start();                               /* 打开SPDIF */
                    sai1_tx_dma_init((uint8_t*)spdif_audiobuf[0],(uint8_t*)spdif_audiobuf[1],SPDIF_DBUF_SIZE,2); /* 配置TX DMA,32位 */
                    sai1_tx_callback = sai_dma_tx_callback;
                    sai1_play_start();                              /* 开启DMA */
                }
                else spdif_rx_stop();                               /* 采样率错误,停止SPDIF播放  */
            }
        }
        
        if (lastsamplerate != spdif_dev.samplerate)                 /* 采样率改变了 */
        {
            lastsamplerate = spdif_dev.samplerate;
            spdif_vu_meter(vuwidth,vux,vuy,0);                          /* 显示vu meter; */
            spdif_vu_meter(vuwidth,lcddev.width - vux - vuwidth,vuy,0); /* 显示vu meter; */
            spdif_show_samplerate(sry,spdif_dev.samplerate,fsr);        /* 显示采样率. */
        }
        
        delay_ms(1000 / OS_TICKS_PER_SEC);                              /* 延时一个时钟节拍 */
        timecnt++; 
        
        if ((timecnt % 20) == 0 && spdif_dev.samplerate)
        {
            for (i = 0;i < 512;i++)                     /* 取前512个数据里面的最大值 */
            {
                tempval = 0.0039061*spdif_vubuf[i*2];   /* 转换为short类型  */
                
                if (tempval < 0)tempval =- tempval;
                
                if (maxval<tempval)maxval = tempval;    /* 取最大值 */
            }
            
            tempval = spdif_vu_get(maxval);
            
            if (tempval > vulevel[0]) vulevel[0] = tempval;
            else if (vulevel[0]) vulevel[0]--;
            
            spdif_vu_meter(vuwidth,vux,vuy,vulevel[0]); /* 显示vu meter; */
            maxval = 0;

            for (i = 0;i < 512;i++)                             /* 取前512个数据里面的最大值 */
            
            {
                tempval = 0.0039061 * spdif_vubuf[i * 2 + 1];   /* 转换为short类型  */
                if (tempval < 0) tempval =- tempval;
                if (maxval < tempval) maxval = tempval;         /* 取最大值 */
            }
            
            tempval = spdif_vu_get(maxval);
            
            if (tempval > vulevel[1]) vulevel[1] = tempval;
            else if (vulevel[1]) vulevel[1]--;
            
            spdif_vu_meter(vuwidth,lcddev.width - vux - vuwidth,vuy,vulevel[1]);    /* 显示vu meter; */
            maxval = 0;
        } 
    }
    
    spdif_rx_stop();                    /* 停止SPDIF播放 */
    sai1_play_stop();                   /* 关闭音频 */
    spdif_rx_mode(SPDIF_RX_IDLE);       /* SPDIFRX IDLE状态 */
    es8388_adda_cfg(0, 0);              /* 关闭DAC&ADC */
    es8388_output_cfg(0, 0);            /* 关闭DAC输出 */
    app_es8388_volset(0);               /* 关闭ES8388音量输出 */
    gui_memin_free(spdif_audiobuf[0]);  /* 释放内存 */
    gui_memin_free(spdif_audiobuf[1]);
    progressbar_delete(volprgb);        /* 删除进度条 */
    return 0;
}
