/**
 ****************************************************************************************************
 * @file        sh3001.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-07-19
 * @brief       SH3001 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
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
 
#include "./BSP/IIC/myiic.h"
#include "./BSP/SH3001/sh3001.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"

static compCoefType g_compcoef;         /* compCoefType结构体,用于存储补偿系数 */
static unsigned char g_store_acc_odr;   /* 保存ACC ODR临时变量 */


/**
 * @brief       从SH3001读取N字节数据
 * @note        SH3001的命令发送,也是用该函数实现(不带参数的命令, 也会有一个状态寄存器需要读取)
 * @param       devaddr  : 寄存器地址
 * @param       regaddr  : 寄存器命令
 * @param       length   : 读取长度
 * @param       readbuf  : 数据存储buf
 * @retval      0, 操作成功
 *              其他, 操作失败
 */
unsigned char sh3001_read_nbytes(unsigned char devaddr, unsigned char regaddr, unsigned char length, unsigned char *readbuf)
{
    unsigned char i;

    iic_start();
    iic_send_byte(devaddr << 1 | 0X00);

    if (iic_wait_ack())
    {
        iic_stop();
        return 1;
    }

    iic_send_byte(regaddr);             /* 写地址/命令 */
    iic_wait_ack();

    iic_start();
    iic_send_byte((devaddr << 1) + 1);
    iic_wait_ack();

    for (i = 0; i < length; i++)        /* 循环读取 数据 */
    {
        readbuf[i] = iic_read_byte(1);
    }

    iic_read_byte(0);
    iic_stop();
    return (SH3001_TRUE);
}

/**
 * @brief       SH3001写入N字节数据
 * @param       devaddr  : 寄存器地址
 * @param       regaddr  : 寄存器命令
 * @param       length   : 写入长度
 * @param       writebuf : 数据存储buf
 * @retval      0, 操作成功
 *              其他, 操作失败
 */
unsigned char sh3001_write_nbytes(unsigned char devaddr, unsigned char regaddr, unsigned char length, unsigned char *writebuf)
{
    unsigned char i;

    iic_start();
    iic_send_byte(devaddr << 1 | 0X00);
    iic_wait_ack();

    iic_send_byte(regaddr);
    iic_wait_ack();

    for (i = 0; i < length; i++)
    {
        iic_send_byte(writebuf[i]);

        if (iic_wait_ack())
        {
            iic_stop();
            return (SH3001_FALSE);
        }
    }

    iic_stop();
    return (SH3001_TRUE);
}

/**
 * @brief       设置加速度计参数
 * @note        acc_cutofffreq = acc_odr * 0.40 or acc_odr * 0.25 or acc_odr * 0.11 or acc_odr * 0.04;
 * @param       acc_odr              acc_range                acc_cutofffreq       acc_filter_enble
 *              SH3001_ODR_1000HZ   SH3001_ACC_RANGE_16G    SH3001_ACC_ODRX040  SH3001_ACC_FILTER_EN
 *              SH3001_ODR_500HZ    SH3001_ACC_RANGE_8G     SH3001_ACC_ODRX025  SH3001_ACC_FILTER_DIS
 *              SH3001_ODR_250HZ    SH3001_ACC_RANGE_4G     SH3001_ACC_ODRX011
 *              SH3001_ODR_125HZ    SH3001_ACC_RANGE_2G     SH3001_ACC_ODRX004
 *              SH3001_ODR_63HZ
 *              SH3001_ODR_31HZ
 *              SH3001_ODR_16HZ
 *              SH3001_ODR_2000HZ
 *              SH3001_ODR_4000HZ
 *              SH3001_ODR_8000HZ
 * @retval      无
 */
static void sh3001_acc_config(uint8_t acc_odr, uint8_t acc_range, uint8_t acc_cutofffreq, uint8_t acc_filter_enble)
{
    unsigned char regdata = 0;

    /* 使能加速度计数字滤波器 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF0, 1, &regdata);
    regdata |= 0x01;
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF0, 1, &regdata);

    /* 设置采样频率 */
    g_store_acc_odr = acc_odr;
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &acc_odr);

    /* 设置加速度计满量程范围 */
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF2, 1, &acc_range);

    /* 设置低通滤波器 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF3, 1, &regdata);
    regdata &= 0x17;
    regdata |= (acc_cutofffreq | acc_filter_enble);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF3, 1, &regdata);
}


/**
 * @brief       设置陀螺仪参数
 * @note        gyro_cutofffreq = gyro_odr * 0.40 or gyro_odr * 0.25 or gyro_odr * 0.11 or gyro_odr * 0.04;
 * @param       gyro_odr             gyro_range_x,y,z          gyro_cutofffreq      gyro_filter_enble
 *              SH3001_ODR_1000HZ   SH3001_GYRO_RANGE_125   SH3001_GYRO_ODRX00  SH3001_GYRO_FILTER_EN
 *              SH3001_ODR_500HZ    SH3001_GYRO_RANGE_250   SH3001_GYRO_ODRX01  SH3001_GYRO_FILTER_DIS
 *              SH3001_ODR_250HZ    SH3001_GYRO_RANGE_500   SH3001_GYRO_ODRX02
 *              SH3001_ODR_125HZ    SH3001_GYRO_RANGE_1000  SH3001_GYRO_ODRX03
 *              SH3001_ODR_63HZ     SH3001_GYRO_RANGE_2000
 *              SH3001_ODR_31HZ
 *              SH3001_ODR_16HZ
 *              SH3001_ODR_2000HZ
 *              SH3001_ODR_4000HZ
 *              SH3001_ODR_8000HZ
 *              SH3001_ODR_16000HZ
 *              SH3001_ODR_32000HZ
 * @retval      无
 */
static void sh3001_gyro_config(uint8_t gyro_odr, uint8_t gyro_range_x, uint8_t gyro_range_y, uint8_t gyro_range_z, uint8_t gyro_cutofffreq, uint8_t gyro_filter_enble)
{
    unsigned char regdata = 0;

    /* 使能陀螺仪数字滤波器 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF0, 1, &regdata);
    regdata |= 0x01;
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF0, 1, &regdata);

    /* 设置采样频率 */
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF1, 1, &gyro_odr);

    /* 设置满量程范围 */
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF3, 1, &gyro_range_x);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF4, 1, &gyro_range_y);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF5, 1, &gyro_range_z);

    /* 设置低通滤波器 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF2, 1, &regdata);
    regdata &= 0xE3;
    regdata |= (gyro_cutofffreq | gyro_filter_enble);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF2, 1, &regdata);
}

/**
 * @brief       设置温度参数
 * @param       temp_odr
 *                  SH3001_TEMP_ODR_500
 *                  SH3001_TEMP_ODR_250
 *                  SH3001_TEMP_ODR_125
 *                  SH3001_TEMP_ODR_63
 * @param       temp_enable
 *                  SH3001_TEMP_EN
 *                  SH3001_TEMP_DIS
 * @retval      无
 */
static void sh3001_temp_config(unsigned char temp_odr, unsigned char temp_enable)
{
    unsigned char regdata = 0;

    /* 设置温度传感器参数 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_TEMP_CONF0, 1, &regdata);
    regdata &= 0x4F;
    regdata |= (temp_odr | temp_enable);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_TEMP_CONF0, 1, &regdata);
}

/**
 * @brief       读取补偿系数
 * @param       g_compcoef    : compCoefType结构体指针
 * @retval      无
 */
static void sh3001_comp_init(compCoefType *g_compcoef)
{
    unsigned char coefdata[2] = {0};

    /* Acc Cross */
    sh3001_read_nbytes(SH3001_ADDRESS, 0x81, 2, coefdata);
    g_compcoef->cYX = (signed char)coefdata[0];
    g_compcoef->cZX = (signed char)coefdata[1];
    sh3001_read_nbytes(SH3001_ADDRESS, 0x91, 2, coefdata);
    g_compcoef->cXY = (signed char)coefdata[0];
    g_compcoef->cZY = (signed char)coefdata[1];
    sh3001_read_nbytes(SH3001_ADDRESS, 0xA1, 2, coefdata);
    g_compcoef->cXZ = (signed char)coefdata[0];
    g_compcoef->cYZ = (signed char)coefdata[1];

    /* Gyro Zero */
    sh3001_read_nbytes(SH3001_ADDRESS, 0x60, 1, coefdata);
    g_compcoef->jX = (signed char)coefdata[0];
    sh3001_read_nbytes(SH3001_ADDRESS, 0x68, 1, coefdata);
    g_compcoef->jY = (signed char)coefdata[0];
    sh3001_read_nbytes(SH3001_ADDRESS, 0x70, 1, coefdata);
    g_compcoef->jZ = (signed char)coefdata[0];

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF3, 1, coefdata);
    coefdata[0] = coefdata[0] & 0x07;
    g_compcoef->xMulti = ((coefdata[0] < 2) || (coefdata[0] >= 7)) ? 1 : (1 << (6 - coefdata[0]));
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF4, 1, coefdata);
    coefdata[0] = coefdata[0] & 0x07;
    g_compcoef->yMulti = ((coefdata[0] < 2) || (coefdata[0] >= 7)) ? 1 : (1 << (6 - coefdata[0]));
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF5, 1, coefdata);
    coefdata[0] = coefdata[0] & 0x07;
    g_compcoef->zMulti = ((coefdata[0] < 2) || (coefdata[0] >= 7)) ? 1 : (1 << (6 - coefdata[0]));

    sh3001_read_nbytes(SH3001_ADDRESS, 0x2E, 1, coefdata);
    g_compcoef->paramP0 = coefdata[0] & 0x1F;
}

/**
 * @brief       复位部分内部模块
 * @param       无
 * @retval      无
 */
static void sh3001_module_reset(void)
{
    const unsigned char regaddr[8] = {0xC0, 0xD3, 0xD3, 0xD5, 0xD4, 0xBB, 0xB9, 0xBA};

    /* MCC版本配置 */
    const unsigned char mcc_regdata_a[8] = {0x38, 0xC6, 0xC1, 0x02, 0x0C, 0x18, 0x18, 0x18};
    const unsigned char mcc_regdata_b[8] = {0x3D, 0xC2, 0xC2, 0x00, 0x04, 0x00, 0x00, 0x00};

    /* MCD版本配置 */
    const unsigned char mcd_regdata_a[8] = {0x38, 0xD6, 0xD1, 0x02, 0x08, 0x18, 0x18, 0x18};
    const unsigned char mcd_regdata_b[8] = {0x3D, 0xD2, 0xD2, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* MCF版本配置 */
    const unsigned char mcf_regdata_a[8] = {0x38, 0x16, 0x11, 0x02, 0x08, 0x18, 0x18, 0x18};
    const unsigned char mcf_regdata_b[8] = {0x3E, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00};

    unsigned char regdata = 0;
    unsigned char *regdata_a = (unsigned char *)mcd_regdata_a;    /* 默认用MCD配置 */
    unsigned char *regdata_b = (unsigned char *)mcd_regdata_b;

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_CHIP_VERSION, 1, &regdata);   /* 读取 SH3001_CHIP_VERSION */

    if (regdata == SH3001_CHIP_VERSION_MCC)         /* MCC版本 */
    {
        regdata_a = (unsigned char *)mcc_regdata_a;
        regdata_b = (unsigned char *)mcc_regdata_b;

    }
    else if (regdata == SH3001_CHIP_VERSION_MCF)     /* MCF版本 */
    {
        regdata_a = (unsigned char *)mcf_regdata_a;
        regdata_b = (unsigned char *)mcf_regdata_b;
    }

    /* Drive Start */
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[0], 1, &regdata_a[0]);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[1], 1, &regdata_a[1]);
    delay_ms(100);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[0], 1, &regdata_b[0]);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[1], 1, &regdata_b[1]);
    delay_ms(50);

    /* ADC Resett */
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[2], 1, &regdata_a[2]);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[3], 1, &regdata_a[3]);
    delay_ms(1);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[2], 1, &regdata_b[2]);
    delay_ms(1);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[3], 1, &regdata_b[3]);
    delay_ms(50);

    /* CVA Resett */
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[4], 1, &regdata_a[4]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[4], 1, &regdata_b[4]);

    delay_ms(1);

    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[5], 1, &regdata_a[5]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[6], 1, &regdata_a[6]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[7], 1, &regdata_a[7]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[5], 1, &regdata_b[5]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[6], 1, &regdata_b[6]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regaddr[7], 1, &regdata_b[7]);
    delay_ms(10);

    /* 设置 INT and INT1 脚开漏输出 */
    regdata = 0x00;
    sh3001_write_nbytes(SH3001_ADDRESS, 0x44, 1, &regdata);
}

/**
 * @brief       切换SH3001电源模式
 *              Normal mode: 1.65mA; Sleep mode: 162uA; Acc normal mode:393uA;
 * @param       powermode
 *                  SH3001_NORMAL_MODE
 *                  SH3001_SLEEP_MODE
 *                  SH3001_POWERDOWN_MODE
 *                  SH3001_ACC_NORMAL_MODE
 * @retval      SH3001_TRUE 成功
 *              SH3001_FALSE 异常
 */
unsigned char sh3001_switch_powermode(unsigned char powermode)
{
    unsigned char regaddr[10] = {0xCF, 0x22, 0x2F, 0xCB, 0xCE, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7};
    unsigned char regdata[10] = {0};
    unsigned char i = 0;
    unsigned char acc_odr = SH3001_ODR_1000HZ;

    if ((powermode != SH3001_NORMAL_MODE)
            && (powermode != SH3001_SLEEP_MODE)
            && (powermode != SH3001_POWERDOWN_MODE)
            && (powermode != SH3001_ACC_NORMAL_MODE))
    {
        return (SH3001_FALSE);
    }


    for (i = 0; i < 10; i++)
    {
        sh3001_read_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
    }

    switch (powermode)
    {
        case SH3001_NORMAL_MODE:
            /* restore accODR */
            sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &g_store_acc_odr);

            regdata[0] = (regdata[0] & 0xF8);
            regdata[1] = (regdata[1] & 0x7F);
            regdata[2] = (regdata[2] & 0xF7);
            regdata[3] = (regdata[3] & 0xF7);
            regdata[4] = (regdata[4] & 0xFE);
            regdata[5] = (regdata[5] & 0xFC) | 0x02;
            regdata[6] = (regdata[6] & 0x9F);
            regdata[7] = (regdata[7] & 0xF9);

            for (i = 0; i < 8; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
            }

            regdata[7] = (regdata[7] & 0x87);
            regdata[8] = (regdata[8] & 0x1F);
            regdata[9] = (regdata[9] & 0x03);

            for (i = 7; i < 10; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
            }

            sh3001_module_reset();
            return (SH3001_TRUE);

        case SH3001_SLEEP_MODE:
            /* store current acc ODR */
            sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &g_store_acc_odr);
            /* 设置acc ODR=1000Hz */
            sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &acc_odr);

            regdata[0] = (regdata[0] & 0xF8) | 0x07;
            regdata[1] = (regdata[1] & 0x7F) | 0x80;
            regdata[2] = (regdata[2] & 0xF7) | 0x08;
            regdata[3] = (regdata[3] & 0xF7) | 0x08;
            regdata[4] = (regdata[4] & 0xFE);
            regdata[5] = (regdata[5] & 0xFC) | 0x01;
            regdata[6] = (regdata[6] & 0x9F);
            regdata[7] = (regdata[7] & 0xF9) | 0x06;

            for (i = 0; i < 8; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
            }

            regdata[7] = (regdata[7] & 0x87);
            regdata[8] = (regdata[8] & 0x1F);
            regdata[9] = (regdata[9] & 0x03);

            for (i = 7; i < 10; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
            }

            return (SH3001_TRUE);

        case SH3001_POWERDOWN_MODE:
            regdata[0] = (regdata[0] & 0xF8);
            regdata[1] = (regdata[1] & 0x7F) | 0x80;
            regdata[2] = (regdata[2] & 0xF7) | 0x08;
            regdata[3] = (regdata[3] & 0xF7) | 0x08;
            regdata[4] = (regdata[4] & 0xFE);
            regdata[5] = (regdata[5] & 0xFC) | 0x01;
            regdata[6] = (regdata[6] & 0x9F) | 0x60;
            regdata[7] = (regdata[7] & 0xF9) | 0x06;

            for (i = 0; i < 8; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
            }

            regdata[7] = (regdata[7] & 0x87);
            regdata[8] = (regdata[8] & 0x1F);
            regdata[9] = (regdata[9] & 0x03);

            for (i = 7; i < 10; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
            }

            return (SH3001_TRUE);

        case SH3001_ACC_NORMAL_MODE:
            regdata[0] = (regdata[0] & 0xF8);
            regdata[1] = (regdata[1] & 0x7F);
            regdata[2] = (regdata[2] & 0xF7);
            regdata[3] = (regdata[3] & 0xF7);
            regdata[4] = (regdata[4] | 0x01);
            regdata[5] = (regdata[5] & 0xFC) | 0x01;
            regdata[6] = (regdata[6] & 0x9F);
            regdata[7] = (regdata[7] & 0xF9) | 0x06;

            for (i = 0; i < 8; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
            }

            regdata[7] = (regdata[7] & 0x87) | 0x78;
            regdata[8] = (regdata[8] & 0x1F) | 0xE0;
            regdata[9] = (regdata[9] & 0x03) | 0xFC;

            for (i = 7; i < 10; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regaddr[i], 1, &regdata[i]);
            }

            return (SH3001_TRUE);

        default:
            return (SH3001_FALSE);
    }
}



/**
 * @brief       初始化SH3001接口
 * @param       无
 * @retval      SH3001_TRUE, 成功
 *              SH3001_FALSE, 异常
 */
unsigned char sh3001_init(void)
{
    unsigned char i = 0;
    unsigned char regdata = 0;

    iic_init(); /* 初始化IIC */
    
    /* 读取CHIP ID */
    do
    {
        sh3001_read_nbytes(SH3001_ADDRESS, SH3001_CHIP_ID, 1, &regdata);
    }while ((regdata != 0x61) && (i++ < 3));

    if ((regdata != 0x61))
    {
        printf("SH3001 CHIP ID:0X%X\r\n", regdata); /* 打印错误的ID */
        return SH3001_FALSE;
    }

    sh3001_module_reset();                  /* 重置内部模块 */
    
    /* ACC配置: 500Hz, 16G, cut off Freq(BW)=500*0.25Hz=125Hz, enable filter; */
    sh3001_acc_config(SH3001_ODR_500HZ, 
                      SH3001_ACC_RANGE_16G, 
                      SH3001_ACC_ODRX025,
                      SH3001_ACC_FILTER_EN);

    /* GYRO配置: 500Hz, X\Y\Z 2000deg/s, cut off Freq(BW)=181Hz, enable filter; */
    sh3001_gyro_config( SH3001_ODR_500HZ,
                        SH3001_GYRO_RANGE_2000,
                        SH3001_GYRO_RANGE_2000,
                        SH3001_GYRO_RANGE_2000,
                        SH3001_GYRO_ODRX00,
                        SH3001_GYRO_FILTER_EN);
    
    /* 温度配置: 输出速率63Hz, 使能温度测量 */
    sh3001_temp_config(SH3001_TEMP_ODR_63, SH3001_TEMP_EN);

    /* SH3001进入正常工作模式 */
    sh3001_switch_powermode(SH3001_NORMAL_MODE);

    /* 读取补偿系数 */
    sh3001_comp_init(&g_compcoef);

    return SH3001_TRUE;
}

/**
 * @brief       读温度值
 * @param       无
 * @retval      温度值,单位为℃(float类型)
 */
float sh3001_get_tempdata(void)
{
    unsigned char regdata[2] = {0};
    unsigned short int tempref[2] = {0};

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_TEMP_CONF0, 2, &regdata[0]);
    tempref[0] = ((unsigned short int)(regdata[0] & 0x0F) << 8) | regdata[1];

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_TEMP_ZL, 2, &regdata[0]);
    tempref[1] = ((unsigned short int)(regdata[1] & 0x0F) << 8) | regdata[0];

    return ( (((float)(tempref[1] - tempref[0])) / 16.0f) + 25.0f );
}

/**
 * @brief       读取SH3001陀螺仪和加速度的数据(未作补偿!不推荐,仅供调试)
 * @param       accdata[3]  : acc X,Y,Z;
 * @param       gyrodata[3] : gyro X,Y,Z;
 * @retval      无
 */
void sh3001_get_imu_data(short accdata[3], short gyrodata[3])
{
    unsigned char regdata[12] = {0};

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_XL, 12, regdata);
    accdata[0] = ((short)regdata[1] << 8) | regdata[0];
    accdata[1] = ((short)regdata[3] << 8) | regdata[2];
    accdata[2] = ((short)regdata[5] << 8) | regdata[4];

    gyrodata[0] = ((short)regdata[7] << 8) | regdata[6];
    gyrodata[1] = ((short)regdata[9] << 8) | regdata[8];
    gyrodata[2] = ((short)regdata[11] << 8) | regdata[10];

    //printf("%d %d %d %d %d %d\r\n", accdata[0], accdata[1], accdata[2], gyrodata[0], gyrodata[1], gyrodata[2]);
}

/**
 * @brief       读取补偿后SH3001陀螺仪和加速度的数据(推荐使用)
 * @param       accdata[3]  : acc X,Y,Z;
 * @param       gyrodata[3] : gyro X,Y,Z;
 * @retval      无
 */
void sh3001_get_imu_compdata(short accdata[3], short gyrodata[3])
{
    unsigned char regdata[15] = {0};
    unsigned char paramp;
    int acctemp[3], gyrotemp[3];

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_XL, 15, regdata);
    accdata[0] = ((short)regdata[1] << 8) | regdata[0];
    accdata[1] = ((short)regdata[3] << 8) | regdata[2];
    accdata[2] = ((short)regdata[5] << 8) | regdata[4];
    gyrodata[0] = ((short)regdata[7] << 8) | regdata[6];
    gyrodata[1] = ((short)regdata[9] << 8) | regdata[8];
    gyrodata[2] = ((short)regdata[11] << 8) | regdata[10];
    paramp = regdata[14] & 0x1F;
    
    acctemp[0] = (int)( accdata[0] + \
                        accdata[1] * ((float)g_compcoef.cXY / 1024.0f) + \
                        accdata[2] * ((float)g_compcoef.cXZ / 1024.0f) );

    acctemp[1] = (int)( accdata[0] * ((float)g_compcoef.cYX / 1024.0f) + \
                        accdata[1] + \
                        accdata[2] * ((float)g_compcoef.cYZ / 1024.0f) );

    acctemp[2] = (int)( accdata[0] * ((float)g_compcoef.cZX / 1024.0f) + \
                        accdata[1] * ((float)g_compcoef.cZY / 1024.0f) + \
                        accdata[2] );

    if (acctemp[0] > 32767)
    {
        acctemp[0] = 32767;
    }
    else if (acctemp[0] < -32768)
    {
        acctemp[0] = -32768;
    }

    if (acctemp[1] > 32767)
    {
        acctemp[1] = 32767;
    }
    else if (acctemp[1] < -32768)
    {
        acctemp[1] = -32768;
    }

    if (acctemp[2] > 32767)
    {
        acctemp[2] = 32767;
    }
    else if (acctemp[2] < -32768)
    {
        acctemp[2] = -32768;
    }

    accdata[0] = (short)acctemp[0];
    accdata[1] = (short)acctemp[1];
    accdata[2] = (short)acctemp[2];

    gyrotemp[0] = gyrodata[0] - (paramp - g_compcoef.paramP0) * g_compcoef.jX * g_compcoef.xMulti;
    gyrotemp[1] = gyrodata[1] - (paramp - g_compcoef.paramP0) * g_compcoef.jY * g_compcoef.yMulti;
    gyrotemp[2] = gyrodata[2] - (paramp - g_compcoef.paramP0) * g_compcoef.jZ * g_compcoef.zMulti;

    if (gyrotemp[0] > 32767)
    {
        gyrotemp[0] = 32767;
    }
    else if (gyrotemp[0] < -32768)
    {
        gyrotemp[0] = -32768;
    }

    if (gyrotemp[1] > 32767)
    {
        gyrotemp[1] = 32767;
    }
    else if (gyrotemp[1] < -32768)
    {
        gyrotemp[1] = -32768;
    }

    if (gyrotemp[2] > 32767)
    {
        gyrotemp[2] = 32767;
    }
    else if (gyrotemp[2] < -32768)
    {
        gyrotemp[2] = -32768;
    }

    gyrodata[0] = (short)gyrotemp[0];
    gyrodata[1] = (short)gyrotemp[1];
    gyrodata[2] = (short)gyrotemp[2];

    //printf("%d %d %d %d %d %d\r\n", accdata[0], accdata[1], accdata[2], gyrodata[0], gyrodata[1], gyrodata[2]);
}



