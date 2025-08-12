/**
 ****************************************************************************************************
 * @file        tpad.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-16
 * @brief       电容触摸按键(TPAD) 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230316
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __TPAD_H
#define __TPAD_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* TPAD 引脚 及 定时器 定义 */

/* 我们使用定时器的输入捕获功能, 对TPAD进行检测
 * 这里的输入捕获使用定时器TIM2_CH1, 捕获TPAD按键的输入
 */
#define TPAD_GPIO_PORT                          GPIOA
#define TPAD_GPIO_PIN                           SYS_GPIO_PIN5
#define TPAD_GPIO_AF                            1                                               /* AF功能选择 */
#define TPAD_GPIO_CLK_ENABLE()                  do{ RCC->AHB4ENR |= 1 << 0; }while(0)           /* PA口时钟使能 */

#define TPAD_TIMX_CAP                           TIM2
#define TPAD_TIMX_CAP_CHY                       1                                               /* 通道Y,  1<= Y <=4 */
#define TPAD_TIMX_CAP_CHY_CCRX                  TIM2->CCR1                                      /* 通道Y的捕获/比较寄存器 */
#define TPAD_TIMX_CAP_CHY_CLK_ENABLE()          do{ RCC->APB1LENR |= 1 << 0; }while(0)          /* TIM2 时钟使能 */

/******************************************************************************************/

/* 触摸的门限值, 也就是必须大于 g_tpad_default_val + TPAD_GATE_VAL
 * 才认为是有效触摸, 改大 TPAD_GATE_VAL, 可以降低灵敏度, 反之, 则可以提高灵敏度
 * 根据实际需求, 选择合适的 TPAD_GATE_VAL 即可
 */
#define TPAD_GATE_VAL       100                 /* 触摸的门限值, 也就是必须大于 g_tpad_default_val + TPAD_GATE_VAL, 才认为是有效触摸 */
#define TPAD_ARR_MAX_VAL    0xFFFFFFFF          /* 最大的ARR值, 一般设置为定时器的ARR最大值 */


/* 接口函数, 可以在其他.c调用 */
uint8_t tpad_init(uint16_t psc);                /* TPAD 初始化 函数 */
uint8_t tpad_scan(uint8_t mode);                /* TPAD 扫描 函数 */


extern volatile uint16_t g_tpad_default_val;    /* 空载的时候(没有手按下),计数器需要的时间 */

/* 静态函数, 仅限 tapd.c调用 */
static void tpad_reset(void);                   /* 复位 */
static uint16_t tpad_get_val(void);             /* 得到定时器捕获值 */
static uint16_t tpad_get_maxval(uint8_t n);     /* 读取n次, 获取最大值 */
static void tpad_timx_cap_init(uint32_t arr, uint16_t psc); /* 定时器输入捕获初始化 */


#endif













