/**
 ****************************************************************************************************
 * @file        timer.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-25
 * @brief       ͨ�ö�ʱ��(�����ۺϲ���ʵ��) ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F103������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221125
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __TIMER_H
#define __TIMER_H


#include "./SYSTEM/sys/sys.h"


/* LCD PWM��������  */
#define LCD_BLPWM_VAL       TIM3->CCR2


void tim3_ch2_pwm_init(uint16_t arr, uint16_t psc);     /* ��ʱ��3 CH2 PWM��ʼ������ */
void tim7_int_init(uint16_t arr, uint16_t psc);         /* ��ʱ��7 ��ʱ�ж����� */
void tim8_int_init(uint16_t arr, uint16_t psc);         /* ��ʱ��3 ��ʱ�ж����� */
void tim6_int_init(uint16_t arr, uint16_t psc);         /* ��ʱ��6 ��ʱ�ж����� */

#endif

















