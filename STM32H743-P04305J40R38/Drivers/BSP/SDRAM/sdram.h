/**
 ****************************************************************************************************
 * @file        sdram.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       SDRAM ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230321
 * ��һ�η���
 *
 ****************************************************************************************************
 */
 
#ifndef _SDRAM_H
#define _SDRAM_H

#include "./SYSTEM/sys/sys.h"


#define BANK5_SDRAM_ADDR        ((uint32_t)(0XC0000000))            /* SDRAM��ʼ��ַ */


uint8_t sdram_send_cmd(uint8_t bankx, uint8_t cmd, uint8_t refresh, uint16_t regval);
void sdram_init(void);
void fmc_sdram_write_buffer(uint8_t *pbuf, uint32_t writeaddr, uint32_t n);
void fmc_sdram_read_buffer(uint8_t *pbuf, uint32_t readaddr, uint32_t n);

#endif



































