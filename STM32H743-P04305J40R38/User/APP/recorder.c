/**
 ****************************************************************************************************
 * @file        camera.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-1
 * @brief       APP-录音机 代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20221201
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "recorder.h"
#include "calendar.h"
#include "audioplay.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/SAI/sai.h"
#include "settings.h"


uint8_t *sairecbuf1;     /* I2S 接收缓冲指针1 */
uint8_t *sairecbuf2;     /* I2S 接收缓冲指针2 */

/**
 * REC录音FIFO管理参数.
 * 由于FATFS文件写入时间的不确定性,如果直接在接收中断里面写文件,可能导致某次写入时间过长
 * 从而引起数据丢失,故加入FIFO控制,以解决此问题
 */
volatile uint8_t g_sai_recfifo_rdpos = 0;           /* 录音FIFO 读位置 */
volatile uint8_t g_sai_recfifo_wrpos = 0;           /* 录音FIFO 写位置 */
uint8_t * p_sai_recfifo_buf[SAI_RX_FIFO_SIZE];  /* 录音接收FIFO数组指针 */


uint32_t g_wav_size;        /* wav数据大小(字节数,不包括文件头!!) */

uint8_t g_rec_sta = 0;      /**
                             * 录音状态
                             * [7]:0,没有开启录音;1,已经开启录音;
                             * [6:1]:保留
                             * [0]:0,正在录音;1,暂停录音;
                             */
                             
uint8_t *g_vubuf;           /* vu用buf,直接指向sairecbuf1/sairecbuf2 */


#define RECORDER_TITLE_COLOR    0XFFFF      /* 录音机标题颜色 */
#define RECORDER_TITLE_BKCOLOR  0X0000      /* 录音机标题背景色 */

#define RECORDER_VU_BKCOLOR     0X39C7      /* VU Meter背景色 */
#define RECORDER_VU_L1COLOR     0X07FF      /* VU Meter L1色 */
#define RECORDER_VU_L2COLOR     0xFFE0      /* VU Meter L2色 */
#define RECORDER_VU_L3COLOR     0xF800      /* VU Meter L3色 */

#define RECORDER_TIME_COLOR     0X07FF      /* 时间颜色 */
#define RECORDER_MAIN_BKCOLOR   0X18E3      /* 主背景色 */

/* 窗体内嵌字颜色 */
#define RECORDER_INWIN_FONT_COLOR   0X736C  /* 0XAD53 */


uint8_t *const RECORDER_DEMO_PIC = "1:/SYSTEM/APP/RECORDER/Demo.bmp";       /* demo图片路径 */
uint8_t *const RECORDER_RECR_PIC = "1:/SYSTEM/APP/RECORDER/RecR.bmp";       /* 录音 松开 */
uint8_t *const RECORDER_RECP_PIC = "1:/SYSTEM/APP/RECORDER/RecP.bmp";       /* 录音 按下 */
uint8_t *const RECORDER_PAUSER_PIC = "1:/SYSTEM/APP/RECORDER/PauseR.bmp";   /* 暂停 松开 */
uint8_t *const RECORDER_PAUSEP_PIC = "1:/SYSTEM/APP/RECORDER/PauseP.bmp";   /* 暂停 按下 */
uint8_t *const RECORDER_STOPR_PIC = "1:/SYSTEM/APP/RECORDER/StopR.bmp";     /* 停止 松开 */
uint8_t *const RECORDER_STOPP_PIC = "1:/SYSTEM/APP/RECORDER/StopP.bmp";     /* 停止 按下 */

/* 录音设置选择列表 */
uint8_t *const recorder_setsel_tbl[GUI_LANGUAGE_NUM][2] =
{
    {"采样率设置", "MIC增益设置",},
    {"採樣率設置", "MIC增益設置",},
    {"Samplerate set", "MIC gain set",},
};

/* 录音采样率提示信息表 */
uint8_t *const recorder_sampleratemsg_tbl[5] = {"8KHz", "16Khz", "32Khz", "44.1Khz", "48Khz",};

/* 录音采样率表 */
const uint16_t recorder_samplerate_tbl[5] = {8000, 16000, 32000, 44100, 48000};

/* 录音提示信息 */
uint8_t *const recorder_remind_tbl[3][GUI_LANGUAGE_NUM] =
{
    "是否保存该录音文件?", "是否保存該錄音文件?", "Do you want to save?",
    {"请先停止录音!", "請先停止錄音!", "Please stop REC first!",},
    {"内存不够!!", "內存不夠!!", "Out of memory!",},
};

/* 00级功能选项表标题 */
uint8_t *const recorder_modesel_tbl[GUI_LANGUAGE_NUM] =
{
    "录音设置", "錄音設置", "Recorder Set",
};


/**
 * @brief       读取一个录音FIFO
 * @param       buf:  数据缓存区首地址
 * @retval      0, 没有数据可读;
 *              1, 读到了1个数据块;
 */
uint8_t rec_sai_fifo_read(uint8_t **buf)
{
    if (g_sai_recfifo_rdpos == g_sai_recfifo_wrpos)     /* 读位置  =  写位置, 说明没得数据可读 */
    {
        return 0;
    }
    
    g_sai_recfifo_rdpos++;          /* 读位置加1 */

    if (g_sai_recfifo_rdpos >= SAI_RX_FIFO_SIZE)    /* 读位置超过了总FIFO数, 归零重新开始 */
    {
        g_sai_recfifo_rdpos = 0;    /* 归零 */
    }
    
    *buf = p_sai_recfifo_buf[g_sai_recfifo_rdpos];      /* 返回对应FIFO BUF的地址 */

    return 1;
}

/**
 * @brief       写一个录音FIFO
 * @param       buf:  数据缓存区首地址
 * @retval      0, 写入成功;
 *              1, 写入失败;
 */
uint8_t rec_sai_fifo_write(uint8_t *buf)
{
    uint16_t i;
    uint8_t temp = g_sai_recfifo_wrpos; /* 记录当前写位置 */
    g_sai_recfifo_wrpos++;              /* 写位置加1 */

    if (g_sai_recfifo_wrpos >= SAI_RX_FIFO_SIZE)    /* 写位置超过了总FIFO数, 归零重新开始 */
    {
        g_sai_recfifo_wrpos = 0;        /* 归零 */
    }
    
    if (g_sai_recfifo_wrpos == g_sai_recfifo_rdpos)     /* 写位置  =  读位置, 说明没得位置可写了 */
    {
        g_sai_recfifo_wrpos = temp;     /* 还原原来的写位置,此次写入失败 */
        return 1;
    }

    for (i = 0; i < SAI_RX_DMA_BUF_SIZE; i++)       /* 循环写数据 */
    {
        p_sai_recfifo_buf[g_sai_recfifo_wrpos][i] = buf[i];
    }

    return 0;
}

/**
 * @brief       录音 SAI RX DMA接收中断服务函数
 *   @note      在中断里面写入数据
 * @param       无
 * @retval      无
 */
void rec_sai_dma_rx_callback(void)
{
    if (DMA2_Stream5->CR & (1 << 19))
    {
        g_vubuf = sairecbuf1;    /* g_vubuf指向sairecbuf1 */

        if (g_rec_sta == 0X80)      /* 录音模式,非暂停,则写文件 */
        {
            rec_sai_fifo_write(sairecbuf1);  /* sairecbuf1 写入FIFO */
        }
    }
    else
    {
        g_vubuf = sairecbuf2;    /* g_vubuf指向sairecbuf2 */

        if (g_rec_sta == 0X80)      /* 录音模式,非暂停,则写文件 */
        {
            rec_sai_fifo_write(sairecbuf2);  /* sairecbuf1 写入FIFO */
        }
    }

}
const uint16_t saiplaybuf[2] = {0X0000, 0X0000}; /* 2个16位数据,用于录音时I2S Master发送.循环发送0 */

/**
 * @brief       进入PCM录音模式
 * @param       无
 * @retval      无
 */
void recorder_enter_rec_mode(void)
{
    es8388_adda_cfg(0, 1);          /* 开启ADC */
    es8388_input_cfg(0);            /* 开启输入通道(通道1,MIC所在通道) */
    es8388_mic_gain(8);             /* MIC增益设置为最大 */
    es8388_alc_ctrl(3, 4, 4);       /* 开启立体声ALC控制,以提高录音音量 */
    es8388_output_cfg(0, 0);        /* 关闭通道1和2的输出 */
    es8388_spkvol_set(0);           /* 关闭喇叭. */
    es8388_sai_cfg(0, 3);           /* 飞利浦标准,16位数据长度 */

    sai1_saia_init(0, 1, 4);        /* SAI1 Block A,主发送,16位数据 */
    sai1_saib_init(3, 1, 4);        /* SAI1 Block B从模式接收,16位 */
    
    sai1_samplerate_set(es8388set.mvol);    /* 设置采样率 */
    
    sai1_tx_dma_init((uint8_t *)&saiplaybuf[0], (uint8_t *)&saiplaybuf[1],1,1); /* 配置TX DMA,16位 */
    sai1_rx_dma_init(sairecbuf1, sairecbuf2, SAI_RX_DMA_BUF_SIZE / 2,1);        /* 配置RX DMA */
    
    sai1_rx_callback = rec_sai_dma_rx_callback; /* 初始化回调函数指sai_rx_callback */
    
    SAI1_TX_DMASx->CR &= ~(1 << 4); /* 关闭 TX DMA传输完成中断(这里不用中断送数据) */
    
    sai1_play_start();              /* 开始SAI数据发送(主机) */
    sai1_rec_start();               /* 开始SAI数据接收(从机) */
}

/**
 * @brief       停止录音模式
 * @param       无
 * @retval      无
 */
void recorder_stop_rec_mode(void)
{
    sai1_play_stop();           /* 停止SAI */
    sai1_rec_stop();
    
    es8388_adda_cfg(0, 0);      /* 关闭DAC&ADC */
    es8388_output_cfg(0, 0);    /* 关闭通道1和2的输出 */
    app_es8388_volset(0);       /* 关闭ES8388音量输出 */
}

/**
 * @brief       设置录音采样频率
 * @param       wavhead         : wav文件头结构体
 * @param       samplerate      : 8K~192K.
 * @retval      无
 */
void recoder_set_samplerate(__WaveHeader *wavhead, uint16_t samplerate)
{
    sai1_play_stop();
    sai1_samplerate_set(samplerate);
    wavhead->fmt.SampleRate = samplerate;   /* 采样率,单位:Hz */
    wavhead->fmt.ByteRate = wavhead->fmt.SampleRate * 4; /* 字节速率=采样率*通道数*(ADC位数/8) */
    sai1_play_start();       /* 开始I2S数据发送(主机) */
    sai1_play_start();        /* 开始I2S数据接收(从机) */
}

/**
 * @brief       初始化WAV头
 * @param       wavhead         : wav文件头结构体
 * @retval      无
 */
void recorder_wav_init(__WaveHeader *wavhead)
{
    wavhead->riff.ChunkID = 0X46464952; /* "RIFF" */
    wavhead->riff.ChunkSize = 0;        /* 还未确定,最后需要计算 */
    wavhead->riff.Format = 0X45564157;  /* "WAVE" */
    wavhead->fmt.ChunkID = 0X20746D66;  /* "fmt " */
    wavhead->fmt.ChunkSize = 16;        /* 大小为16个字节 */
    wavhead->fmt.AudioFormat = 0X01;    /* 0X01,表示PCM;0X01,表示IMA ADPCM */
    wavhead->fmt.NumOfChannels = 2;     /* 双声道 */
    wavhead->fmt.SampleRate = 0;        /* 采样率,单位:Hz,后续再确定 */
    wavhead->fmt.ByteRate = 0;          /* 字节速率=采样率*通道数*(ADC位数/8),后续确定 */
    wavhead->fmt.BlockAlign = 4;        /* 块大小=通道数*(ADC位数/8) */
    wavhead->fmt.BitsPerSample = 16;    /* 16位PCM */
    wavhead->data.ChunkID = 0X61746164; /* "data" */
    wavhead->data.ChunkSize = 0;        /* 数据大小,还需要计算 */
}

/* 电平阀值表 */
const uint16_t vu_val_tbl[10] = {1200, 2400, 3600, 4800, 6000, 8000, 11000, 16000, 21000, 28000};

/**
 * @brief       从信号电平得到vu表征值
 * @param       signallevel     : 信号电平
 * @retval      vu值
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
 * @brief       显示VU Meter
 * @param       x,y             : 坐标
 * @param       level           : 0~10;
 * @retval      无
 */
void recorder_vu_meter(uint16_t x, uint16_t y, uint8_t level)
{
    uint8_t i;
    uint16_t vucolor = RECORDER_VU_L1COLOR;

    if (level > 10)return ;

    if (level == 0)
    {
        gui_fill_rectangle(x, y, 218, 10, RECORDER_VU_BKCOLOR); /* 填充背景色 */
        return;
    }

    for (i = 0; i < level; i++)
    {
        if (i == 9)vucolor = RECORDER_VU_L3COLOR;
        else if (i > 5)vucolor = RECORDER_VU_L2COLOR;

        gui_fill_rectangle(x + 22 * i, y, 20, 10, vucolor);     /* 填充背景色 */
    }

    if (level < 10)gui_fill_rectangle(x + level * 22, y, 218 - level * 22, 10, RECORDER_VU_BKCOLOR);    /* 填充背景色 */
}

/**
 * @brief       显示录音时长(显示尺寸为:150*60)
 * @param       x,y             : 坐标
 * @param       tsec            : 秒钟数
 * @retval      无
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
 * @brief       显示名字
 * @param       x,y             : 坐标
 * @param       name            : 名字
 * @retval      无
 */
void recorder_show_name(uint16_t x, uint16_t y, uint8_t *name)
{
    gui_fill_rectangle(x - 1, y - 1, lcddev.width, 13, RECORDER_MAIN_BKCOLOR);  /* 填充背景色 */
    gui_show_ptstrwhiterim(x, y, lcddev.width, y + 12, 0, BLACK, WHITE, 12, name);
}

/**
 * @brief       显示采样率
 * @param       x,y             : 坐标(不要从0开始)
 * @param       samplerate      : 采样率
 * @retval      无
 */
void recorder_show_samplerate(uint16_t x, uint16_t y, uint16_t samplerate)
{
    uint8_t *buf = 0;
    float temp;
    temp = (float)samplerate / 1000;
    buf = gui_memin_malloc(60); /* 申请内存 */

    if (buf == 0)return;

    if (samplerate % 1000)sprintf((char *)buf, "%.1fKHz", temp);    /* 有小数点 */
    else sprintf((char *)buf, "%dKHz", samplerate / 1000);

    gui_fill_rectangle(x, y, 42, 12, RECORDER_MAIN_BKCOLOR);        /* 填充背景色 */
    gui_show_string(buf, x, y, 42, 12, 12, RECORDER_INWIN_FONT_COLOR);  /* 显示agc */
    gui_memin_free(buf);        /* 释放内存 */
}

/**
 * @brief       加载录音机主界面UI
 * @param       无
 * @retval      无
 */
void recorder_load_ui(void)
{
    gui_fill_rectangle(0, 0, lcddev.width, gui_phy.tbheight, RECORDER_TITLE_BKCOLOR);           /* 填充背景色 */
    gui_show_strmid(0, 0, lcddev.width, gui_phy.tbheight, RECORDER_TITLE_COLOR, gui_phy.tbfsize, (uint8_t *)APP_MFUNS_CAPTION_TBL[11][gui_phy.language]); /* 显示标题 */
    gui_fill_rectangle(0, gui_phy.tbheight, lcddev.width, lcddev.height - gui_phy.tbheight, RECORDER_MAIN_BKCOLOR);             /* 填充底色 */
    minibmp_decode((uint8_t *)RECORDER_DEMO_PIC, (lcddev.width - 100) / 2, 100 + (lcddev.height - 320) / 2, 100, 100, 0, 0);    /* 解码100*100的图片DEMO */
    recorder_vu_meter((lcddev.width - 218) / 2, (lcddev.height - 320) / 2 + 200 + 5, 0);        /* 显示vu meter */
    app_gui_tcbar(0, lcddev.height - gui_phy.tbheight, lcddev.width, gui_phy.tbheight, 0x01);   /* 上分界线 */
}

/**
 * @brief       通过时间获取文件名
 *  @note       仅限在SD卡/U盘保存,不支持FLASH DISK保存
 *  @note       组合成:形如"0:PAINT/PAINT20120321210633.wav"/"2:PAINT/PAINT20120321210633.wav"的文件名
 * @param       pname           : 带路径的名字
 * @retval      无
 */
void recorder_new_pathname(uint8_t *pname)
{
    calendar_get_time(&calendar);
    calendar_get_date(&calendar);

    if (gui_phy.memdevflag & (1 << 0))sprintf((char *)pname, "0:RECORDER/REC%04d%02d%02d%02d%02d%02d.wav", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);        /* 首选保存在SD卡 */
    else if (gui_phy.memdevflag & (1 << 3))sprintf((char *)pname, "3:RECORDER/REC%04d%02d%02d%02d%02d%02d.wav", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);   /* SD卡不存在,则保存在U盘 */
}

/**
 * @brief       显示AGC大小
 * @param       x,y             : 坐标
 * @param       agc             : 增益值 0~8,对应0dB~24dB,3dB/Step
 * @retval      无
 */
void recorder_show_agc(uint16_t x, uint16_t y, uint8_t agc)
{
    uint8_t *buf;
    float temp;
    buf = gui_memin_malloc(60); /* 申请内存 */

    if (buf == 0)return;

    temp = agc * 3;
    sprintf((char *)buf, "%.2fdB", temp);
    gui_phy.back_color = APP_WIN_BACK_COLOR;                /* 设置背景色为底色 */
    gui_fill_rectangle(x, y, 48, 12, APP_WIN_BACK_COLOR);   /* 填充背景色 */
    gui_show_string(buf, x, y, 48, 12, 12, RECORDER_INWIN_FONT_COLOR);  /* 显示agc */
    gui_memin_free(buf);/* 释放内存 */
}

/**
 * @brief       agc设置界面.固定尺寸:180*122
 * @param       x,y             : 坐标
 * @param       agc             : 自动增益指针,范围:0~63,对应-12dB~35.25dB,0.75dB/Step
 * @param       caption         : 窗口名字
 * @retval      0,成功设置;
 *              其他,不进行设置
 */
uint8_t recorder_agc_set(uint16_t x, uint16_t y, uint8_t *agc, uint8_t *caption)
{
    uint8_t rval = 0, res;
    _window_obj *twin = 0;      /* 窗体 */
    _btn_obj *rbtn = 0;         /* 取消按钮 */
    _btn_obj *okbtn = 0;        /* 确定按钮 */
    _progressbar_obj *agcprgb;  /* AGC设置进度条 */
    uint8_t tempagc = *agc;

    twin = window_creat(x, y, 180, 122, 0, 1 | 1 << 5, 16);     /* 创建窗口 */
    agcprgb = progressbar_creat(x + 10, y + 52, 160, 15, 0X20); /* 创建进度条 */

    if (agcprgb == NULL)rval = 1;

    okbtn = btn_creat(x + 20, y + 82, 60, 30, 0, 0x02);         /* 创建按钮 */
    rbtn = btn_creat(x + 20 + 60 + 20, y + 82, 60, 30, 0, 0x02);/* 创建按钮 */

    if (twin == NULL || rbtn == NULL || okbtn == NULL || rval)rval = 1;
    else
    {
        /* 窗口的名字和背景色 */
        twin->caption = caption;
        twin->windowbkc = APP_WIN_BACK_COLOR;
        
        /* 返回按钮的颜色 */
        rbtn->bkctbl[0] = 0X8452;   /* 边框颜色 */
        rbtn->bkctbl[1] = 0XAD97;   /* 第一行的颜色 */
        rbtn->bkctbl[2] = 0XAD97;   /* 上半部分颜色 */
        rbtn->bkctbl[3] = 0X8452;   /* 下半部分颜色 */
        okbtn->bkctbl[0] = 0X8452;  /* 边框颜色 */
        okbtn->bkctbl[1] = 0XAD97;  /* 第一行的颜色 */
        okbtn->bkctbl[2] = 0XAD97;  /* 上半部分颜色 */
        okbtn->bkctbl[3] = 0X8452;  /* 下半部分颜色 */
        
        agcprgb->totallen = 8;      /* 最大AGC为8 */
        agcprgb->curpos = tempagc;  /* 当前尺寸 */
        rbtn->caption = (uint8_t *)GUI_CANCEL_CAPTION_TBL[gui_phy.language];    /* 标题为取消 */
        okbtn->caption = (uint8_t *)GUI_OK_CAPTION_TBL[gui_phy.language];       /* 标题为确定 */
        window_draw(twin);          /* 画出窗体 */
        btn_draw(rbtn);             /* 画按钮 */
        btn_draw(okbtn);            /* 画按钮 */
        progressbar_draw_progressbar(agcprgb);
        gui_show_string("AGC:", x + 10, y + 38, 24, 12, 12, RECORDER_INWIN_FONT_COLOR); /* 显示SIZE */
        recorder_show_agc(x + 10 + 24, y + 38, tempagc);

        while (rval == 0)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
            delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */

            if (system_task_return)
            {
                delay_ms(10);
                if (tpad_scan(1)) 
                {
                    break;  /* TPAD返回,再次确认,排除干扰 */
                }
                else system_task_return = 0;
            }

            res = btn_check(rbtn, &in_obj);         /* 取消按钮检测 */

            if (res && ((rbtn->sta & 0X80) == 0))   /* 有有效操作 */
            {
                rval = 1;
                break;/* 退出 */
            }

            res = btn_check(okbtn, &in_obj);        /* 确认按钮检测 */

            if (res && ((okbtn->sta & 0X80) == 0))  /* 有有效操作 */
            {
                rval = 0XFF;
                break;/* 确认了 */
            }

            res = progressbar_check(agcprgb, &in_obj);

            if (res && (tempagc != agcprgb->curpos))    /* 进度条改动了 */
            {
                tempagc = agcprgb->curpos;      /* 保存最新的结果 */
                recorder_show_agc(x + 10 + 24, y + 38, tempagc);
                es8388_mic_gain(tempagc);       /* 设置增益 */
            }
        }
    }

    window_delete(twin);        /* 删除窗口 */
    btn_delete(rbtn);           /* 删除按钮 */
    progressbar_delete(agcprgb);/* 删除进度条 */
    system_task_return = 0;

    if (rval == 0XFF)
    {
        *agc = tempagc;
        return 0;
    }

    return rval;
}

/**
 * @brief       录音机
 *  @note       所有录音文件,均保存在SD卡RECORDER文件夹内.
 * @param       无
 * @retval      无
 */
uint8_t recorder_play(void)
{
    uint8_t res;
    uint8_t rval = 0;
    __WaveHeader *wavhead = 0;
    FIL * f_rec;
    uint8_t *pdatabuf;
    uint8_t *pname = 0;
    _btn_obj *rbtn = 0;         /* 取消按钮 */
    _btn_obj *mbtn = 0;         /* 选项按钮 */
    _btn_obj *recbtn = 0;       /* 录音按钮 */
    _btn_obj *stopbtn = 0;      /* 停止录音按钮 */

    uint16_t *pset_bkctbl = 0;  /* 设置时背景色指针 */
    uint32_t recsec = 0;        /* 录音时间 */

    uint8_t timecnt = 0;
    uint8_t vulevel = 0;
    short *tempvubuf;
    short tempval;
    uint16_t maxval = 0;
    uint16_t i;

    uint8_t recspd = 1;     /* 录音采样率选择,默认选择16Khz录音 */
    uint8_t recagc = 5;     /* 默认增益为5*3dB */
    
    uint8_t *p;             /* 指针 */
    uint8_t *buf;           /* 缓存 */
    uint32_t readlen;       /* 总读取长度 */
    uint16_t bread;         /* 读取的长度 */

    /* 申请内存 */
    for (i = 0; i < SAI_RX_FIFO_SIZE; i++)
    {
        p_sai_recfifo_buf[i] = gui_memin_malloc(SAI_RX_DMA_BUF_SIZE);   /* I2S录音FIFO内存申请 */

        if (p_sai_recfifo_buf[i] == NULL)
        {
            break;  /* 申请失败 */
        }
    }

    sairecbuf1 = gui_memin_malloc(SAI_RX_DMA_BUF_SIZE);  /* I2S录音内存1申请 */
    sairecbuf2 = gui_memin_malloc(SAI_RX_DMA_BUF_SIZE);  /* I2S录音内存2申请 */
    
    f_rec = (FIL *)gui_memin_malloc(sizeof(FIL));       /* 开辟FIL字节的内存区域 */
    wavhead = (__WaveHeader *)gui_memin_malloc(sizeof(__WaveHeader)); /* 开辟__WaveHeader字节的内存区域 */
    pname = gui_memin_malloc(60);                       /* 申请30个字节内存,类似"0:RECORDER/REC20120321210633.wav" */
    pset_bkctbl = gui_memex_malloc(180 * 272 * 2);      /* 为设置时的背景色表申请内存 */
    buf = gui_memin_malloc(1024);                       /* 读buf申请1024字节缓存 */

    if (!sairecbuf2 || !f_rec || !wavhead || !buf || !pset_bkctbl)rval = 1;
    else
    {
        /* 加载字体 */
        res = f_open(f_rec, (const TCHAR *)APP_ASCII_S6030, FA_READ); /* 打开文件夹 */

        if (res == FR_OK)
        {
            asc2_s6030 = (uint8_t *)gui_memex_malloc(f_rec->obj.objsize);   /* 为大字体开辟缓存地址 */

            if (asc2_s6030 == 0)rval = 1;
            else
            {
                p = asc2_s6030;
                readlen = 0;

                while (readlen < f_rec->obj.objsize)    /* 循环读取 */
                {
                    res = f_read(f_rec, buf, 1024, (UINT *)&bread); /* 读出rec里面的内容 */
                    readlen += bread;
                    my_mem_copy(p, buf, bread);
                    p += bread;

                    if (res)break;
                }
            }
        }

        if (res)rval = 1;

        recorder_load_ui(); /* 装载主界面 */
        rbtn = btn_creat(lcddev.width - 2 * gui_phy.tbfsize - 8 - 1, lcddev.height - gui_phy.tbheight, 2 * gui_phy.tbfsize + 8, gui_phy.tbheight - 1, 0, 0x03); /* 创建文字按钮 */
        mbtn = btn_creat(0, lcddev.height - gui_phy.tbheight, 2 * gui_phy.tbfsize + 8, gui_phy.tbheight - 1, 0, 0x03);  /* 创建文字按钮 */
        recbtn = btn_creat((lcddev.width - 96) / 3, (lcddev.height - 320) / 2 + 215 + 18, 48, 48, 0, 1);                /* 创建图片按钮 */
        stopbtn = btn_creat((lcddev.width - 96) * 2 / 3 + 48, (lcddev.height - 320) / 2 + 215 + 18, 48, 48, 0, 1);      /* 创建图片按钮 */

        if (!rbtn || !mbtn || !recbtn || !stopbtn)rval = 1; /* 没有足够内存够分配 */
        else
        {
            rbtn->caption = (uint8_t *)GUI_BACK_CAPTION_TBL[gui_phy.language]; /* 返回 */
            rbtn->font = gui_phy.tbfsize;   /* 设置新的字体大小 */
            rbtn->bcfdcolor = WHITE;        /* 按下时的颜色 */
            rbtn->bcfucolor = WHITE;        /* 松开时的颜色 */

            mbtn->caption = (uint8_t *)GUI_OPTION_CAPTION_TBL[gui_phy.language]; /* 返回 */
            mbtn->font = gui_phy.tbfsize;   /* 设置新的字体大小 */
            mbtn->bcfdcolor = WHITE;        /* 按下时的颜色 */
            mbtn->bcfucolor = WHITE;        /* 松开时的颜色 */

            recbtn->picbtnpathu = (uint8_t *)RECORDER_RECR_PIC;
            recbtn->picbtnpathd = (uint8_t *)RECORDER_PAUSEP_PIC;
            recbtn->bcfucolor = 0X0001;     /* 不填充背景 */
            recbtn->bcfdcolor = 0X0001;     /* 不填充背景 */
            recbtn->sta = 0;

            stopbtn->picbtnpathu = (uint8_t *)RECORDER_STOPR_PIC;
            stopbtn->picbtnpathd = (uint8_t *)RECORDER_STOPP_PIC;
            stopbtn->bcfucolor = 0X0001;    /* 不填充背景 */
            stopbtn->bcfdcolor = 0X0001;    /* 不填充背景 */
            recbtn->sta = 0;
        }
    }

    if (rval == 0)
    {
        if (gui_phy.memdevflag & (1 << 0))f_mkdir("0:RECORDER");    /* 强制创建文件夹,给录音机用 */

        if (gui_phy.memdevflag & (1 << 3))f_mkdir("3:RECORDER");    /* 强制创建文件夹,给录音机用 */

        btn_draw(rbtn);
        btn_draw(mbtn);
        btn_draw(recbtn);
        recbtn->picbtnpathu = (uint8_t *)RECORDER_PAUSER_PIC;
        recbtn->picbtnpathd = (uint8_t *)RECORDER_RECP_PIC;
        btn_draw(stopbtn);

        if (g_audiodev.status & (1 << 7))   /* 当前在放歌??必须停止 */
        {
            audio_stop_req(&g_audiodev);    /* 停止音频播放 */
            audio_task_delete();            /* 删除音乐播放任务 */
        }

        tempvubuf = 0;
        recsec = 0;
        g_rec_sta = 0;
        g_wav_size = 0;
        recorder_enter_rec_mode();          /* 进入录音模式,并设置AGC */
        recoder_set_samplerate(wavhead, recorder_samplerate_tbl[recspd]); /* 设置采样率 */
        recorder_show_samplerate((lcddev.width - 218) / 2, (lcddev.height - 320) / 2 + 200 + 5 - 15, recorder_samplerate_tbl[recspd]); /* 显示采样率 */
        es8388_mic_gain(recagc);            /* 设置增益 */
        recorder_show_time((lcddev.width - 150) / 2, 40 + (lcddev.height - 320) / 2, recsec); /* 显示时间 */

        while (rval == 0)
        {
            tp_dev.scan(0);
            in_obj.get_key(&tp_dev, IN_TYPE_TOUCH); /* 得到按键键值 */
            

            if (rec_sai_fifo_read(&pdatabuf) && (g_rec_sta & 0X80)) /* 读取一次数据,读到数据了,写入文件 */
            {
                res = f_write(f_rec, pdatabuf, SAI_RX_DMA_BUF_SIZE, (UINT *)&bread);    /* 写入文件 */

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

                    for (i = 0; i < 512; i++)   /* 取前512个数据里面的最大值 */
                    {
                        tempval = tempvubuf[i];

                        if (tempval < 0)tempval = -tempval;

                        if (maxval < tempval)maxval = tempval;  /* 取最大值 */
                    }

                    tempval = recorder_vu_get(maxval);

                    if (tempval > vulevel)vulevel = tempval;
                    else if (vulevel)vulevel--;

                    recorder_vu_meter((lcddev.width - 218) / 2, (lcddev.height - 320) / 2 + 200 + 5, vulevel); /* 显示vu meter */
                    maxval = 0;
                }

                delay_ms(1000 / OS_TICKS_PER_SEC);  /* 延时一个时钟节拍 */

                if (recsec != (g_wav_size / wavhead->fmt.ByteRate)) /* 录音时间显示 */
                {
                    recsec = g_wav_size / wavhead->fmt.ByteRate;    /* 录音时间 */
                    recorder_show_time((lcddev.width - 150) / 2, 40 + (lcddev.height - 320) / 2, recsec); /* 显示时间 */
                }
            }
            
            if (system_task_return)
            {
                delay_ms(10);
                if (tpad_scan(1)) 
                {
                    break;  /* TPAD返回,再次确认,排除干扰 */
                }
                else system_task_return = 0;
            }

            res = btn_check(rbtn, &in_obj);         /* 检查返回按钮 */

            if (res && ((rbtn->sta & (1 << 7)) == 0) && (rbtn->sta & (1 << 6)))break; /* 返回按钮 */

            res = btn_check(mbtn, &in_obj);         /* 检查设置按钮 */

            if (res && ((mbtn->sta & (1 << 7)) == 0) && (mbtn->sta & (1 << 6)))
            {
                app_read_bkcolor((lcddev.width - 180) / 2, (lcddev.height - 272) / 2, 180, 272, pset_bkctbl); /* 读取背景色 */
                res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 152) / 2, 180, 72 + 40 * 2, (uint8_t **)recorder_setsel_tbl[gui_phy.language], 2, (uint8_t *)&rval, 0X90, (uint8_t *)recorder_modesel_tbl[gui_phy.language]); /* 2个选择 */

                if (res == 0)
                {
                    app_recover_bkcolor((lcddev.width - 180) / 2, (lcddev.height - 272) / 2, 180, 272, pset_bkctbl); /* 恢复背景色 */

                    if (rval == 0)  /* 设置采样率 */
                    {
                        if (g_rec_sta & 0X80) /* 正在录音?提示用户必须禁止录音再设置采样率 */
                        {
                            window_msg_box((lcddev.width - 180) / 2, (lcddev.height - 80) / 2, 180, 80, (uint8_t *)recorder_remind_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
                            delay_ms(1500);
                        }
                        else
                        {
                            res = app_items_sel((lcddev.width - 180) / 2, (lcddev.height - 272) / 2, 180, 72 + 40 * 5, (uint8_t **)recorder_sampleratemsg_tbl, 5, (uint8_t *)&recspd, 0X90, (uint8_t *)recorder_setsel_tbl[gui_phy.language][0]); /* 单选 */

                            if (res == 0)recoder_set_samplerate(wavhead, recorder_samplerate_tbl[recspd]); /* 设置采样率 */
                        }
                    }
                    else    /* 设置AGC增益 */
                    {
                        res = recorder_agc_set((lcddev.width - 180) / 2, (lcddev.height - 122) / 2, &recagc, (uint8_t *)recorder_setsel_tbl[gui_phy.language][1]); /* 设置AGC */
                        es8388_mic_gain(recagc); /* 设置增益 */
                    }
                }

                rval = 0;   /* 恢复rval的值 */
                app_recover_bkcolor((lcddev.width - 180) / 2, (lcddev.height - 272) / 2, 180, 272, pset_bkctbl); /* 恢复背景色 */
                recorder_show_samplerate((lcddev.width - 218) / 2, (lcddev.height - 320) / 2 + 200 + 5 - 15, recorder_samplerate_tbl[recspd]); /* 显示采样率 */
            }

            res = btn_check(recbtn, &in_obj);   /* 检查录音按钮 */

            if (res && ((recbtn->sta & (1 << 7)) == 0) && (recbtn->sta & (1 << 6)))
            {
                if (g_rec_sta & 0X01)     /* 原来是暂停,继续录音 */
                {
                    g_rec_sta &= 0XFE;    /* 取消暂停 */
                    recbtn->picbtnpathu = (uint8_t *)RECORDER_RECR_PIC;
                    recbtn->picbtnpathd = (uint8_t *)RECORDER_PAUSEP_PIC;
                }
                else if (g_rec_sta & 0X80)     /* 已经在录音了,暂停 */
                {
                    g_rec_sta |= 0X01;    /* 暂停 */
                    recbtn->picbtnpathu = (uint8_t *)RECORDER_PAUSER_PIC;
                    recbtn->picbtnpathd = (uint8_t *)RECORDER_RECP_PIC;
                }
                else    /* 还没开始录音 */
                {
                    g_rec_sta |= 0X80;    /* 开始录音 */
                    g_wav_size = 0;        /* 文件大小设置为0 */
                    recbtn->picbtnpathu = (uint8_t *)RECORDER_RECR_PIC;
                    recbtn->picbtnpathd = (uint8_t *)RECORDER_PAUSEP_PIC;
                    pname[0] = '\0';    /* 添加结束符 */
                    recorder_new_pathname(pname);   /* 得到新的名字 */
                    recorder_show_name(2, gui_phy.tbheight + 4, pname); /* 显示名字 */
                    recorder_wav_init(wavhead);     /* 初始化wav数据 */
                    recoder_set_samplerate(wavhead, recorder_samplerate_tbl[recspd]); /* 设置采样率 */
                    res = f_open(f_rec, (const TCHAR *)pname, FA_CREATE_ALWAYS | FA_WRITE);

                    if (res)    /* 文件创建失败 */
                    {
                        g_rec_sta = 0;    /* 创建文件失败,不能录音 */
                        rval = 0XFE;    /* 提示是否存在SD卡 */
                    }
                    else res = f_write(f_rec, (const void *)wavhead, sizeof(__WaveHeader), (UINT *)&bread); /* 写入头数据 */
                }
            }

            res = btn_check(stopbtn, &in_obj);  /* 检查停止按钮 */

            if (res && ((recbtn->sta & (1 << 7)) == 0) && (recbtn->sta & (1 << 6)))
            {
                if (g_rec_sta & 0X80) /* 有录音 */
                {
                    wavhead->riff.ChunkSize = g_wav_size + 36; /* 整个文件的大小-8; */
                    wavhead->data.ChunkSize = g_wav_size;      /* 数据大小 */
                    f_lseek(f_rec, 0);                      /* 偏移到文件头 */
                    f_write(f_rec, (const void *)wavhead, sizeof(__WaveHeader), (UINT *)&bread); /* 写入头数据 */
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
                recorder_show_name(2, gui_phy.tbheight + 4, "");    /* 显示名字 */
                recorder_show_time((lcddev.width - 150) / 2, 40 + (lcddev.height - 320) / 2, recsec); /* 显示时间 */
            }
        }
        
        recorder_stop_rec_mode();/* 停止录音 */
    }
    else
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)recorder_remind_tbl[2][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* 内存错误 */
        delay_ms(2000);
    }

    if (rval == 0XFE)       /* 创建文件失败了,需要提示是否存在SD卡 */
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)APP_CREAT_ERR_MSG_TBL[gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* 提示SD卡是否存在 */
        delay_ms(2000);     /* 等待2秒钟 */
    }

    if (g_rec_sta & 0X80)     /* 如果正在录音,则提示保存这个录音文件 */
    {
        res = window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, "", (uint8_t *)recorder_remind_tbl[0][gui_phy.language], 12, 0, 0X03, 0);

        if (res == 1)       /* 需要保存 */
        {
            wavhead->riff.ChunkSize = g_wav_size + 36; /* 整个文件的大小-8; */
            wavhead->data.ChunkSize = g_wav_size;      /* 数据大小 */
            f_lseek(f_rec, 0);                      /* 偏移到文件头 */
            f_write(f_rec, (const void *)wavhead, sizeof(__WaveHeader), (UINT *)&bread); /* 写入头数据 */
            f_close(f_rec);
        }
    }

    /* 释放内存 */
    for (i = 0; i < SAI_RX_FIFO_SIZE; i++)
    {
        gui_memin_free(p_sai_recfifo_buf[i]);   /* I2S录音FIFO内存释放 */
    }
    
    gui_memin_free(sairecbuf1);
    gui_memin_free(sairecbuf2);
    gui_memin_free(f_rec);
    gui_memin_free(wavhead);
    gui_memin_free(pname);
    gui_memex_free(pset_bkctbl);
    gui_memex_free(asc2_s6030);
    gui_memin_free(buf);
    asc2_s6030 = 0; /* 清零 */
    btn_delete(rbtn);
    btn_delete(mbtn);
    btn_delete(recbtn);
    btn_delete(stopbtn);
    return rval;
}



































