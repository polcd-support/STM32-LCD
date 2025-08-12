/**
 ****************************************************************************************************
 * @file        ethernetif.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.3
 * @date        2022-12-05
 * @brief       lwip-网络接口驱动 代码
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
 * V1.0 20221205
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__
#include "lwip/err.h"
#include "lwip/netif.h"


/* 网卡的名字 */
#define IFNAME0 'e'
#define IFNAME1 'n'


err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);
#endif
