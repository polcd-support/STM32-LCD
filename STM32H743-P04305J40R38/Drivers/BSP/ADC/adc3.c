/**
 ****************************************************************************************************
 * @file        adc3.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       ADC3(�����ڲ��¶ȴ�����) ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230322
 * ��һ�η���
 ****************************************************************************************************
 */

#include "./BSP/ADC/adc3.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       ADC3��ʼ������
 * @note        ������ר����֧��ADC3, ��ADC1/2���ֿ���, ����ʹ��
 *              ����ʹ��16λ����, ADC����ʱ��=32M, ת��ʱ��Ϊ:�������� + 8.5��ADC����
 *              ��������������: 810.5, ��ת��ʱ�� = 819��ADC���� = 25.6us
 * @param       ��
 * @retval      ��
 */
void adc3_init(void)
{
    RCC->AHB4ENR |= 1 << 24;        /* ʹ��ADC3ʱ�� */

    RCC->AHB4RSTR |= 1 << 24;       /* ADC3��λ */
    RCC->AHB4RSTR &= ~(1 << 24);    /* ��λ���� */

    RCC->D3CCIPR &= ~(3 << 16);     /* ADCSEL[1:0]���� */
    RCC->D3CCIPR |= 2 << 16;        /* ADCSEL[1:0]=2,per_ck��ΪADCʱ��Դ,Ĭ��ѡ��hsi_ker_ck��Ϊper_ck,Ƶ��:64Mhz */
    ADC3_COMMON->CCR |= 0 << 18;    /* PRESC[3:0]=0,����ʱ�Ӳ���Ƶ,��adc_ker_ck=per_ck=64Mhz,ADC����ʱ��=����ʱ��/2=32M */

    ADC3_COMMON->CCR |= 1 << 23;    /* VSENSEEN=1,ʹ���ڲ��¶ȴ�����ͨ�� */

    ADC3->CR = 0;                   /* CR�Ĵ�������,DEEPPWD����,�����˯�߻���. */
    ADC3->CR |= 1 << 28;            /* ADVREGEN=1,ʹ��ADC��ѹ�� */

    delay_ms(10);                   /* �ȴ���ѹ���������,Լ10us,������ʱ��һ��,û��ϵ. */

    ADC3->CR |= 3 << 8;             /* BOOST[1:0]=11,ADC������boostģʽ3(25M��ADCƵ�ʡ�50M) */
    ADC3->CFGR &= ~(1 << 13);       /* CONT=0,����ת��ģʽ */
    ADC3->CFGR |= 1 << 12;          /* OVRMOD=1,��дģʽ(DR�Ĵ����ɱ���д) */
    ADC3->CFGR &= ~(3 << 10);       /* EXTEN[1:0]=0,������� */
    ADC3->CFGR &= ~(7 << 2);        /* RES[2:0]λ���� */
    ADC3->CFGR |= 0 << 2;           /* RES[2:0]=0,16λ�ֱ���(0,16λ;1,14λ;2,12λ;3,10λ;4,8λ.) */

    ADC3->CFGR2 &= ~((uint32_t)15 << 28);   /* LSHIFT[3:0]=0,������,�����Ҷ���. */
    ADC3->CFGR2 &= ~((uint32_t)0X3FF << 16);/* OSR[9:0]=0,��ʹ�ù����� */


    ADC3->CR &= ~((uint32_t)1 << 30);/* ADCALDIF=0,У׼����ת��ͨ�� */
    ADC3->CR |= 1 << 16;            /* ADCALLIN=1,����У׼ */
    ADC3->CR |= (uint32_t)1 << 31;  /* ����У׼ */

    while (ADC3->CR & ((uint32_t)1 << 31)); /* �ȴ�У׼��� */

    ADC3->SQR1 &= ~(0XF << 0);      /* L[3:0]���� */
    ADC3->SQR1 |= 0 << 0;           /* L[3:0]=0,1��ת���ڹ��������� Ҳ����ֻת����������1 */

    ADC3->CR |= 1 << 0; /* ����ADת���� */
}

/**
 * @brief       ����ADC3ͨ������ʱ��
 * @param       ch   : ͨ����, 0~19
 * @param       stime: ����ʱ��  0~7, ��Ӧ��ϵΪ:
 *   @arg       000, 1.5��ADCʱ������        001, 2.5��ADCʱ������
 *   @arg       010, 8.5��ADCʱ������        011, 16.5��ADCʱ������
 *   @arg       100, 32.5��ADCʱ������       101, 64.5��ADCʱ������
 *   @arg       110, 387.5��ADCʱ������      111, 810.5��ADCʱ������
 * @retval      ��
 */
void adc3_channel_set(uint8_t ch, uint8_t stime)
{
    if (ch < 10)              /* ͨ��0~9,ʹ��SMPR1���� */
    {
        ADC3->SMPR1 &= ~(7 << (3 * ch));        /* ͨ��ch ����ʱ����� */
        ADC3->SMPR1 |= 7 << (3 * ch);           /* ͨ��ch ������������,����Խ�߾���Խ�� */
    }
    else     /* ͨ��10~19,ʹ��SMPR2���� */
    {
        ADC3->SMPR2 &= ~(7 << (3 * (ch - 10))); /* ͨ��ch ����ʱ����� */
        ADC3->SMPR2 |= 7 << (3 * (ch - 10));    /* ͨ��ch ������������,����Խ�߾���Խ�� */
    }
}

/**
 * @brief       ���ADCת����Ľ��
 * @param       ch: ͨ����, 0~19
 * @retval      ��
 */
uint32_t adc3_get_result(uint8_t ch)
{
    adc3_channel_set(ch, 7);        /* ����ADCX��Ӧͨ������ʱ��Ϊ810.5��ʱ������ */

    ADC3->PCSEL |= 1 << ch;         /* ADCת��ͨ��Ԥѡ�� */
    /* ����ת������ */
    ADC3->SQR1 &= ~(0X1F << 6 * 1); /* ��������1ͨ������ */
    ADC3->SQR1 |= ch << 6 * 1;      /* ���ù�������1��ת��ͨ��Ϊch */
    ADC3->CR |= 1 << 2;             /* ��������ת��ͨ�� */

    while (!(ADC3->ISR & 1 << 2));  /* �ȴ�ת������ */

    return ADC3->DR;                /* ����adcֵ */
}

/**
 * @brief       ��ȡͨ��ch��ת��ֵ��ȡtimes��,Ȼ��ƽ��
 * @param       ch      : ͨ����, 0~19
 * @param       times   : ��ȡ����
 * @retval      ͨ��ch��times��ת�����ƽ��ֵ
 */
uint32_t adc3_get_result_average(uint8_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++)   /* ��ȡtimes������ */
    {
        temp_val += adc3_get_result(ch);
        delay_ms(5);
    }

    return temp_val / times;    /* ����ƽ��ֵ */
}

/**
 * @brief       ��ȡ�ڲ��¶ȴ������¶�ֵ
 * @param       ��
 * @retval      �¶�ֵ(������100��,��λ:��.)
 */
short adc3_get_temperature(void)
{
    uint32_t adcx;
    short result;
    double temperature;
    float temp = 0;
    uint16_t ts_cal1, ts_cal2;
    
    ts_cal1 = *(volatile uint16_t *)(0X1FF1E820);   /* ��ȡTS_CAL1 */
    ts_cal2 = *(volatile uint16_t *)(0X1FF1E840);   /* ��ȡTS_CAL2 */
    temp = (float)((110.0 - 30.0) / (ts_cal2 - ts_cal1));   /* ��ȡ�������� */
    
    adcx = adc3_get_result_average(ADC3_TEMPSENSOR_CHX, 10);/* ��ȡ�ڲ��¶ȴ�����ͨ��,10��ȡƽ�� */
    temperature = (float)(temp * (adcx - ts_cal1) + 30);    /* �����¶� */
    
    result = temperature *= 100;  /* ����100��. */
    return result;

}



















