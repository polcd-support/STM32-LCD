/**
 ****************************************************************************************************
 * @file        netplay.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2022-12-01
 * @brief       APP-网络通信 代码
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
 * V1.1 20221201
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#ifndef __NETPLAY_H
#define __NETPLAY_H
#include "common.h"  
#include "lwip_comm.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "httpd.h" 


#define NET_RMEMO_MAXLEN        10000       /* RMEMO最大接收的字符数.最大不能超过65535 */
#define NET_SMEMO_MAXLEN        400         /* RMEMO最大允许输入400个字符 */


#define NET_IP_BACK_COLOR       0X42F0      /* IP区域背景颜色 */
#define NET_COM_RIM_COLOR       0X7BCF      /* 分割线颜色 */
#define NET_MSG_FONT_COLOR      0X4A49      /* 提示消息字体颜色 */
#define NET_MSG_BACK_COLOR      0XBE3B      /* 提示消息背景颜色 */
#define NET_MEMO_BACK_COLOR     0XA599      /* 2个memo区域背景颜色 */


void net_load_ui(void);
void net_msg_show(uint16_t y,uint16_t height,uint8_t fsize,uint32_t tx,uint32_t rx,uint8_t prot,uint8_t flag); 
void net_edit_colorset(_edit_obj *ipx,_edit_obj *portx,uint8_t prot,uint8_t connsta); 
uint16_t net_get_port(uint8_t *str);
uint32_t net_get_ip(uint8_t *str); 
void net_tcpserver_remove_timewait(void); 
void net_disconnect(struct netconn *netconn1,struct netconn *netconn2);
uint8_t net_test(void);
uint8_t net_play(void);

#endif















 



















