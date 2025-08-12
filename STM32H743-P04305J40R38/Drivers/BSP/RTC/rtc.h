/**
 ****************************************************************************************************
 * @file        rtc.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       RTC ��������
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
 * V1.0 20230322
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __RTC_H
#define __RTC_H

#include "./SYSTEM/sys/sys.h"


uint8_t rtc_init(void);     /* ��ʼ��RTC */
uint32_t rtc_read_bkr(uint32_t bkrx);               /* ���󱸼Ĵ��� */
void rtc_write_bkr(uint32_t bkrx, uint32_t data);   /* д�󱸼Ĵ��� */
void rtc_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *ampm);    /* ��ȡʱ�� */
uint8_t rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ampm); /* ����ʱ�� */
void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week); /* ��ȡ���� */
uint8_t rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week);  /* �������� */

void rtc_set_wakeup(uint8_t wksel, uint16_t cnt);   /* ���������Ի��� */
uint8_t rtc_get_week(uint16_t year, uint8_t month, uint8_t day);    /* ��ȡ���� */
void rtc_set_alarma(uint8_t week, uint8_t hour, uint8_t min, uint8_t sec);  /* �������� */

#endif

















