/**
 ****************************************************************************************************
 * @file        usart3.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-02
 * @brief       串口3 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 F429开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20221202
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __USART3_H
#define __USART3_H

#include "./SYSTEM/sys/sys.h"
#include "stdio.h"


#define USART3_MAX_RECV_LEN     600         /* 最大接收缓存字节数 */
#define USART3_MAX_SEND_LEN     600         /* 最大发送缓存字节数 */
#define USART3_RX_EN            1           /* 0,不接收;1,接收 */


extern uint8_t  g_usart3_rx_buf[USART3_MAX_RECV_LEN];   /* 接收缓冲,最大USART3_MAX_RECV_LEN字节 */
extern volatile uint16_t g_usart3_rx_sta;               /* 接收数据状态 */


void usart3_init(uint32_t sclk,uint32_t baudrate);
void u3_printf(char* fmt,...);
#endif
















