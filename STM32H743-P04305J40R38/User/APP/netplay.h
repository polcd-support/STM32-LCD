/**
 ****************************************************************************************************
 * @file        netplay.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-12-01
 * @brief       APP-����ͨ�� ����
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.1 20221201
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
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


#define NET_RMEMO_MAXLEN        10000       /* RMEMO�����յ��ַ���.����ܳ���65535 */
#define NET_SMEMO_MAXLEN        400         /* RMEMO�����������400���ַ� */


#define NET_IP_BACK_COLOR       0X42F0      /* IP���򱳾���ɫ */
#define NET_COM_RIM_COLOR       0X7BCF      /* �ָ�����ɫ */
#define NET_MSG_FONT_COLOR      0X4A49      /* ��ʾ��Ϣ������ɫ */
#define NET_MSG_BACK_COLOR      0XBE3B      /* ��ʾ��Ϣ������ɫ */
#define NET_MEMO_BACK_COLOR     0XA599      /* 2��memo���򱳾���ɫ */


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















 



















