/**
 ****************************************************************************************************
 * @file        es8388.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-14
 * @brief       ES8388 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220114
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/ES8388/es8388.h"
#include "./BSP/IIC/myiic.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/24CXX/24cxx.h"
#include "settings.h"
#include "os.h"


/* ES8388默认参数 */
_es8388_obj es8388set =
{
    50,             /* 音量:50 */
    0, 0, 0, 0, 0,  /* 频率标志设为0 */
    0, 0, 0, 0, 0,  /* 中心频率增益表全0 */
    0,              /* 3D效果为0 */
    1,              /* 开启板载喇叭 */
    0,
};

/**
 * @brief       读取ES8388数据
 *  note        ES8388数据保存在:SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings)之后
 * @param       es8388dev       : ES8388数据结构体
 * @retval      无
 */
void es8388_read_para(_es8388_obj *es8388dev)
{
    at24cxx_read(SYSTEM_PARA_SAVE_BASE + sizeof(_system_setings), (uint8_t *)es8388dev, sizeof(_es8388_obj));
}

/**
 * @brief       写入ES8388数据
 *  note        ES8388数据保存在:SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings)之后
 * @param       es8388dev       : ES8388数据结构体
 * @retval      无
 */
void es8388_save_para(_es8388_obj *es8388dev)
{
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();    /* 进入临界区(无法被中断打断) */
    at24cxx_write(SYSTEM_PARA_SAVE_BASE + sizeof(_system_setings), (uint8_t *)es8388dev, sizeof(_es8388_obj));
    OS_EXIT_CRITICAL();     /* 退出临界区(可以被中断打断) */
}

/**
 * @brief       ES8388初始化
 * @param       无
 * @retval      0,初始化正常
 *              其他,错误代码
 */
uint8_t es8388_init(void)
{
    iic_init();                     /* 初始化IIC接口 */

    es8388_write_reg(0, 0x80);      /* 软复位ES8388 */
    es8388_write_reg(0, 0x00);
    delay_ms(100);                  /* 等待复位 */

    es8388_write_reg(0x01, 0x58);
    es8388_write_reg(0x01, 0x50);
    es8388_write_reg(0x02, 0xF3);
    es8388_write_reg(0x02, 0xF0);

    es8388_write_reg(0x03, 0x09);   /* 麦克风偏置电源关闭 */
    es8388_write_reg(0x00, 0x06);   /* 使能参考		500K驱动使能 */
    es8388_write_reg(0x04, 0x00);   /* DAC电源管理，不打开任何通道 */
    es8388_write_reg(0x08, 0x00);   /* MCLK不分频 */
    es8388_write_reg(0x2B, 0x80);   /* DAC控制	DACLRC与ADCLRC相同 */

    es8388_write_reg(0x09, 0x88);   /* ADC L/R PGA增益配置为+24dB */
    es8388_write_reg(0x0C, 0x4C);   /* ADC	数据选择为left data = left ADC, right data = left ADC  音频数据为16bit */
    es8388_write_reg(0x0D, 0x02);   /* ADC配置 MCLK/采样率=256 */
    es8388_write_reg(0x10, 0x00);   /* ADC数字音量控制将信号衰减 L  设置为最小！！！ */
    es8388_write_reg(0x11, 0x00);   /* ADC数字音量控制将信号衰减 R  设置为最小！！！ */

    es8388_write_reg(0x17, 0x18);   /* DAC 音频数据为16bit */
    es8388_write_reg(0x18, 0x02);   /* DAC	配置 MCLK/采样率=256 */
    es8388_write_reg(0x1A, 0x00);   /* DAC数字音量控制将信号衰减 L  设置为最小！！！ */
    es8388_write_reg(0x1B, 0x00);   /* DAC数字音量控制将信号衰减 R  设置为最小！！！ */
    es8388_write_reg(0x27, 0xB8);   /* L混频器 */
    es8388_write_reg(0x2A, 0xB8);   /* R混频器 */
    delay_ms(100);

    return 0;
}

/**
 * @brief       ES8388写寄存器
 * @param       reg : 寄存器地址
 * @param       val : 要写入寄存器的值
 * @retval      0,成功
 *              其他,错误代码
 */
uint8_t es8388_write_reg(uint8_t reg, uint8_t val)
{
    iic_start();

    iic_send_byte((ES8388_ADDR << 1) | 0);  /* 发送器件地址+写命令 */

    if (iic_wait_ack())
    {
        return 1;                           /* 等待应答(成功?/失败?) */
    }

    iic_send_byte(reg);                     /* 写寄存器地址 */

    if (iic_wait_ack())
    {
        return 2;                           /* 等待应答(成功?/失败?) */
    }

    iic_send_byte(val & 0xFF);              /* 发送数据 */

    if (iic_wait_ack())
    {
        return 3;                           /* 等待应答(成功?/失败?) */
    }

    iic_stop();

    return 0;
}

/**
 * @brief       ES8388读寄存器
 * @param       reg : 寄存器地址
 * @retval      读取到的数据
 */
uint8_t es8388_read_reg(uint8_t reg)
{
    uint8_t temp = 0;

    iic_start();

    iic_send_byte((ES8388_ADDR << 1) | 0);  /* 发送器件地址+写命令 */

    if (iic_wait_ack())
    {
        return 1;                           /*等待应答(成功?/失败?) */
    }

    iic_send_byte(reg);                     /* 写寄存器地址 */

    if (iic_wait_ack())
    {
        return 1;                           /* 等待应答(成功?/失败?) */
    }

    iic_start();
    iic_send_byte((ES8388_ADDR << 1) | 1);  /* 发送器件地址+读命令 */

    if (iic_wait_ack())
    {
        return 1;                           /* 等待应答(成功?/失败?) */
    }

    temp = iic_read_byte(0);

    iic_stop();

    return temp;
}

/**
 * @brief       设置ES8388工作模式
 * @param       fmt : 工作模式
 *    @arg      0, 飞利浦标准I2S;
 *    @arg      1, MSB(左对齐);
 *    @arg      2, LSB(右对齐);
 *    @arg      3, PCM/DSP
 * @param       len : 数据长度
 *    @arg      0, 24bit
 *    @arg      1, 20bit
 *    @arg      2, 18bit
 *    @arg      3, 16bit
 *    @arg      4, 32bit
 * @retval      无
 */
void es8388_sai_cfg(uint8_t fmt, uint8_t len)
{
    fmt &= 0x03;
    len &= 0x07;    /* 限定范围 */
    es8388_write_reg(23, (fmt << 1) | (len << 3));  /* R23,ES8388工作模式设置 */
}

/**
 * @brief       设置耳机音量
 * @param       voluem : 音量大小(0 ~ 33)
 * @retval      无
 */
void es8388_hpvol_set(uint8_t volume)
{
    if (volume > 33)
    {
        volume = 33;
    }
    es8388_write_reg(0x2E, volume);
    es8388_write_reg(0x2F, volume);
}

/**
 * @brief       设置喇叭音量
 * @param       volume : 音量大小(0 ~ 33)
 * @retval      无
 */
void es8388_spkvol_set(uint8_t volume)
{
    if (volume > 33)
    {
        volume = 33;
    }
    es8388_write_reg(0x30, volume);
    es8388_write_reg(0x31, volume);
}

/**
 * @brief       设置3D环绕声
 * @param       depth : 0 ~ 7(3D强度,0关闭,7最强)
 * @retval      无
 */
void es8388_3d_set(uint8_t depth)
{
    depth &= 0x7;       /* 限定范围 */
    es8388_write_reg(0x1D, depth << 2);    /* R7,3D环绕设置 */
}

/**
 * @brief       ES8388 DAC/ADC配置
 * @param       dacen : dac使能(1) / 关闭(0)
 * @param       adcen : adc使能(1) / 关闭(0)
 * @retval      无
 */
void es8388_adda_cfg(uint8_t dacen, uint8_t adcen)
{
    uint8_t tempreg = 0;

    tempreg |= !dacen << 0;
    tempreg |= !adcen << 1;
    tempreg |= !dacen << 2;
    tempreg |= !adcen << 3;
    es8388_write_reg(0x02, tempreg);
}

/**
 * @brief       ES8388 DAC输出通道配置
 * @param       o1en : 通道1使能(1)/禁止(0)
 * @param       o2en : 通道2使能(1)/禁止(0)
 * @retval      无
 */
void es8388_output_cfg(uint8_t o1en, uint8_t o2en)
{
    uint8_t tempreg = 0;
    tempreg |= o1en * (3 << 4);
    tempreg |= o2en * (3 << 2);
    es8388_write_reg(0x04, tempreg);
}

/**
 * @brief       ES8388 MIC增益设置(MIC PGA增益)
 * @param       gain : 0~8, 对应0~24dB  3dB/Step
 * @retval      无
 */
void es8388_mic_gain(uint8_t gain)
{
    gain &= 0x0F;
    gain |= gain << 4;
    es8388_write_reg(0x09, gain);       /* R9,左右通道PGA增益设置 */
}

/**
 * @brief       ES8388 ALC设置
 * @param       sel
 *   @arg       0,关闭ALC
 *   @arg       1,右通道ALC
 *   @arg       2,左通道ALC
 *   @arg       3,立体声ALC
 * @param       maxgain : 0~7,对应-6.5~+35.5dB
 * @param       minigain: 0~7,对应-12~+30dB 6dB/STEP
 * @retval      无
 */
void es8388_alc_ctrl(uint8_t sel, uint8_t maxgain, uint8_t mingain)
{
    uint8_t tempreg = 0;
    tempreg = sel << 6;
    tempreg |= (maxgain & 0x07) << 3;
    tempreg |= mingain & 0x07;
    es8388_write_reg(0x12, tempreg);     /* R18,ALC设置 */
}

/**
 * @brief       ES8388 ADC输出通道配置
 * @param       in : 输入通道
 *    @arg      0, 通道1输入
 *    @arg      1, 通道2输入
 * @retval      无
 */
void es8388_input_cfg(uint8_t in)
{
    es8388_write_reg(0x0A, (5 * in) << 4);   /* ADC1 输入通道选择L/R	INPUT1 */
}

/**
 * @brief       设置EQ1
 * @param       cfreq:截止频率,0~3,分别对应:80/105/135/175Hz
 * @param       gain:增益,0~24,对应-12~+12dB
 * @retval      无
 */
void es8388_eq1_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval;
    cfreq &= 0X3;                   /* 限定范围 */
    
    if(gain > 24) gain = 24;
    
    gain = 24 - gain;
    regval = es8388_read_reg(18);
    regval &= 0X100;
    regval |= cfreq<<5;             /* 设置截止频率 */
    regval |= gain;                 /* 设置增益 */
    es8388_write_reg(18,regval);    /* R18,EQ1设置  */
}

/**
 * @brief       设置EQ2
 * @param       cfreq:截止频率,0~3,分别对应:230/300/385/500Hz
 * @param       gain:增益,0~24,对应-12~+12dB
 * @retval      无
 */
void es8388_eq2_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval=0;
    cfreq &= 0X3;                   /* 限定范围 */
    
    if (gain > 24) gain = 24;
    
    gain = 24 - gain; 
    regval |= cfreq << 5;           /* 设置截止频率 */
    regval |= gain;                 /* 设置增益 */
    es8388_write_reg(19,regval);    /* R19,EQ2设置 */
}

/**
 * @brief       设置EQ3
 * @param       cfreq:截止频率,0~3,分别对应:650/850/1100/1400Hz
 * @param       gain:增益,0~24,对应-12~+12dB
 * @retval      无
 */
void es8388_eq3_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval=0;
    cfreq &= 0X3;                   /* 限定范围 */
    
    if (gain > 24) gain = 24;
    
    gain = 24 - gain; 
    regval |= cfreq << 5;           /* 设置截止频率 */
    regval |= gain;                 /* 设置增益 */
    es8388_write_reg(20,regval);    /* R20,EQ3设置 */
}

/**
 * @brief       设置EQ4
 * @param       cfreq:截止频率,0~3,分别对应:800/2400/3200/4100Hz
 * @param       gain:增益,0~24,对应-12~+12dB
 * @retval      无
 */
void es8388_eq4_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval = 0;
    cfreq &= 0X3;                   //限定范围 */
    
    if (gain > 24) gain=24;
    
    gain = 24 - gain; 
    regval |= cfreq << 5;           /* 设置截止频率 */
    regval |= gain;                 /* 设置增益 */
    es8388_write_reg(21,regval);    /* R21,EQ4设置 */
}

/**
 * @brief       设置EQ5
 * @param       cfreq:截止频率,0~3,分别对应:5300/6900/9000/11700Hz
 * @param       gain:增益,0~24,对应-12~+12dB
 * @retval      无
 */
void es8388_eq5_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval=0;
    cfreq &= 0X3;                   /* 限定范围 */
    
    if (gain > 24) gain = 24;
    
    gain = 24 - gain; 
    regval |= cfreq << 5;           /* 设置截止频率 */
    regval |= gain;                 /* 设置增益 */
    es8388_write_reg(22,regval);    /* R22,EQ5设置 */
}

