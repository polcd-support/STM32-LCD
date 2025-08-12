/**
 ****************************************************************************************************
 * @file        usart3.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-02
 * @brief       ����3 ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ F429������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221202
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __USART3_H
#define __USART3_H

#include "./SYSTEM/sys/sys.h"
#include "stdio.h"


#define USART3_MAX_RECV_LEN     600         /* �����ջ����ֽ��� */
#define USART3_MAX_SEND_LEN     600         /* ����ͻ����ֽ��� */
#define USART3_RX_EN            1           /* 0,������;1,���� */


extern uint8_t  g_usart3_rx_buf[USART3_MAX_RECV_LEN];   /* ���ջ���,���USART3_MAX_RECV_LEN�ֽ� */
extern volatile uint16_t g_usart3_rx_sta;               /* ��������״̬ */


void usart3_init(uint32_t sclk,uint32_t baudrate);
void u3_printf(char* fmt,...);
#endif
















