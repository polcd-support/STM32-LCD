/**
 ****************************************************************************************************
 * @file        stm32_audio.h
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

#ifndef __STM32_AUDIO_H
#define __STM32_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "./BSP/SAI/sai.h"
#include "./BSP/ES8388/es8388.h"



uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size);
void    BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size);
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option);
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume); 
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd); 



#ifdef __cplusplus
}
#endif

#endif














