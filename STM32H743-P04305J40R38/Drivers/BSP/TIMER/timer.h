/**
 ****************************************************************************************************
 * @file        timer.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-25
 * @brief       通用定时器(服务综合测试实验) 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20221125
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __TIMER_H
#define __TIMER_H


#include "./SYSTEM/sys/sys.h"


/* LCD PWM背光设置  */
#define LCD_BLPWM_VAL       TIM3->CCR2


void tim3_ch2_pwm_init(uint16_t arr, uint16_t psc);     /* 定时器3 CH2 PWM初始化函数 */
void tim7_int_init(uint16_t arr, uint16_t psc);         /* 定时器7 定时中断配置 */
void tim8_int_init(uint16_t arr, uint16_t psc);         /* 定时器3 定时中断配置 */
void tim6_int_init(uint16_t arr, uint16_t psc);         /* 定时器6 定时中断配置 */

#endif

















