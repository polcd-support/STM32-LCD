/**
 ****************************************************************************************************
 * @file        imu.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-07-19
 * @brief       姿态解算 代码
 *              核心代码参考自:https://gitee.com/brimon-zzy/icm20602forstm32f103?_from=gitee_search
 * @license     
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
 * V1.0 20220719
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __IMU_H
#define __IMU_H

#include "./SYSTEM/sys/sys.h"


/* 四元数结构体类型 */
typedef struct{
    float q0;
    float q1;
    float q2;
    float q3;
}quater_info_t;


/* 欧拉角结构体类型 */
typedef struct{
    float pitch;
    float roll;
    float yaw;
}eulerian_angles_t;

uint8_t imu_init(void);
eulerian_angles_t imu_get_eulerian_angles(float gx, float gy, float gz, float ax, float ay, float az);
void imu_data_calibration(short *gx, short *gy, short *gz, short *ax, short *ay, short *az);

#endif
