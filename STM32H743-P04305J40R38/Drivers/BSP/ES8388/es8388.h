/**
 ****************************************************************************************************
 * @file        es8388.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-14
 * @brief       ES8388 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
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
 * V1.0 20220114
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __ES8388_H
#define __ES8388_H

#include "./SYSTEM/SYS/sys.h"


__packed typedef struct
{
    uint8_t mvol;       /* 音量(耳机和喇叭一起调),范围:0~63 */
    uint8_t cfreq[5];   /* 中心频率选择数组,每个元素可选范围为0~3,且代表的频率各不相同
                         * [0]对应频率:80,105,135,175
                         * [1]对应频率:230,300,385,500
                         * [2]对应频率:650,850,1100,1400
                         * [3]对应频率:1800,2400,3200,4100
                         * [4]对应频率:5300,6900,9000,11700 
                         */
    
    uint8_t freqval[5]; /* 中心频率增益表 */
    uint8_t d3;         /* 3d设置 */
    uint8_t speakersw;  /* 板载喇叭开关,0,关闭;1,打开 */
    uint8_t saveflag;   /* 保存标志,0X0A,保存过了;其他,还从未保存 */
} _es8388_obj;

extern _es8388_obj es8388set;/* ES8388的设置 */

#define ES8388_ADDR     0x10                        /* ES8388的器件地址,固定为0x10 */

uint8_t es8388_init(void);                          /* ES8388初始化 */
uint8_t es8388_write_reg(uint8_t reg, uint8_t val); /* ES8388写寄存器 */
uint8_t es8388_read_reg(uint8_t reg);               /* ES8388读寄存器 */
void es8388_sai_cfg(uint8_t fmt, uint8_t len);      /* 设置SAI工作模式 */
void es8388_hpvol_set(uint8_t volume);              /* 设置耳机音量 */
void es8388_spkvol_set(uint8_t volume);             /* 设置喇叭音量 */
void es8388_3d_set(uint8_t depth);                  /* 设置3D环绕声 */
void es8388_adda_cfg(uint8_t dacen, uint8_t adcen); /* ES8388 DAC/ADC配置 */
void es8388_output_cfg(uint8_t o1en, uint8_t o2en); /* ES8388 DAC输出通道配置 */
void es8388_mic_gain(uint8_t gain);                 /* ES8388 MIC增益设置(MIC PGA增益) */
void es8388_alc_ctrl(uint8_t sel, uint8_t maxgain, uint8_t mingain);    /* ES8388 ALC设置 */
void es8388_input_cfg(uint8_t in);                  /* ES8388 ADC输出通道配置 */
void es8388_eq1_eet(uint8_t cfreq,uint8_t gain);
void es8388_eq2_eet(uint8_t cfreq,uint8_t gain);
void es8388_eq3_eet(uint8_t cfreq,uint8_t gain);
void es8388_eq4_eet(uint8_t cfreq,uint8_t gain);
void es8388_eq5_eet(uint8_t cfreq,uint8_t gain);
#endif


