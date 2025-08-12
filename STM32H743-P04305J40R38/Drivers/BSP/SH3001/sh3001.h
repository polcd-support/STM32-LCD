/**
 ****************************************************************************************************
 * @file        sh3001.h
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
#ifndef __SH3001_H 
#define __SH3001_H

#include "./SYSTEM/sys/sys.h"


/* SH3001器件地址 */
#define SH3001_ADDRESS              (0x36)


/******************************************************************
*    SH3001 Registers Macro Definitions 
******************************************************************/
#define SH3001_ACC_XL                (0x00)
#define SH3001_ACC_XH                (0x01)
#define SH3001_ACC_YL                (0x02)
#define SH3001_ACC_YH                (0x03)
#define SH3001_ACC_ZL                (0x04)
#define SH3001_ACC_ZH                (0x05)
#define SH3001_GYRO_XL               (0x06)
#define SH3001_GYRO_XH               (0x07)
#define SH3001_GYRO_YL               (0x08)
#define SH3001_GYRO_YH               (0x09)
#define SH3001_GYRO_ZL               (0x0A)
#define SH3001_GYRO_ZH               (0x0B)
#define SH3001_CHIP_ID               (0x0F)
#define SH3001_TEMP_ZL               (0x0C)
#define SH3001_TEMP_CONF0            (0x20)
#define SH3001_TEMP_CONF1            (0x21)

#define SH3001_ACC_CONF0             (0x22)        // accelerometer config 0x22-0x26
#define SH3001_ACC_CONF1             (0x23)
#define SH3001_ACC_CONF2             (0x25)
#define SH3001_ACC_CONF3             (0x26)

#define SH3001_GYRO_CONF0            (0x28)        // gyroscope config 0x28-0x2B
#define SH3001_GYRO_CONF1            (0x29)
#define SH3001_GYRO_CONF2            (0x2B)


#define SH3001_GYRO_CONF3           (0x8F)
#define SH3001_GYRO_CONF4           (0x9F)
#define SH3001_GYRO_CONF5           (0xAF)
#define SH3001_CHIP_VERSION         (0xDD)
#define SH3001_CHIP_ID1             (0xDF)
#define SH3001_AUX_I2C_CONF         (0xFD)


/******************************************************************
*    ACC Config Macro Definitions 
******************************************************************/
#define SH3001_ODR_1000HZ           (0x00)
#define SH3001_ODR_500HZ            (0x01)
#define SH3001_ODR_250HZ            (0x02)
#define SH3001_ODR_125HZ            (0x03)
#define SH3001_ODR_63HZ             (0x04)
#define SH3001_ODR_31HZ             (0x05)
#define SH3001_ODR_16HZ             (0x06)
#define SH3001_ODR_2000HZ           (0x08)
#define SH3001_ODR_4000HZ           (0x09)
#define SH3001_ODR_8000HZ           (0x0A)
#define SH3001_ODR_16000HZ          (0x0B)
#define SH3001_ODR_32000HZ          (0x0C)

#define SH3001_ACC_RANGE_16G        (0x02)
#define SH3001_ACC_RANGE_8G         (0x03)
#define SH3001_ACC_RANGE_4G         (0x04)
#define SH3001_ACC_RANGE_2G         (0x05)

#define SH3001_ACC_ODRX040          (0x00)
#define SH3001_ACC_ODRX025          (0x20)
#define SH3001_ACC_ODRX011          (0x40)
#define SH3001_ACC_ODRX004          (0x60)

#define SH3001_ACC_FILTER_EN        (0x00)
#define SH3001_ACC_FILTER_DIS       (0x08)

/******************************************************************
*    GYRO Config Macro Definitions 
******************************************************************/
#define SH3001_GYRO_RANGE_125       (0x02)
#define SH3001_GYRO_RANGE_250       (0x03)
#define SH3001_GYRO_RANGE_500       (0x04)
#define SH3001_GYRO_RANGE_1000      (0x05)
#define SH3001_GYRO_RANGE_2000      (0x06)

#define SH3001_GYRO_ODRX00          (0x00)
#define SH3001_GYRO_ODRX01          (0x04)
#define SH3001_GYRO_ODRX02          (0x08)
#define SH3001_GYRO_ODRX03          (0x0C)

#define SH3001_GYRO_FILTER_EN       (0x00)
#define SH3001_GYRO_FILTER_DIS      (0x10)


/******************************************************************
*    Temperature Config Macro Definitions 
******************************************************************/
#define SH3001_TEMP_ODR_500         (0x00)
#define SH3001_TEMP_ODR_250         (0x10)
#define SH3001_TEMP_ODR_125         (0x20)
#define SH3001_TEMP_ODR_63          (0x30)

#define SH3001_TEMP_EN              (0x80)
#define SH3001_TEMP_DIS             (0x00)

/******************************************************************
*    INT Config Macro Definitions 
******************************************************************/
#define SH3001_INT_LOWG             (0x8000)
#define SH3001_INT_HIGHG            (0x4000)
#define SH3001_INT_INACT            (0x2000)
#define SH3001_INT_ACT              (0x1000)
#define SH3001_INT_DOUBLE_TAP       (0x0800)
#define SH3001_INT_SINGLE_TAP       (0x0400)
#define SH3001_INT_FLAT             (0x0200)
#define SH3001_INT_ORIENTATION      (0x0100)
#define SH3001_INT_FIFO_GYRO        (0x0010)
#define SH3001_INT_GYRO_READY       (0x0008)
#define SH3001_INT_ACC_FIFO         (0x0004)
#define SH3001_INT_ACC_READY        (0x0002)
#define SH3001_INT_FREE_FALL        (0x0001)
#define SH3001_INT_UP_DOWN_Z        (0x0040)


#define SH3001_INT_ENABLE             (0x01)
#define SH3001_INT_DISABLE            (0x00)

#define SH3001_INT_MAP_INT1           (0x01)
#define SH3001_INT_MAP_INT            (0x00)

#define SH3001_INT0_LEVEL_LOW         (0x80)
#define SH3001_INT0_LEVEL_HIGH        (0x7F)
#define SH3001_INT_NO_LATCH           (0x40)
#define SH3001_INT_LATCH              (0xBF)
#define SH3001_INT1_LEVEL_LOW         (0x20)
#define SH3001_INT1_LEVEL_HIGH        (0xDF)
#define SH3001_INT_CLEAR_ANY          (0x10)
#define SH3001_INT_CLEAR_STATUS       (0xEF)
#define SH3001_INT1_NORMAL            (0x04)
#define SH3001_INT1_OD                (0xFB)
#define SH3001_INT0_NORMAL            (0x01)
#define SH3001_INT0_OD                (0xFE)

/******************************************************************
*    Orientation Blocking Config Macro Definitions 
******************************************************************/
#define SH3001_ORIENT_BLOCK_MODE0    (0x00)
#define SH3001_ORIENT_BLOCK_MODE1    (0x04)
#define SH3001_ORIENT_BLOCK_MODE2    (0x08)
#define SH3001_ORIENT_BLOCK_MODE3    (0x0C)

#define SH3001_ORIENT_SYMM           (0x00)
#define SH3001_ORIENT_HIGH_ASYMM     (0x01)
#define SH3001_ORIENT_LOW_ASYMM      (0x02)


/******************************************************************
*    Flat Time Config Macro Definitions 
******************************************************************/
#define SH3001_FLAT_TIME_500MS         (0x40)
#define SH3001_FLAT_TIME_1000MS        (0x80)
#define SH3001_FLAT_TIME_2000MS        (0xC0)


/******************************************************************
*    ACT and INACT Int Config Macro Definitions 
******************************************************************/
#define SH3001_ACT_AC_MODE        (0x80)
#define SH3001_ACT_DC_MODE        (0x00)
#define SH3001_ACT_X_INT_EN       (0x40)
#define SH3001_ACT_X_INT_DIS      (0x00)
#define SH3001_ACT_Y_INT_EN       (0x20)
#define SH3001_ACT_Y_INT_DIS      (0x00)
#define SH3001_ACT_Z_INT_EN       (0x10)
#define SH3001_ACT_Z_INT_DIS      (0x00)


#define SH3001_INACT_AC_MODE      (0x08)
#define SH3001_INACT_DC_MODE      (0x00)
#define SH3001_INACT_X_INT_EN     (0x04)
#define SH3001_INACT_X_INT_DIS    (0x00)
#define SH3001_INACT_Y_INT_EN     (0x02)
#define SH3001_INACT_Y_INT_DIS    (0x00)
#define SH3001_INACT_Z_INT_EN     (0x01)
#define SH3001_INACT_Z_INT_DIS    (0x00)


#define SH3001_LINK_PRE_STA       (0x01)
#define SH3001_LINK_PRE_STA_NO    (0x00)


/******************************************************************
*    TAP Int Config Macro Definitions 
******************************************************************/
#define SH3001_TAP_X_INT_EN        (0x08)
#define SH3001_TAP_X_INT_DIS       (0x00)
#define SH3001_TAP_Y_INT_EN        (0x04)
#define SH3001_TAP_Y_INT_DIS       (0x00)
#define SH3001_TAP_Z_INT_EN        (0x02)
#define SH3001_TAP_Z_INT_DIS       (0x00)


/******************************************************************
*    HIGHG Int Config Macro Definitions 
******************************************************************/
#define SH3001_HIGHG_ALL_INT_EN     (0x80)
#define SH3001_HIGHG_ALL_INT_DIS    (0x00)
#define SH3001_HIGHG_X_INT_EN       (0x40)
#define SH3001_HIGHG_X_INT_DIS      (0x00)
#define SH3001_HIGHG_Y_INT_EN       (0x20)
#define SH3001_HIGHG_Y_INT_DIS      (0x00)
#define SH3001_HIGHG_Z_INT_EN       (0x10)
#define SH3001_HIGHG_Z_INT_DIS      (0x00)


/******************************************************************
*    LOWG Int Config Macro Definitions 
******************************************************************/
#define SH3001_LOWG_ALL_INT_EN         (0x01)
#define SH3001_LOWG_ALL_INT_DIS        (0x00)



/******************************************************************
*    FIFO Config Macro Definitions 
******************************************************************/
#define SH3001_FIFO_MODE_DIS         (0x00)
#define SH3001_FIFO_MODE_FIFO        (0x01)
#define SH3001_FIFO_MODE_STREAM      (0x02)
#define SH3001_FIFO_MODE_TRIGGER     (0x03)

#define SH3001_FIFO_ACC_DOWNS_EN     (0x80)
#define SH3001_FIFO_ACC_DOWNS_DIS    (0x00)
#define SH3001_FIFO_GYRO_DOWNS_EN    (0x08)
#define SH3001_FIFO_GYRO_DOWNS_DIS   (0x00)

#define SH3001_FIFO_FREQ_X1_2        (0x00)
#define SH3001_FIFO_FREQ_X1_4        (0x01)
#define SH3001_FIFO_FREQ_X1_8        (0x02)
#define SH3001_FIFO_FREQ_X1_16       (0x03)
#define SH3001_FIFO_FREQ_X1_32       (0x04)
#define SH3001_FIFO_FREQ_X1_64       (0x05)
#define SH3001_FIFO_FREQ_X1_128      (0x06)
#define SH3001_FIFO_FREQ_X1_256      (0x07)

#define SH3001_FIFO_EXT_Z_EN        (0x2000)
#define SH3001_FIFO_EXT_Y_EN        (0x1000)
#define SH3001_FIFO_EXT_X_EN        (0x0080)
#define SH3001_FIFO_TEMPERATURE_EN  (0x0040)
#define SH3001_FIFO_GYRO_Z_EN       (0x0020)
#define SH3001_FIFO_GYRO_Y_EN       (0x0010)
#define SH3001_FIFO_GYRO_X_EN       (0x0008)
#define SH3001_FIFO_ACC_Z_EN        (0x0004)
#define SH3001_FIFO_ACC_Y_EN        (0x0002)
#define SH3001_FIFO_ACC_X_EN        (0x0001)
#define SH3001_FIFO_ALL_DIS         (0x0000)

/******************************************************************
*    AUX I2C Config Macro Definitions 
******************************************************************/
#define SH3001_MI2C_NORMAL_MODE           (0x00)
#define SH3001_MI2C_BYPASS_MODE           (0x01)

#define SH3001_MI2C_READ_ODR_200HZ        (0x00)
#define SH3001_MI2C_READ_ODR_100HZ        (0x10)
#define SH3001_MI2C_READ_ODR_50HZ         (0x20)
#define SH3001_MI2C_READ_ODR_25HZ         (0x30)

#define SH3001_MI2C_FAIL                  (0x20)
#define SH3001_MI2C_SUCCESS               (0x10)

#define SH3001_MI2C_READ_MODE_AUTO        (0x40)
#define SH3001_MI2C_READ_MODE_MANUAL      (0x00)



/******************************************************************
*    Other Macro Definitions 
******************************************************************/
#define SH3001_TRUE        (0)
#define SH3001_FALSE       (1)


#define SH3001_NORMAL_MODE       (0x00)
#define SH3001_SLEEP_MODE        (0x01)
#define SH3001_POWERDOWN_MODE    (0x02)
#define SH3001_ACC_NORMAL_MODE   (0x03)

#define SH3001_CHIP_VERSION_MCC     (0x08)
#define SH3001_CHIP_VERSION_MCD     (0x10)
#define SH3001_CHIP_VERSION_MCF     (0x20)
/***************************************************************************
*       Type Definitions                                              
****************************************************************************/

typedef struct
{
    signed char cXY;
    signed char cXZ;
    signed char cYX;
    signed char cYZ;
    signed char cZX;
    signed char cZY;
    signed char jX;
    signed char jY;
    signed char jZ;
    unsigned char xMulti;
    unsigned char yMulti;
    unsigned char zMulti;
    unsigned char paramP0;    
} compCoefType;    

/***************************************************************************
        Exported Functions                                           
****************************************************************************/

unsigned char sh3001_read_nbytes(unsigned char devaddr, unsigned char regaddr, unsigned char length, unsigned char *readbuf);
unsigned char sh3001_write_nbytes(unsigned char devaddr, unsigned char regaddr, unsigned char length, unsigned char *writebuf);
unsigned char sh3001_switch_powermode(unsigned char powermode);

unsigned char sh3001_init(void);
float sh3001_get_tempdata(void);
void sh3001_get_imu_data(short accdata[3], short gyrodata[3]);
void sh3001_get_imu_compdata(short accdata[3], short gyrodata[3]);

#endif
