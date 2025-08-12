/**
 ****************************************************************************************************
 * @file        lwip_comm.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-05
 * @brief       lwip通用驱动 代码
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
 * V1.3 20221205
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H

#include "./BSP/ETHERNET/ethernet.h"


#define LWIP_MAX_DHCP_TRIES      10    /* DHCP服务器最大重试次数 */

/* lwip控制结构体 */
typedef struct  
{
    uint8_t mac[6];              /* MAC地址 */
    uint8_t remoteip[4];         /* 远端主机IP地址 */
    uint8_t ip[4];               /* 本机IP地址 */
    uint8_t netmask[4];          /* 子网掩码 */
    uint8_t gateway[4];          /* 默认网关的IP地址 */
    
    volatile uint8_t dhcpstatus; /* dhcp状态 */
                                 /* 0,未获取DHCP地址; */
                                 /* 1,进入DHCP获取状态 */
                                 /* 2,成功获取DHCP地址 */
                                 /* 0XFF,获取失败. */
}__lwip_dev;

extern __lwip_dev lwipdev;       /* lwip控制结构体 */
 
void lwip_pkt_handle(void);
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
uint8_t lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);
uint8_t lwip_comm_init(void);
void lwip_comm_dhcp_creat(void);
void lwip_comm_dhcp_delete(void);
void lwip_comm_destroy(void);
void lwip_comm_delete_next_timeout(void);

#endif













