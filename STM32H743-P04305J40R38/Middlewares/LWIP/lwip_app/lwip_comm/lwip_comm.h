/**
 ****************************************************************************************************
 * @file        lwip_comm.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-05
 * @brief       lwipͨ������ ����
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
 * V1.3 20221205
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H

#include "./BSP/ETHERNET/ethernet.h"


#define LWIP_MAX_DHCP_TRIES      10    /* DHCP������������Դ��� */

/* lwip���ƽṹ�� */
typedef struct  
{
    uint8_t mac[6];              /* MAC��ַ */
    uint8_t remoteip[4];         /* Զ������IP��ַ */
    uint8_t ip[4];               /* ����IP��ַ */
    uint8_t netmask[4];          /* �������� */
    uint8_t gateway[4];          /* Ĭ�����ص�IP��ַ */
    
    volatile uint8_t dhcpstatus; /* dhcp״̬ */
                                 /* 0,δ��ȡDHCP��ַ; */
                                 /* 1,����DHCP��ȡ״̬ */
                                 /* 2,�ɹ���ȡDHCP��ַ */
                                 /* 0XFF,��ȡʧ��. */
}__lwip_dev;

extern __lwip_dev lwipdev;       /* lwip���ƽṹ�� */
 
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













