/**
 ****************************************************************************************************
 * @file        es8388.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-14
 * @brief       ES8388 ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220114
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/ES8388/es8388.h"
#include "./BSP/IIC/myiic.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/24CXX/24cxx.h"
#include "settings.h"
#include "os.h"


/* ES8388Ĭ�ϲ��� */
_es8388_obj es8388set =
{
    50,             /* ����:50 */
    0, 0, 0, 0, 0,  /* Ƶ�ʱ�־��Ϊ0 */
    0, 0, 0, 0, 0,  /* ����Ƶ�������ȫ0 */
    0,              /* 3DЧ��Ϊ0 */
    1,              /* ������������ */
    0,
};

/**
 * @brief       ��ȡES8388����
 *  note        ES8388���ݱ�����:SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings)֮��
 * @param       es8388dev       : ES8388���ݽṹ��
 * @retval      ��
 */
void es8388_read_para(_es8388_obj *es8388dev)
{
    at24cxx_read(SYSTEM_PARA_SAVE_BASE + sizeof(_system_setings), (uint8_t *)es8388dev, sizeof(_es8388_obj));
}

/**
 * @brief       д��ES8388����
 *  note        ES8388���ݱ�����:SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings)֮��
 * @param       es8388dev       : ES8388���ݽṹ��
 * @retval      ��
 */
void es8388_save_para(_es8388_obj *es8388dev)
{
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();    /* �����ٽ���(�޷����жϴ��) */
    at24cxx_write(SYSTEM_PARA_SAVE_BASE + sizeof(_system_setings), (uint8_t *)es8388dev, sizeof(_es8388_obj));
    OS_EXIT_CRITICAL();     /* �˳��ٽ���(���Ա��жϴ��) */
}

/**
 * @brief       ES8388��ʼ��
 * @param       ��
 * @retval      0,��ʼ������
 *              ����,�������
 */
uint8_t es8388_init(void)
{
    iic_init();                     /* ��ʼ��IIC�ӿ� */

    es8388_write_reg(0, 0x80);      /* ��λES8388 */
    es8388_write_reg(0, 0x00);
    delay_ms(100);                  /* �ȴ���λ */

    es8388_write_reg(0x01, 0x58);
    es8388_write_reg(0x01, 0x50);
    es8388_write_reg(0x02, 0xF3);
    es8388_write_reg(0x02, 0xF0);

    es8388_write_reg(0x03, 0x09);   /* ��˷�ƫ�õ�Դ�ر� */
    es8388_write_reg(0x00, 0x06);   /* ʹ�ܲο�		500K����ʹ�� */
    es8388_write_reg(0x04, 0x00);   /* DAC��Դ���������κ�ͨ�� */
    es8388_write_reg(0x08, 0x00);   /* MCLK����Ƶ */
    es8388_write_reg(0x2B, 0x80);   /* DAC����	DACLRC��ADCLRC��ͬ */

    es8388_write_reg(0x09, 0x88);   /* ADC L/R PGA��������Ϊ+24dB */
    es8388_write_reg(0x0C, 0x4C);   /* ADC	����ѡ��Ϊleft data = left ADC, right data = left ADC  ��Ƶ����Ϊ16bit */
    es8388_write_reg(0x0D, 0x02);   /* ADC���� MCLK/������=256 */
    es8388_write_reg(0x10, 0x00);   /* ADC�����������ƽ��ź�˥�� L  ����Ϊ��С������ */
    es8388_write_reg(0x11, 0x00);   /* ADC�����������ƽ��ź�˥�� R  ����Ϊ��С������ */

    es8388_write_reg(0x17, 0x18);   /* DAC ��Ƶ����Ϊ16bit */
    es8388_write_reg(0x18, 0x02);   /* DAC	���� MCLK/������=256 */
    es8388_write_reg(0x1A, 0x00);   /* DAC�����������ƽ��ź�˥�� L  ����Ϊ��С������ */
    es8388_write_reg(0x1B, 0x00);   /* DAC�����������ƽ��ź�˥�� R  ����Ϊ��С������ */
    es8388_write_reg(0x27, 0xB8);   /* L��Ƶ�� */
    es8388_write_reg(0x2A, 0xB8);   /* R��Ƶ�� */
    delay_ms(100);

    return 0;
}

/**
 * @brief       ES8388д�Ĵ���
 * @param       reg : �Ĵ�����ַ
 * @param       val : Ҫд��Ĵ�����ֵ
 * @retval      0,�ɹ�
 *              ����,�������
 */
uint8_t es8388_write_reg(uint8_t reg, uint8_t val)
{
    iic_start();

    iic_send_byte((ES8388_ADDR << 1) | 0);  /* ����������ַ+д���� */

    if (iic_wait_ack())
    {
        return 1;                           /* �ȴ�Ӧ��(�ɹ�?/ʧ��?) */
    }

    iic_send_byte(reg);                     /* д�Ĵ�����ַ */

    if (iic_wait_ack())
    {
        return 2;                           /* �ȴ�Ӧ��(�ɹ�?/ʧ��?) */
    }

    iic_send_byte(val & 0xFF);              /* �������� */

    if (iic_wait_ack())
    {
        return 3;                           /* �ȴ�Ӧ��(�ɹ�?/ʧ��?) */
    }

    iic_stop();

    return 0;
}

/**
 * @brief       ES8388���Ĵ���
 * @param       reg : �Ĵ�����ַ
 * @retval      ��ȡ��������
 */
uint8_t es8388_read_reg(uint8_t reg)
{
    uint8_t temp = 0;

    iic_start();

    iic_send_byte((ES8388_ADDR << 1) | 0);  /* ����������ַ+д���� */

    if (iic_wait_ack())
    {
        return 1;                           /*�ȴ�Ӧ��(�ɹ�?/ʧ��?) */
    }

    iic_send_byte(reg);                     /* д�Ĵ�����ַ */

    if (iic_wait_ack())
    {
        return 1;                           /* �ȴ�Ӧ��(�ɹ�?/ʧ��?) */
    }

    iic_start();
    iic_send_byte((ES8388_ADDR << 1) | 1);  /* ����������ַ+������ */

    if (iic_wait_ack())
    {
        return 1;                           /* �ȴ�Ӧ��(�ɹ�?/ʧ��?) */
    }

    temp = iic_read_byte(0);

    iic_stop();

    return temp;
}

/**
 * @brief       ����ES8388����ģʽ
 * @param       fmt : ����ģʽ
 *    @arg      0, �����ֱ�׼I2S;
 *    @arg      1, MSB(�����);
 *    @arg      2, LSB(�Ҷ���);
 *    @arg      3, PCM/DSP
 * @param       len : ���ݳ���
 *    @arg      0, 24bit
 *    @arg      1, 20bit
 *    @arg      2, 18bit
 *    @arg      3, 16bit
 *    @arg      4, 32bit
 * @retval      ��
 */
void es8388_sai_cfg(uint8_t fmt, uint8_t len)
{
    fmt &= 0x03;
    len &= 0x07;    /* �޶���Χ */
    es8388_write_reg(23, (fmt << 1) | (len << 3));  /* R23,ES8388����ģʽ���� */
}

/**
 * @brief       ���ö�������
 * @param       voluem : ������С(0 ~ 33)
 * @retval      ��
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
 * @brief       ������������
 * @param       volume : ������С(0 ~ 33)
 * @retval      ��
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
 * @brief       ����3D������
 * @param       depth : 0 ~ 7(3Dǿ��,0�ر�,7��ǿ)
 * @retval      ��
 */
void es8388_3d_set(uint8_t depth)
{
    depth &= 0x7;       /* �޶���Χ */
    es8388_write_reg(0x1D, depth << 2);    /* R7,3D�������� */
}

/**
 * @brief       ES8388 DAC/ADC����
 * @param       dacen : dacʹ��(1) / �ر�(0)
 * @param       adcen : adcʹ��(1) / �ر�(0)
 * @retval      ��
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
 * @brief       ES8388 DAC���ͨ������
 * @param       o1en : ͨ��1ʹ��(1)/��ֹ(0)
 * @param       o2en : ͨ��2ʹ��(1)/��ֹ(0)
 * @retval      ��
 */
void es8388_output_cfg(uint8_t o1en, uint8_t o2en)
{
    uint8_t tempreg = 0;
    tempreg |= o1en * (3 << 4);
    tempreg |= o2en * (3 << 2);
    es8388_write_reg(0x04, tempreg);
}

/**
 * @brief       ES8388 MIC��������(MIC PGA����)
 * @param       gain : 0~8, ��Ӧ0~24dB  3dB/Step
 * @retval      ��
 */
void es8388_mic_gain(uint8_t gain)
{
    gain &= 0x0F;
    gain |= gain << 4;
    es8388_write_reg(0x09, gain);       /* R9,����ͨ��PGA�������� */
}

/**
 * @brief       ES8388 ALC����
 * @param       sel
 *   @arg       0,�ر�ALC
 *   @arg       1,��ͨ��ALC
 *   @arg       2,��ͨ��ALC
 *   @arg       3,������ALC
 * @param       maxgain : 0~7,��Ӧ-6.5~+35.5dB
 * @param       minigain: 0~7,��Ӧ-12~+30dB 6dB/STEP
 * @retval      ��
 */
void es8388_alc_ctrl(uint8_t sel, uint8_t maxgain, uint8_t mingain)
{
    uint8_t tempreg = 0;
    tempreg = sel << 6;
    tempreg |= (maxgain & 0x07) << 3;
    tempreg |= mingain & 0x07;
    es8388_write_reg(0x12, tempreg);     /* R18,ALC���� */
}

/**
 * @brief       ES8388 ADC���ͨ������
 * @param       in : ����ͨ��
 *    @arg      0, ͨ��1����
 *    @arg      1, ͨ��2����
 * @retval      ��
 */
void es8388_input_cfg(uint8_t in)
{
    es8388_write_reg(0x0A, (5 * in) << 4);   /* ADC1 ����ͨ��ѡ��L/R	INPUT1 */
}

/**
 * @brief       ����EQ1
 * @param       cfreq:��ֹƵ��,0~3,�ֱ��Ӧ:80/105/135/175Hz
 * @param       gain:����,0~24,��Ӧ-12~+12dB
 * @retval      ��
 */
void es8388_eq1_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval;
    cfreq &= 0X3;                   /* �޶���Χ */
    
    if(gain > 24) gain = 24;
    
    gain = 24 - gain;
    regval = es8388_read_reg(18);
    regval &= 0X100;
    regval |= cfreq<<5;             /* ���ý�ֹƵ�� */
    regval |= gain;                 /* �������� */
    es8388_write_reg(18,regval);    /* R18,EQ1����  */
}

/**
 * @brief       ����EQ2
 * @param       cfreq:��ֹƵ��,0~3,�ֱ��Ӧ:230/300/385/500Hz
 * @param       gain:����,0~24,��Ӧ-12~+12dB
 * @retval      ��
 */
void es8388_eq2_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval=0;
    cfreq &= 0X3;                   /* �޶���Χ */
    
    if (gain > 24) gain = 24;
    
    gain = 24 - gain; 
    regval |= cfreq << 5;           /* ���ý�ֹƵ�� */
    regval |= gain;                 /* �������� */
    es8388_write_reg(19,regval);    /* R19,EQ2���� */
}

/**
 * @brief       ����EQ3
 * @param       cfreq:��ֹƵ��,0~3,�ֱ��Ӧ:650/850/1100/1400Hz
 * @param       gain:����,0~24,��Ӧ-12~+12dB
 * @retval      ��
 */
void es8388_eq3_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval=0;
    cfreq &= 0X3;                   /* �޶���Χ */
    
    if (gain > 24) gain = 24;
    
    gain = 24 - gain; 
    regval |= cfreq << 5;           /* ���ý�ֹƵ�� */
    regval |= gain;                 /* �������� */
    es8388_write_reg(20,regval);    /* R20,EQ3���� */
}

/**
 * @brief       ����EQ4
 * @param       cfreq:��ֹƵ��,0~3,�ֱ��Ӧ:800/2400/3200/4100Hz
 * @param       gain:����,0~24,��Ӧ-12~+12dB
 * @retval      ��
 */
void es8388_eq4_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval = 0;
    cfreq &= 0X3;                   //�޶���Χ */
    
    if (gain > 24) gain=24;
    
    gain = 24 - gain; 
    regval |= cfreq << 5;           /* ���ý�ֹƵ�� */
    regval |= gain;                 /* �������� */
    es8388_write_reg(21,regval);    /* R21,EQ4���� */
}

/**
 * @brief       ����EQ5
 * @param       cfreq:��ֹƵ��,0~3,�ֱ��Ӧ:5300/6900/9000/11700Hz
 * @param       gain:����,0~24,��Ӧ-12~+12dB
 * @retval      ��
 */
void es8388_eq5_eet(uint8_t cfreq,uint8_t gain)
{ 
    uint16_t regval=0;
    cfreq &= 0X3;                   /* �޶���Χ */
    
    if (gain > 24) gain = 24;
    
    gain = 24 - gain; 
    regval |= cfreq << 5;           /* ���ý�ֹƵ�� */
    regval |= gain;                 /* �������� */
    es8388_write_reg(22,regval);    /* R22,EQ5���� */
}

