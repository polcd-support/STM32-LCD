/**
 ****************************************************************************************************
 * @file        sdram.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       SDRAM 驱动代码
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
 * V1.0 20230321
 * 第一次发布
 *
 ****************************************************************************************************
 */
 
#ifndef _SDRAM_H
#define _SDRAM_H

#include "./SYSTEM/sys/sys.h"


#define BANK5_SDRAM_ADDR        ((uint32_t)(0XC0000000))            /* SDRAM开始地址 */


uint8_t sdram_send_cmd(uint8_t bankx, uint8_t cmd, uint8_t refresh, uint16_t regval);
void sdram_init(void);
void fmc_sdram_write_buffer(uint8_t *pbuf, uint32_t writeaddr, uint32_t n);
void fmc_sdram_read_buffer(uint8_t *pbuf, uint32_t readaddr, uint32_t n);

#endif



































