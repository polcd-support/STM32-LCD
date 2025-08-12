/**
 ****************************************************************************************************
 * @file        spdif.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-25
 * @brief       SPDIF ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230325
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/SPDIF/spdif.h"
#include "./SYSTEM/delay/delay.h"
#include "os.h"


spdif_rx_dev spdif_dev;     /* SPDIF���ƽṹ�� */


/**
 * @brief       SPDIF RX��ʼ��
 * @param       ��
 * @retval      ��
 */
void spdif_rx_init(void)
{
    uint32_t tempreg;
    
    RCC->APB1LENR |= 1 << 16;            /* ʹ��SPDIF RXʱ�� */

    SPDIF_RX_GPIO_CLK_ENABLE();         /* ʹ��SPDIF RX��GPIOʱ�� */

    sys_gpio_set(SPDIF_RX_GPIO_PORT,    /* SPDIF RX ����GPIO �˿� */
                 SPDIF_RX_GPIO_PIN,     /* SPDIF RX ����GPIO ���ź� */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);

    sys_gpio_af_set(SPDIF_RX_GPIO_PORT,  SPDIF_RX_GPIO_PIN,  SPDIF_RX_GPIO_AF); /* SPDIF RX��, AF�������� */

    RCC->D2CCIP1R &= ~(3 << 20);        /* SPDIFSEL[1:0]���� */
    RCC->D2CCIP1R |= 0 << 20;           /* SPDIFSEL[1:0]=0,ѡ��pll1_q_ck,��ΪSPDIFʱ��Դ,Ƶ��Ϊ200Mhz */
    spdif_dev.clock = 200000000;        /* ����SPDIF CLK��Ƶ��,Ϊ200Mhz,Ҫ֧��192Khz�����ʱ��뱣֤clock��135.2Mhz */
    
    spdif_rx_mode(SPDIF_RX_IDLE);       /* �Ƚ���IDLEģʽ */
    tempreg = 1 << 16;                  /* ѡ��SPDIF RXͨ��1(��ѡ��0,1,2,3) */
    tempreg |= 1 << 14;                 /* �ȴ�SPDIF RX��·�ϵĻ */
    tempreg |= 2 << 12;                 /* ͬ���׶�,�����������15�� */
    tempreg |= 0 << 11;                 /* ��ͨ��A��ȡͨ��״̬ */
    tempreg |= 0 << 10;                 /* SPDIF����ͨ��״̬���û���Ϣ��ʹ��DMA���� */
    tempreg |= 1 << 9;                  /* ��ͷ���Ͳ�д��DR */
    tempreg |= 1 << 8;                  /* ͨ��״̬���û�λ��д��DR */
    tempreg |= 1 << 7;                  /* ��Ч��λ��д��DR */
    tempreg |= 1 << 6;                  /* ��żУ�����λ��д��DR */
    tempreg |= 0 << 4;                  /* ���ݲ����Ҷ���(LSB),���֧��24λ��Ƶ������ */
    tempreg |= 1 << 3;                  /* ������ģʽ */
    tempreg |= 1 << 2;                  /* SPDIF��Ƶ����ʹ��DMA������ */
    SPDIFRX->CR = tempreg;
    SPDIFRX->IMR |= (1 << 6) | (1 << 2);    /* ʹ����żУ������жϺʹ��нӿڴ����ж� */

    sys_nvic_init(1, 0, SPDIF_RX_IRQn, 2);  /* ����SPDIF RX�ж�,��ռ1�������ȼ�0����2 */
}

/**
 * @brief       ����SPDIF�Ĺ���ģʽ
 * @param       mode        : 0, IDLEģʽ
 *                            1, RXͬ��ģʽ
 *                            2, ����
 *                            3, ��������ģʽ
 * @retval      ��
 */
void spdif_rx_mode(uint8_t mode)
{
    uint32_t tempreg = 0;
    tempreg = SPDIFRX->CR;
    tempreg &= ~(0X03);
    tempreg |= mode & 0X03;         /* ����ģʽ */
    SPDIFRX->CR = tempreg;
}

/**
 * @brief       SPDIF RX����DMA����
 *  @note       ����Ϊ˫����ģʽ,������DMA��������ж�
 * @param       buf0        : M0AR��ַ
 * @param       buf1        : M1AR��ַ
 * @param       num         : ÿ�δ���������
 * @param       width       : λ��(�洢��������,ͬʱ����),0,8λ; 1,16λ; 2,32λ;
 * @retval      ��
 */
void spdif_rx_dma_init(uint32_t *buf0, uint32_t *buf1, uint16_t num, uint8_t width)
{
    RCC->AHB1ENR |= 1 << 0;             /* DMA1ʱ��ʹ�� */

    while (DMA1_Stream1->CR & 0X01);    /* �ȴ�DMA1_Stream1������ */

    /* ���<<STM32H7xx�ο��ֲ�>>16.3.2��,Table 116 */
    DMAMUX1_Channel1->CCR = 93;         /* DMA1_Stream1��ͨ��ѡ��: 93,��SPDIFRX_DT��Ӧ��ͨ�� */
    
    DMA1->LIFCR |= 0X3D << 6 * 0;       /* ���ͨ��1�������жϱ�־ */
    DMA1_Stream1->FCR = 0X0000021;      /* ����ΪĬ��ֵ */

    DMA1_Stream1->PAR = (uint32_t)&SPDIFRX->DR; /* �����ַΪ:SPDIFRX->DR */
    DMA1_Stream1->M0AR = (uint32_t)buf0;        /* �ڴ�1��ַ */
    DMA1_Stream1->M1AR = (uint32_t)buf1;        /* �ڴ�2��ַ */
    DMA1_Stream1->NDTR = num;                   /* ���ó��� */
    DMA1_Stream1->CR = 0;                       /* ��ȫ����λCR�Ĵ���ֵ */
    DMA1_Stream1->CR |= 0 << 6;                 /* ���赽�洢��ģʽ */
    DMA1_Stream1->CR |= 1 << 8;                 /* ѭ��ģʽ */
    DMA1_Stream1->CR |= 0 << 9;                 /* ���������ģʽ */
    DMA1_Stream1->CR |= 1 << 10;                /* �洢������ģʽ */
    DMA1_Stream1->CR |= (uint16_t)width << 11;  /* �������ݳ���:16λ/32λ */
    DMA1_Stream1->CR |= (uint16_t)width << 13;  /* �洢�����ݳ���:16λ/32λ */
    DMA1_Stream1->CR |= 2 << 16;                /* �����ȼ� */
    DMA1_Stream1->CR |= 1 << 18;                /* ˫����ģʽ */
    DMA1_Stream1->CR |= 0 << 21;                /* ����ͻ�����δ��� */
    DMA1_Stream1->CR |= 0 << 23;                /* �洢��ͻ�����δ��� */
    DMA1_Stream1->CR |= 0 << 25;                /* ѡ��ͨ��0 SPDIF RX DRͨ�� */

    DMA1_Stream1->FCR &= ~(1 << 2);             /* ��ʹ��FIFOģʽ */
    DMA1_Stream1->FCR &= ~(3 << 0);             /* ��FIFO ���� */
}

/**
 * @brief       �ȴ�����ͬ��״̬
 *  @note       ͬ������Ժ��Զ��������״̬
 * @param       ��
 * @retval      0, δͬ��
 *              1, ��ͬ��
 */
uint8_t spdif_rx_wait_sync(void)
{
    uint8_t res = 0;
    uint8_t timeout = 0;
    spdif_rx_mode(SPDIF_RX_SYNC);       /* ����Ϊͬ��ģʽ */

    while (1)
    {
        timeout++;
        delay_ms(2);

        if (timeout > 100)break;

        if (SPDIFRX->SR & (1 << 5))     /* ͬ�����? */
        {
            res = 1;                    /* ���ͬ����� */
            spdif_rx_mode(SPDIF_RX_RCV);/* �������ģʽ */
            break;
        }
    }

    return res;
}

/**
 * @brief       ��ȡSPDIF RX�յ�����Ƶ������
 * @param       ��
 * @retval      0, ����Ĳ�����
 *              ����ֵ, ��Ƶ������
 */
uint32_t spdif_rx_get_samplerate(void)
{
    uint16_t spdif_w5;
    uint32_t samplerate;
    
    spdif_w5 = SPDIFRX->SR >> 16;
    samplerate = (spdif_dev.clock * 5) / (spdif_w5 & 0X7FFF);
    samplerate >>= 6;       /* ����64 */

    if ((8000 - 1500 <= samplerate) && (samplerate <= 8000 + 1500))samplerate = 8000;           /* 8K�Ĳ����� */
    else if ((11025 - 1500 <= samplerate) && (samplerate <= 11025 + 1500))samplerate = 11025;   /* 11.025K�Ĳ����� */
    else if ((16000 - 1500 <= samplerate) && (samplerate <= 16000 + 1500))samplerate = 16000;   /* 16K�Ĳ����� */
    else if ((22050 - 1500 <= samplerate) && (samplerate <= 22050 + 1500))samplerate = 22050;   /* 22.05K�Ĳ����� */
    else if ((32000 - 1500 <= samplerate) && (samplerate <= 32000 + 1500))samplerate = 32000;   /* 32K�Ĳ����� */
    else if ((44100 - 1500 <= samplerate) && (samplerate <= 44100 + 1500))samplerate = 44100;   /* 44.1K�Ĳ����� */
    else if ((48000 - 1500 <= samplerate) && (samplerate <= 48000 + 1500))samplerate = 48000;   /* 48K�Ĳ����� */
    else if ((88200 - 1500 <= samplerate) && (samplerate <= 88200 + 1500))samplerate = 88200;   /* 88.2K�Ĳ����� */
    else if ((96000 - 1500 <= samplerate) && (samplerate <= 96000 + 1500))samplerate = 96000;   /* 96K�Ĳ����� */
    else if ((176400 - 6000 <= samplerate) && (samplerate <= 176400 + 6000))samplerate = 176400;/* 176.4K�Ĳ� */
    else if ((192000 - 6000 <= samplerate) && (samplerate <= 192000 + 6000))samplerate = 192000;/* 192K�Ĳ� */
    else samplerate = 0;

    return samplerate;
}

/* SAI DMA�ص�����ָ�� */
void (*spdif_rx_stop_callback)(void);   /* �ص����� */

/**
 * @brief       SPDIF�����жϷ�����
 * @param       ��
 * @retval      ��
 */
void SPDIF_RX_IRQHandler(void)
{
    uint32_t sr = SPDIFRX->SR;
    
    OSIntEnter();

    /* ������ʱ��ͬ����֡�����ж�,�������ж�һ��Ҫ����*/
    if (sr & ((1 << 8) | (1 << 7) | (1 << 6)))  /* ��ʱ����/ͬ������/֡���� */
    {
        spdif_rx_stop();                /* �������󣬹ر�SPDIF���� */
        spdif_rx_stop_callback();       /* ���ûص����� */
        spdif_rx_mode(SPDIF_RX_IDLE);   /* ��������ʱ��ͬ����֡�����ʱ��Ҫ��SPDIFRXENд0������ж� */
    }

    if (sr & (1 << 3))                  /* ������������� */
    {
        SPDIFRX->IFCR |= 1 << 3;        /* ���������� */
    }

    if (sr & (1 << 2))                  /* ��������żУ����� */
    {
        SPDIFRX->IFCR |= 1 << 2;        /* �����żУ����� */
    }
    
    OSIntExit(); 
}

/**
 * @brief       SPDIF��ʼ����
 * @param       ��
 * @retval      ��
 */
void spdif_rx_start(void)
{
    spdif_dev.consta = 1;           /* ����Ѿ���SPDIF */
    DMA1_Stream1->CR |= 1 << 0;     /* ����SPDIF DR RX���� */
}

/**
 * @brief       SPDIF�رղ���
 * @param       ��
 * @retval      ��
 */
void spdif_rx_stop(void)
{
    spdif_dev.consta = 0;           /* ����Ѿ��ر�SPDIF */
    DMA1_Stream1->CR &= ~(1 << 0);  /* ����SPDIF DMA���� */
}











