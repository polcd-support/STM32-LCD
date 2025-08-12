/**
 ****************************************************************************************************
 * @file        adc.h
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
 * V1.0 20230321
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __ADC_H
#define __ADC_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* ADC������ ���� */

#define ADC_ADCX_CHY_GPIO_PORT              GPIOA
#define ADC_ADCX_CHY_GPIO_PIN               SYS_GPIO_PIN4 
#define ADC_ADCX_CHY_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA��ʱ��ʹ�� */

#define ADC_ADCX                            ADC1 
#define ADC_ADCX_CHY                        18                                      /* ͨ��Y,  0 <= Y <= 19 */ 
#define ADC_ADCX_CHY_CLK_ENABLE()           do{ RCC->AHB1ENR |= 1 << 5; }while(0)   /* ADC1 ʱ��ʹ�� */

/******************************************************************************************/


void adc_init(void);                /* ADC��ʼ�� */
void adc_channel_set(ADC_TypeDef *adcx, uint8_t ch, uint8_t stime); /* ADCͨ������ */
uint32_t adc_get_result(uint8_t ch);/* ���ĳ��ͨ��ֵ  */
uint32_t adc_get_result_average(uint8_t ch, uint8_t times); /* �õ�ĳ��ͨ����������������ƽ��ֵ */

#endif 















