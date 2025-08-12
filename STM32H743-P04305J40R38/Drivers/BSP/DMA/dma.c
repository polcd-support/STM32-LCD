/**
 ****************************************************************************************************
 * @file        dma.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-16
 * @brief       DMA ��������
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
 * V1.0 20230316
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/DMA/dma.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       ����TX DMA��ʼ������
 * @note        ����Ĵ�����ʽ�ǹ̶���, ���Ҫ���ݲ�ͬ��������޸�
 *              �Ӵ洢�� -> ����ģʽ/8λ���ݿ��/�洢������ģʽ
 *
 * @param       dma_streamx : DMA������,DMA1_Stream0~7/DMA2_Stream0~7
 * @param       ch          :  DMAͨ��ѡ��,��Χ:1~115(���<<STM32H7xx�ο��ֲ�>>16.3.2��,Table 116)
 * @param       par         : �����ַ
 * @param       mar         : �洢����ַ
 * @retval      ��
 */
void dma_usart_tx_config(DMA_Stream_TypeDef *dma_streamx, uint8_t ch, uint32_t par, uint32_t mar)
{
    dma_mux_init(dma_streamx, ch);  /* ��ʼ��DMA �������� */
    
    dma_streamx->PAR = par;     /* DMA�����ַ */
    dma_streamx->M0AR = mar;    /* DMA �洢��0��ַ */
    dma_streamx->NDTR = 0;      /* DMA ���䳤������, ������dma_enable�������� */
    dma_streamx->CR = 0;        /* ��ȫ����λCR�Ĵ���ֵ */

    dma_streamx->CR |= 1 << 6;  /* �洢��������ģʽ */
    dma_streamx->CR |= 0 << 8;  /* ��ѭ��ģʽ(��ʹ����ͨģʽ) */
    dma_streamx->CR |= 0 << 9;  /* ���������ģʽ */
    dma_streamx->CR |= 1 << 10; /* �洢������ģʽ */
    dma_streamx->CR |= 0 << 11; /* �������ݳ���:8λ */
    dma_streamx->CR |= 0 << 13; /* �洢�����ݳ���:8λ */
    dma_streamx->CR |= 1 << 16; /* �е����ȼ� */
    dma_streamx->CR |= 0 << 21; /* ����ͻ�����δ��� */
    dma_streamx->CR |= 0 << 23; /* �洢��ͻ�����δ��� */

    //dma_streamx->FCR=0X21;      /* FIFO���ƼĴ��� */
}

/**
 * @brief       DMA����������ʼ��
 * @note        �������ܰ���:
 *              1, ʹ��DMAʱ��
 *              2, ��ն�ӦStream����������ж϶��־
 *              3, ѡ��DMA����ͨ��
 *
 * @param       dma_streamx : DMA������,DMA1_Stream0~7/DMA2_Stream0~7
 * @param       ch          : DMAͨ��ѡ��,��Χ:1~115(���<<STM32H7xx�ο��ֲ�>>16.3.2��,Table 116)
 * @retval      ��
 */
void dma_mux_init(DMA_Stream_TypeDef *dma_streamx, uint8_t ch)
{
    DMA_TypeDef *DMAx;
    DMAMUX_Channel_TypeDef *DMAMUXx;
    uint8_t streamx;

    if ((uint32_t)dma_streamx > (uint32_t)DMA2)   /* �õ���ǰstream������DMA2����DMA1 */
    {
        DMAx = DMA2;
        RCC->AHB1ENR |= 1 << 1;     /* DMA2ʱ��ʹ�� */
    }
    else
    {
        DMAx = DMA1;
        RCC->AHB1ENR |= 1 << 0;     /* DMA1ʱ��ʹ�� */
    }

    while (dma_streamx->CR & 0X01); /* �ȴ�DMA������ */

    streamx = (((uint32_t)dma_streamx - (uint32_t)DMAx) - 0X10) / 0X18; /* �õ�streamͨ���� */

    if (streamx >= 6)
    {
        DMAx->HIFCR |= 0X3D << (6 * (streamx - 6) + 16);   /* ���֮ǰ��stream�ϵ������жϱ�־ */
    }
    else if (streamx >= 4)
    {
        DMAx->HIFCR |= 0X3D << 6 * (streamx - 4);           /* ���֮ǰ��stream�ϵ������жϱ�־ */
    }
    else if (streamx >= 2)
    {
        DMAx->LIFCR |= 0X3D << (6 * (streamx - 2) + 16);    /* ���֮ǰ��stream�ϵ������жϱ�־ */
    }
    else
    {
        DMAx->LIFCR |= 0X3D << 6 * streamx; /* ���֮ǰ��stream�ϵ������жϱ�־ */
    }
    
    if ((uint32_t)dma_streamx > (uint32_t)DMA2)
    {
        streamx += 8;  /* �����DMA2,ͨ�����+8 */
    }

    DMAMUXx = (DMAMUX_Channel_TypeDef *)(DMAMUX1_BASE + streamx * 4);   /* �õ���Ӧ��DMAMUXͨ�����Ƶ�ַ */
    DMAMUXx->CCR = ch & 0XFF;   /* ѡ��DMA����ͨ�� */
}


/**
 * @brief       ����һ��DMA����
 * @param       dma_streamx : DMA������,DMA1_Stream0~7/DMA2_Stream0~7
 * @param       ndtr        : ���ݴ�����
 * @retval      ��
 */
void dma_enable(DMA_Stream_TypeDef *dma_streamx, uint16_t ndtr)
{
    dma_streamx->CR &= ~(1 << 0);   /* �ر�DMA���� */

    while (dma_streamx->CR & 0X1);  /* ȷ��DMA���Ա����� */

    dma_streamx->NDTR = ndtr;       /* DMA �洢��0��ַ */
    dma_streamx->CR |= 1 << 0;      /* ����DMA���� */
}



























