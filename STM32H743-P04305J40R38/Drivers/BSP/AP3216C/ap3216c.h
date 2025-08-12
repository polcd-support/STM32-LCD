/**
 ****************************************************************************************************
 * @file        ap3216c.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       AP3216C 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230322
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __AP3216C_H
#define __AP3216C_H

#include "./SYSTEM/sys/sys.h"


#define AP3216C_ADDR    0X3C    /* AP3216C器件IIC地址(左移了一位) */


uint8_t ap3216c_init(void); 
uint8_t ap3216c_write_one_byte(uint8_t reg, uint8_t data);
uint8_t ap3216c_read_one_byte(uint8_t reg);
void ap3216c_read_data(uint16_t* ir, uint16_t* ps, uint16_t* als);

#endif
