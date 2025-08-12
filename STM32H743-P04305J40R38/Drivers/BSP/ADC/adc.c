/**
 ****************************************************************************************************
 * @file        adc.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       ADC ��������
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
 *
 ****************************************************************************************************
 */
 
#include "./BSP/ADC/adc.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       ADC��ʼ������
 * @note        ������֧��ADC1/ADC2����ͨ��,���ǲ�֧��ADC3
 *              ����ʹ��16λ����, ADC����ʱ��=32M, ת��ʱ��Ϊ:�������� + 8.5��ADC����
 *              ��������������: 810.5, ��ת��ʱ�� = 819��ADC���� = 25.6us
 * @param       ��
 * @retval      ��
 */
void adc_init(void)
{
    ADC_ADCX_CHY_GPIO_CLK_ENABLE(); /* IO��ʱ��ʹ�� */
    ADC_ADCX_CHY_CLK_ENABLE();      /* ADCʱ��ʹ�� */
    
    sys_gpio_set(ADC_ADCX_CHY_GPIO_PORT, ADC_ADCX_CHY_GPIO_PIN,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */

    RCC->AHB1RSTR |= 1 << 5;        /* ADC1/2��λ */
    RCC->AHB1RSTR &= ~(1 << 5);     /* ��λ���� */

    RCC->D3CCIPR &= ~(3 << 16);     /* ADCSEL[1:0]���� */
    RCC->D3CCIPR |= 2 << 16;        /* ADCSEL[1:0]=2,per_ck��ΪADCʱ��Դ,Ĭ��ѡ��hsi_ker_ck��Ϊper_ck,Ƶ��:64Mhz */
    ADC12_COMMON->CCR |= 1 << 18;   /* PRESC[3:0]=1,����ʱ��2��Ƶ,��adc_ker_ck=per_ck=64Mhz,ADC����ʱ��=����ʱ��/2=32M(���ܳ���36Mhz) */

    ADC_ADCX->CR = 0;               /* CR�Ĵ�������,DEEPPWD����,�����˯�߻���. */
    ADC_ADCX->CR |= 1 << 28;        /* ADVREGEN=1,ʹ��ADC��ѹ�� */
    
    delay_ms(10);                   /* �ȴ���ѹ���������,Լ10us,������ʱ��һ��,û��ϵ. */
    
    ADC_ADCX->CR |= 1 << 8;         /* BOOST=1,ADC������boostģʽ(ADCƵ�ʴ���20M��ʱ��,��������boostλ) */
    ADC_ADCX->CFGR &= ~(1 << 13);   /* CONT=0,����ת��ģʽ */
    ADC_ADCX->CFGR |= 1 << 12;      /* OVRMOD=1,��дģʽ(DR�Ĵ����ɱ���д) */
    ADC_ADCX->CFGR &= ~(3 << 10);   /* EXTEN[1:0]=0,������� */
    ADC_ADCX->CFGR &= ~(7 << 2);    /* RES[2:0]λ���� */
    ADC_ADCX->CFGR |= 0 << 2;       /* RES[2:0]=0,16λ�ֱ���(0,16λ;1,14λ;2,12λ;3,10λ;4,8λ.) */

    ADC_ADCX->CFGR2 &= ~((uint32_t)15 << 28);   /* LSHIFT[3:0]=0,������,�����Ҷ���. */
    ADC_ADCX->CFGR2 &= ~((uint32_t)0X3FF << 16);/* OSR[9:0]=0,��ʹ�ù����� */


    ADC_ADCX->CR &= ~((uint32_t)1 << 30);   /* ADCALDIF=0,У׼����ת��ͨ�� */
    ADC_ADCX->CR |= 1 << 16;            /* ADCALLIN=1,����У׼ */
    ADC_ADCX->CR |= (uint32_t)1 << 31;  /* ����У׼ */

    while (ADC_ADCX->CR & ((uint32_t)1 << 31)); /* �ȴ�У׼��� */

    ADC_ADCX->SQR1 &= ~(0XF << 0);  /* L[3:0]���� */
    ADC_ADCX->SQR1 |= 0 << 0;       /* L[3:0]=0,1��ת���ڹ��������� Ҳ����ֻת����������1 */
 
    ADC_ADCX->CR |= 1 << 0; /* ����ADת���� */
}

/**
 * @brief       ����ADCͨ������ʱ��
 * @param       adcx : adc�ṹ��ָ��, ADC1 / ADC2
 * @param       ch   : ͨ����, 0~19
 * @param       stime: ����ʱ��  0~7, ��Ӧ��ϵΪ:
 *   @arg       000, 1.5��ADCʱ������        001, 2.5��ADCʱ������
 *   @arg       010, 8.5��ADCʱ������        011, 16.5��ADCʱ������
 *   @arg       100, 32.5��ADCʱ������       101, 64.5��ADCʱ������
 *   @arg       110, 387.5��ADCʱ������      111, 810.5��ADCʱ������ 
 * @retval      ��
 */
void adc_channel_set(ADC_TypeDef *adcx, uint8_t ch, uint8_t stime)
{
    if (ch < 10)              /* ͨ��0~9,ʹ��SMPR1���� */
    { 
        adcx->SMPR1 &= ~(7 << (3 * ch));        /* ͨ��ch ����ʱ����� */
        adcx->SMPR1 |= 7 << (3 * ch);           /* ͨ��ch ������������,����Խ�߾���Խ�� */
    }
    else    /* ͨ��10~19,ʹ��SMPR2���� */
    { 
        adcx->SMPR2 &= ~(7 << (3 * (ch - 10))); /* ͨ��ch ����ʱ����� */
        adcx->SMPR2 |= 7 << (3 * (ch - 10));    /* ͨ��ch ������������,����Խ�߾���Խ�� */
    } 
}

/**
 * @brief       ���ADCת����Ľ�� 
 * @param       ch: ͨ����, 0~19
 * @retval      ��
 */
uint32_t adc_get_result(uint8_t ch)
{
    adc_channel_set(ADC_ADCX, ch, 7);   /* ����ADCX��Ӧͨ������ʱ��Ϊ810.5��ʱ������ */
    
    ADC_ADCX->PCSEL |= 1 << ch;         /* ADCת��ͨ��Ԥѡ�� */
    /* ����ת������ */
    ADC_ADCX->SQR1 &= ~(0X1F << 6 * 1); /* ��������1ͨ������ */
    ADC_ADCX->SQR1 |= ch << 6 * 1;      /* ���ù�������1��ת��ͨ��Ϊch */
    ADC_ADCX->CR |= 1 << 2;             /* ��������ת��ͨ�� */

    while (!(ADC_ADCX->ISR & 1 << 2));  /* �ȴ�ת������ */

    return ADC_ADCX->DR;                /* ����adcֵ */
}

/**
 * @brief       ��ȡͨ��ch��ת��ֵ��ȡtimes��,Ȼ��ƽ��
 * @param       ch      : ͨ����, 0~19
 * @param       times   : ��ȡ����
 * @retval      ͨ��ch��times��ת�����ƽ��ֵ
 */
uint32_t adc_get_result_average(uint8_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++) /* ��ȡtimes������ */
    {
        temp_val += adc_get_result(ch);
        delay_ms(5);
    }

    return temp_val / times;    /* ����ƽ��ֵ */
}









