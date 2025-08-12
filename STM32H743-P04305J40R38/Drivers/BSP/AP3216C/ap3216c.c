/**
 ****************************************************************************************************
 * @file        ap3216c.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       AP3216C 驱动代码
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
 * V1.0 20230322
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/AP3216C/ap3216c.h"
#include "./BSP/IIC/myiic.h"
#include "./SYSTEM/delay/delay.h"


 /**
 * @brief       初始化AP3216C
 * @param       无
 * @retval      0, 成功;
                1, 失败;
 */
uint8_t ap3216c_init(void)
{
    uint8_t temp = 0;

    iic_init();                          /* 初始化IIC */

    ap3216c_write_one_byte(0x00, 0X04);  /* 复位AP3216C */
    delay_ms(50);                        /* AP33216C复位至少10ms */
    ap3216c_write_one_byte(0x00, 0X03);  /* 开启ALS、PS+IR    */
    temp = ap3216c_read_one_byte(0X00);  /* 读取刚刚写进去的0X03 */

    if (temp == 0X03)return 0;           /* AP3216C正常 */
    else return 1;                       /* AP3216C失败 */
}

/**
 * @brief       读取AP3216C的数据
 *  @note       读取原始数据，包括ALS,PS和IR
 *              如果同时打开ALS,IR+PS的话两次数据读取的时间间隔要大于112.5ms
 * @param       ir      : IR传感器值
 * @param       ps      : PS传感器值
 * @param       als     : ALS传感器值
 * @retval      无
 */
void ap3216c_read_data(uint16_t* ir, uint16_t* ps, uint16_t* als)
{
    uint8_t buf[6];
    uint8_t i;

    for (i = 0;i < 6;i++)
    {
        buf[i] = ap3216c_read_one_byte(0X0A + i);                   /* 循环读取所有传感器数据 */
    }

    if (buf[0] & 0X80) * ir = 0;                                    /* IR_OF位为1,则数据无效 */
    else *ir = ((uint16_t)buf[1] << 2) | (buf[0] & 0X03);           /* 读取IR传感器的数据   */
    *als = ((uint16_t)buf[3] << 8) | buf[2];                        /* 读取ALS传感器的数据   */ 
    if (buf[4] & 0x40) * ps = 0;                                    /* IR_OF位为1,则数据无效 */
    else *ps = ((uint16_t)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);  /* 读取PS传感器的数据 */
}

/**
 * @brief       AP3216C写入一个字节
 * @param       reg     : 寄存器地址
 * @param       data    : 要写入的数据
 * @retval      0, 成功;
                1, 失败;
 */
uint8_t ap3216c_write_one_byte(uint8_t reg, uint8_t data)
{
    iic_start();
    iic_send_byte(AP3216C_ADDR | 0X00); /* 发送器件地址+写命令 */

    if (iic_wait_ack())                 /* 等待应答 */
    {
        iic_stop();
        return 1;
    }

    iic_send_byte(reg);                 /* 写寄存器地址 */
    iic_wait_ack();                     /* 等待应答 */
    iic_send_byte(data);                /* 发送数据 */

    if (iic_wait_ack())                 /* 等待ACK */
    {
        iic_stop();
        return 1;
    }

    iic_stop();
    return 0;
}

/**
 * @brief       AP3216C读取一个字节
 * @param       reg     : 寄存器地址
 * @retval      读到的数据
 */
uint8_t ap3216c_read_one_byte(uint8_t reg)
{
    uint8_t res;

    iic_start();
    iic_send_byte(AP3216C_ADDR | 0X00); /* 发送器件地址+写命令 */
    iic_wait_ack();                     /* 等待应答 */
    iic_send_byte(reg);                 /* 写寄存器地址 */
    iic_wait_ack();                     /* 等待应答 */
    iic_start();                
    iic_send_byte(AP3216C_ADDR | 0X01); /* 发送器件地址+读命令 */
    iic_wait_ack();                     /* 等待应答 */
    res = iic_read_byte(0);             /* 读数据,发送nACK   */
    iic_stop();                         /* 产生一个停止条件 */

    return res;  
}














