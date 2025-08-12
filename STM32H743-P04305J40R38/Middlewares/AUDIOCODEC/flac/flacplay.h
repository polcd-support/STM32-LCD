/**
 ****************************************************************************************************
 * @file        flacplay.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       flac解码 代码
 *              1, 支持16/24位单声道/立体声flac的解码
 *              2, 最高支持192K/16bit或96K/24bit的flac解码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
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
 * V1.0 20221127
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __FLACPLAY_H__
#define __FLACPLAY_H__

#include <inttypes.h>
#include <string.h>
#include "flacdecoder.h"
#include "./SYSTEM/sys/sys.h"
#include "./FATFS/source/ff.h"


/* flaC 标签 */
typedef __packed struct
{
    uint8_t id[3];      /* ID,在文件起始位置,必须是flaC 4个字母 */
} FLAC_Tag;

/* metadata 数据块头信息结构体 */
typedef __packed struct
{
    uint8_t head;       /* metadata block头 */
    uint8_t size[3];    /* metadata block数据长度 */
} MD_Block_Head;

/* FLAC控制结构体 */
typedef __packed struct
{
    uint32_t totsec ;   /* 整首歌时长,单位:秒 */
    uint32_t cursec ;   /* 当前播放时长 */

    uint32_t bitrate;   /* 比特率 */
    uint32_t samplerate;/* 采样率 */
    uint16_t outsamples;/* PCM输出数据量大小 */
    uint16_t bps;       /* 位数,比如16bit,24bit,32bit */

    uint32_t datastart; /* 数据帧开始的位置(在文件里面的偏移) */
} __flacctrl;

extern __flacctrl *flacctrl;


uint8_t flac_init(FIL *fx, __flacctrl *fctrl, FLACContext *fc);
void flac_i2s_dma_tx_callback(void);
void flac_get_curtime(FIL *fx, __flacctrl *flacx);
uint8_t flac_play_song(uint8_t *fname);

#endif




























