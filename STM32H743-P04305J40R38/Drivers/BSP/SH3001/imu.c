/**
 ****************************************************************************************************
 * @file        imu.c
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

#include "./BSP/SH3001/imu.h"
#include "./BSP/SH3001/sh3001.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include <math.h>



#define IMU_DELTA_T         0.005f          /* 5ms计算一次 */
#define IMU_M_PI            3.1425f         /* PI值 */

#define IMU_NEW_WEIGHT      0.35f           /* 新值权重 */
#define IMU_OLD_WEIGHT      0.65f           /* 旧值权重 */


quater_info_t g_q_info = {1, 0, 0};         /* 全局四元数 */

float g_param_kp = 50.0f;                   /* 加速度计(磁力计)的收敛速率比例增益50 */
float g_param_ki = 0.20f;                   /* 陀螺仪收敛速率的积分增益 0.2 */

short g_acc_avg[3];                         /* 加速度平均值 */
short g_gyro_avg[3];                        /* 陀螺仪平均值 */


/**
 * @brief       开方函数
 * @param       x           : 待开方的值
 * @retval      开方结果
 */
static float imu_inv_sqrt(float x)
{
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

/**
 * @brief       数据转换
 * @note        对加速度数据做一阶低通滤波(参考匿名)，对gyro转成弧度每秒(2000dps)
 * @param       gx, gy, gz  : 3轴陀螺仪数据指针
 * @param       ax, ay, az  : 3轴加速度数据指针
 * @retval      无 
 */
static void imu_data_transform(float *gx, float *gy, float *gz, float *ax, float *ay, float *az)
{
    static double lastax = 0;
    static double lastay = 0;
    static double lastaz = 0;

    *ax = *ax * IMU_NEW_WEIGHT + lastax * IMU_OLD_WEIGHT;
    *ay = *ay * IMU_NEW_WEIGHT + lastay * IMU_OLD_WEIGHT;
    *az = *az * IMU_NEW_WEIGHT + lastaz * IMU_OLD_WEIGHT;

    lastax = *ax;
    lastay = *ay;
    lastaz = *az;
     
    *gx = *gx * IMU_M_PI / 180 / 16.4f;
    *gy = *gy * IMU_M_PI / 180 / 16.4f;
    *gz = *gz * IMU_M_PI / 180 / 16.4f;
}


/**
 * @brief       姿态解算融合, 核心算法
 * @note        使用的是互补滤波算法，没有使用Kalman滤波算法
 *              尽量保证该函数的调用频率为: IMU_DELTA_T , 否则YAW会相应的偏大/偏小
 * @param       gx, gy, gz  : 3轴陀螺仪数据
 * @param       ax, ay, az  : 3轴加速度数据
 * @retval      无 
 */
static void imu_ahrsupdate_nomagnetic(float gx, float gy, float gz, float ax, float ay, float az)
{
    static float i_ex, i_ey, i_ez;    /* 误差积分 */

    float half_t = 0.5f * IMU_DELTA_T;
    float vx, vy, vz;      /* 当前的机体坐标系上的重力单位向量 */
    float ex, ey, ez;      /* 四元数计算值与加速度计测量值的误差 */
    float q0 = g_q_info.q0;
    float q1 = g_q_info.q1;
    float q2 = g_q_info.q2;
    float q3 = g_q_info.q3;
    float q0q0 = q0 * q0;
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q0q3 = q0 * q3;
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;
    float delta_2 = 0;

    /* 对加速度数据进行归一化 得到单位加速度 */
    float norm = imu_inv_sqrt(ax * ax + ay * ay + az * az);
    ax = ax * norm;
    ay = ay * norm;
    az = az * norm;
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    ex = ay * vz - az * vy;
    ey = az * vx - ax * vz;
    ez = ax * vy - ay * vx;

    /* 用差乘误差来做PI修正陀螺仪零偏，
     * 通过调节 g_param_kp，g_param_ki 两个参数，
     * 可以控制加速度计修正陀螺仪积分姿态的速度。*/
    i_ex += IMU_DELTA_T * ex;   /* integral error scaled by Ki */
    i_ey += IMU_DELTA_T * ey;
    i_ez += IMU_DELTA_T * ez;

    gx = gx + g_param_kp * ex + g_param_ki * i_ex;
    gy = gy + g_param_kp * ey + g_param_ki * i_ey;
    gz = gz + g_param_kp * ez + g_param_ki * i_ez;


    /*数据修正完成，下面是四元数微分方程*/

    /* 四元数微分方程，其中half_t为测量周期的1/4，gx gy gz为陀螺仪角速度，
       以下都是已知量，这里使用了一阶龙哥库塔求解四元数微分方程 */
//    q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * half_t;
//    q1 = q1 + ( q0 * gx + q2 * gz - q3 * gy) * half_t;
//    q2 = q2 + ( q0 * gy - q1 * gz + q3 * gx) * half_t;
//    q3 = q3 + ( q0 * gz + q1 * gy - q2 * gx) * half_t;
    delta_2 = (2 * half_t * gx) * (2 * half_t * gx) + (2 * half_t * gy) * (2 * half_t * gy) + (2 * half_t * gz) * (2 * half_t * gz);

    /* 整合四元数率    四元数微分方程  四元数更新算法，二阶毕卡法 */
    q0 = (1 - delta_2 / 8) * q0 + (-q1 * gx - q2 * gy - q3 * gz) * half_t;
    q1 = (1 - delta_2 / 8) * q1 + (q0 * gx + q2 * gz - q3 * gy) * half_t;
    q2 = (1 - delta_2 / 8) * q2 + (q0 * gy - q1 * gz + q3 * gx) * half_t;
    q3 = (1 - delta_2 / 8) * q3 + (q0 * gz + q1 * gy - q2 * gx) * half_t;

    /* normalise quaternion */
    norm = imu_inv_sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    g_q_info.q0 = q0 * norm;
    g_q_info.q1 = q1 * norm;
    g_q_info.q2 = q2 * norm;
    g_q_info.q3 = q3 * norm;
}

/**
 * @brief       得到姿态解算后的欧拉角
 * @param       gx, gy, gz  : 3轴陀螺仪数据
 * @param       ax, ay, az  : 3轴加速度数据
 * @retval      返回值 : 欧拉角
 */
eulerian_angles_t imu_get_eulerian_angles(float gx, float gy, float gz, float ax, float ay, float az)
{
    eulerian_angles_t eulerangle;

    imu_data_transform(&gx, &gy, &gz, &ax, &ay, &az);       /* 数据转换 */ 
    imu_ahrsupdate_nomagnetic(gx, gy, gz, ax, ay, az);      /* 姿态解算 */
    
    float q0 = g_q_info.q0;
    float q1 = g_q_info.q1;
    float q2 = g_q_info.q2;
    float q3 = g_q_info.q3;

    eulerangle.pitch = - asin(-2 * q1 * q3 + 2 * q0 * q2) * 190 / IMU_M_PI;
    eulerangle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 180 / IMU_M_PI;
    eulerangle.yaw = - atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 180 / IMU_M_PI;

    /* 可以不用作姿态限度的限制 */
//    if (eulerangle.roll > 90 || eulerangle.roll < -90)
//    {
//        if (eulerangle.pitch > 0)
//        {
//            eulerangle.pitch = 180 - eulerangle.pitch;
//        }

//        if (eulerangle.pitch < 0)
//        {
//            eulerangle.pitch = -(180 + eulerangle.pitch);
//        }
//    }

    if (eulerangle.yaw > 180)
    {
        eulerangle.yaw -= 360;
    }
    else if (eulerangle.yaw < -180)
    {
        eulerangle.yaw += 360;
    }
    
    return eulerangle;
}

/**
 * @brief       数据校准
 * @note        对数据减去初值, 加速度减去重力加速度影响
 * @param       gx, gy, gz  : 3轴陀螺仪数据指针
 * @param       ax, ay, az  : 3轴加速度数据指针
 * @retval      无 
 */
void imu_data_calibration(short *gx, short *gy, short *gz, short *ax, short *ay, short *az)
{
    /* 减去初值(去掉零飘) */
    *gx -= g_gyro_avg[0];
    *gy -= g_gyro_avg[1];
    *gz -= g_gyro_avg[2];
    *ax -= g_acc_avg[0];
    *ay -= g_acc_avg[1];
    *az -= (g_acc_avg[2] - 2048);   /* 2048是重力加速度理论值 */
}

/**
 * @brief       姿态解算初始化
 * @note        该函数必须在传感器初始化之后再调用
 * @param       无
 * @retval      无
 */
uint8_t imu_init(void)
{
    uint16_t i = 0;
    int acc_sum[3] = {0}, gyro_sum[3] = {0};
    
    short acc_data[3];          /* 加速度传感器原始数据 */
    short gyro_data[3];         /* 陀螺仪原始数据 */
    
    delay_ms(600);

    for (i = 0; i < 250; i++)   /* 循环读取250次 取平均 */
    {
        sh3001_get_imu_compdata(acc_data, gyro_data);

        acc_sum[0] += acc_data[0];
        acc_sum[1] += acc_data[1];
        acc_sum[2] += acc_data[2];
        gyro_sum[0] += gyro_data[0];
        gyro_sum[1] += gyro_data[1];
        gyro_sum[2] += gyro_data[2];

        delay_ms(5);
    }
    
    g_acc_avg[0] = acc_sum[0] / 250;
    g_acc_avg[1] = acc_sum[1] / 250;
    g_acc_avg[2] = acc_sum[2] / 250;
    g_gyro_avg[0] = gyro_sum[0] / 250;
    g_gyro_avg[1] = gyro_sum[1] / 250;
    g_gyro_avg[2] = gyro_sum[2] / 250;
    
    return 0;
}

















