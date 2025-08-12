/**
 ****************************************************************************************************
 * @file        apeplay.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-27
 * @brief       ape解码 代码
 *              1, 支持16位单声道/立体声ape的解码
 *              2, 最高支持96K的APE格式(LV1和LV2)
 *              3, LV1~LV3,在48K及以下流畅播放,LV4,LV5大卡
 *              4, 对某些ape文件,可能不支持,请用Monkey's Audio软件进行转换一下,即可正常播放
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

#ifndef __APEPLAY_H__
#define __APEPLAY_H__

#include "apedecoder.h"
#include "parser.h"
#include "./SYSTEM/sys/sys.h"


#define APE_FILE_BUF_SZ         48*1024     /* APE解码时,文件buf大小 */
#define APE_BLOCKS_PER_LOOP     2*1024      /* APE解码时,每个循环解码block的个数 */

/* APE控制结构体 */
typedef __packed struct
{
    uint32_t totsec ;       /* 整首歌时长,单位:秒 */
    uint32_t cursec ;       /* 当前播放时长 */

    uint32_t bitrate;       /* 比特率 */
    uint32_t samplerate;    /* 采样率 */
    uint16_t outsamples;    /* PCM输出数据量大小 */
    uint16_t bps;           /* 位数,比如16bit,24bit,32bit */

    uint32_t datastart;     /* 数据帧开始的位置(在文件里面的偏移) */
} __apectrl;

extern __apectrl *apectrl;


void ape_fill_buffer(uint32_t *buf, uint16_t size);
void ape_i2s_dma_tx_callback(void);
void ape_get_curtime(FIL *fx, __apectrl *apectrl);
uint8_t ape_play_song(uint8_t *fname);

#endif
