/**
 ****************************************************************************************************
 * @file        tomcatplay.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2022-10-28
 * @brief       APP-TOM猫 代码
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
 * V1.1 20221028
 * 1, 修改注释方式
 * 2, 修改uint8_t/uint16_t/uint32_t为uint8_t/uint16_t/uint32_t
 *
 ****************************************************************************************************
 */

#ifndef __TOMCATPLAY_H
#define __TOMCATPLAY_H

#include "common.h"


/* 各图标/图片路径 */
extern uint8_t*const TOMCAT_DEMO_PIC;//demo图片路径 	      


void tomcat_load_ui(void);
void tomcat_show_spd(uint16_t x,uint16_t y,uint16_t spd);
uint8_t tomcat_agcspd_set(uint16_t x,uint16_t y,uint8_t *agc,uint16_t *speed,uint8_t*caption);
void tomcat_rec_mode(uint8_t agc);
void tomcat_play_wav(uint8_t *buf,uint32_t len);
void tomcat_data_move(uint8_t* buf,uint16_t size,uint16_t dx);
uint8_t tomcat_play(void);

#endif























