/**
 ****************************************************************************************************
 * @file        stm32_audio.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-01-06
 * @brief       stm32_audio 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
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
 * V1.0 20230106
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/USART/usart.h"
#include "stm32_audio.h"
#include "usbd_audio_if.h"
#include "usbd_audio.h"


uint8_t g_volume = 0;                       /* 当前音量 */
extern uint8_t g_usbd_audio_dataout_en;     /* USB数据输出状态,在usbd_audio.c里面定义 */


/**
 * @brief       播放时,I2S DMA传输回调函数
 * @param       无
 * @retval      无
 */
void bsp_audio_i2s_dma_callback(void)
{
    BSP_AUDIO_OUT_TransferComplete_CallBack();      /* 执行回调函数,更新数据 */
}


/**
 * @brief  配置音频接口
 * @param  OutputDevice :输出设备选择,未用到.
 * @param  Volume       :音量大小, 0~100
 * @param  AudioFreq    :音频采样率
 * @retval               操作结果
 *   @arg                0    , 成功
 *   @arg                其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
    es8388_i2s_cfg(0, 3);               /* 飞利浦标准,16位数据长度 */
    sai1_saia_init(0, 1, 4);            /* 设置SAI,主发送,16位数据 */
    sai1_samplerate_set(AudioFreq);     /* 设置采样率 */
    BSP_AUDIO_OUT_SetVolume(Volume);    /* 设置音量 */
    sai1_tx_callback = bsp_audio_i2s_dma_callback;  /* 回调函数指 bsp_audio_i2s_dma_callback */
    
    printf("EVAL_AUDIO_Init:%d,%d\r\n", Volume, AudioFreq);
    
    return 0;
}

/**
 * @brief  开始播放音频数据
 * @param  pBuffer  : 音频数据流首地址指针
 * @param  Size     : 数据流大小(单位:字节)
 * @retval            操作结果
 *   @arg             0    , 成功
 *   @arg             其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size)
{
    sai1_tx_dma_init((uint8_t*) pBuffer, 0, Size / 2, 1);   /* 配置I2S TX DMA */
    SAI1_TX_DMASx->CR &= ~(1 << 8);     /* 单次模式 */
    SAI1_TX_DMASx->CR &= ~(1 << 18);    /* 改成单缓冲模式 */
    sai1_play_start();                  /* 开启DMA */
    
    printf("EVAL_AUDIO_Play:%x,%d\r\n", pBuffer, Size);
    
    return 0;
}

/**
 * @brief  切换音频数据流buffer,每次DMA中断完成会调用该函数
 * @param  pData    : 音频数据流缓存首地址
 * @param  Size     : 音频数据流大小(单位:harf word,也就是2个字节)
 * @retval          : 无
  */
void BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size)
{
    if (g_usbd_audio_dataout_en == 1)       /* USB有音频数据输出 */
    {
        g_usbd_audio_dataout_en = 0;        /* 清除标记位 */
    }
    else if (g_usbd_audio_dataout_en == 0)  /* 没有收到任何数据,全部填充0 */
    {
        my_mem_set(pData, 0, AUDIO_TOTAL_BUF_SIZE);
        g_usbd_audio_dataout_en = 2;        /* 标记已经清零了buffer,停止数据输出 */
    }

    if ((Size * 2) != AUDIO_TOTAL_BUF_SIZE)
    {
        //printf("chgbuf:%x,%d\r\n", pData, Size);  /* 想观察数据,可以printf,但是会有轻微的音箱音质 */
    }

    SAI1_TX_DMASx->CR &= ~(1 << 0);         /* 关闭DMA传输 */

    while (SAI1_TX_DMASx->CR & 0X1);        /* 确保DMA可以被设置 */

    SAI1_TX_DMASx->NDTR = Size ;            /* 设置传输长度 */
    SAI1_TX_DMASx->CR |= 1 << 0;            /* 开启DMA传输 */
}

/**
 * @brief  停止播放
 * @retval            操作结果
 *   @arg             0    , 成功
 *   @arg             其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_Pause(void)
{
    printf("EVAL_AUDIO_Stop\r\n");
    return 0;
}

/**
 * @brief  停止播放
 * @param  Option   : 未用到
 * @retval            操作结果
 *   @arg             0    , 成功
 *   @arg             其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option)
{
    printf("EVAL_AUDIO_Stop\r\n");
    return 0;
}

/**
 * @brief  音量设置
 * @param  Volume: 音量大小, 0~100
 * @retval      操作结果
 *   @arg       0    , 成功
 *   @arg       其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume)
{
    g_volume = Volume;
    es8388_hpvol_set(g_volume * 0.3);
    es8388_spkvol_set(g_volume * 0.3);
    return 0;
}

/**
 * @brief  静音设置
 * @param  Cmd      : 0,不静音; 1,静音
 * @retval            操作结果
 *   @arg             0    , 成功
 *   @arg             其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd)
{
    if(Cmd)
    {
        es8388_output_cfg(0, 0);    /* 关闭输出 ,就是静音 */
    }else
    {
        es8388_output_cfg(1, 1);    /* 打开输出 ,取消静音 */
    }
    
    printf("BSP_AUDIO_OUT_SetMute:%d\r\n", Cmd);
    return 0;
}





















