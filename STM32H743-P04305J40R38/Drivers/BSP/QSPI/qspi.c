/**
 ****************************************************************************************************
 * @file        qspi.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       QSPI ��������
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

#include "./BSP/QSPI/qspi.h"


/**
 * @brief       �ȴ�״̬��־
 * @param       flag : ��Ҫ�ȴ��ı�־λ
 * @param       sta  : ��Ҫ�ȴ���״̬
 * @param       wtime: �ȴ�ʱ��
 * @retval      0, �ȴ��ɹ�; 1, �ȴ�ʧ��.
 */
uint8_t qspi_wait_flag(uint32_t flag, uint8_t sta, uint32_t wtime)
{
    uint8_t flagsta = 0;

    while (wtime)
    {
        flagsta = (QUADSPI->SR & flag) ? 1 : 0; /* ��ȡ״̬��־ */

        if (flagsta == sta)break;

        wtime--;
    }

    if (wtime)return 0;
    else return 1;
}

/**
 * @brief       ��ʼ��QSPI�ӿ�
 * @param       ��
 * @retval      0, �ɹ�; 1, ʧ��.
 */
uint8_t qspi_init(void)
{
    uint32_t tempreg = 0;

    RCC->AHB3ENR |= 1 << 14;            /* QSPIʱ��ʹ�� */
    QSPI_BK1_CLK_GPIO_CLK_ENABLE();     /* QSPI_BK1_CLK IO��ʱ��ʹ�� */
    QSPI_BK1_NCS_GPIO_CLK_ENABLE();     /* QSPI_BK1_NCS IO��ʱ��ʹ�� */
    QSPI_BK1_IO0_GPIO_CLK_ENABLE();     /* QSPI_BK1_IO0 IO��ʱ��ʹ�� */
    QSPI_BK1_IO1_GPIO_CLK_ENABLE();     /* QSPI_BK1_IO1 IO��ʱ��ʹ�� */
    QSPI_BK1_IO2_GPIO_CLK_ENABLE();     /* QSPI_BK1_IO2 IO��ʱ��ʹ�� */
    QSPI_BK1_IO3_GPIO_CLK_ENABLE();     /* QSPI_BK1_IO3 IO��ʱ��ʹ�� */
    
   sys_gpio_set(QSPI_BK1_CLK_GPIO_PORT, QSPI_BK1_CLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* QSPI_BK1_CLK����ģʽ���� */

    sys_gpio_set(QSPI_BK1_NCS_GPIO_PORT, QSPI_BK1_NCS_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* QSPI_BK1_NCS����ģʽ���� */

    sys_gpio_set(QSPI_BK1_IO0_GPIO_PORT, QSPI_BK1_IO0_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* QSPI_BK1_IO0����ģʽ���� */

    sys_gpio_set(QSPI_BK1_IO1_GPIO_PORT, QSPI_BK1_IO1_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* QSPI_BK1_IO1����ģʽ���� */

    sys_gpio_set(QSPI_BK1_IO2_GPIO_PORT, QSPI_BK1_IO2_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* QSPI_BK1_IO2����ģʽ���� */

    sys_gpio_set(QSPI_BK1_IO3_GPIO_PORT, QSPI_BK1_IO3_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* QSPI_BK1_IO3����ģʽ���� */
 
    sys_gpio_af_set(QSPI_BK1_CLK_GPIO_PORT, QSPI_BK1_CLK_GPIO_PIN, QSPI_BK1_CLK_GPIO_AF);   /* QSPI_BK1_CLK��, AF�������� */
    sys_gpio_af_set(QSPI_BK1_NCS_GPIO_PORT, QSPI_BK1_NCS_GPIO_PIN, QSPI_BK1_NCS_GPIO_AF);   /* QSPI_BK1_NCS��, AF�������� */
    sys_gpio_af_set(QSPI_BK1_IO0_GPIO_PORT, QSPI_BK1_IO0_GPIO_PIN, QSPI_BK1_IO0_GPIO_AF);   /* QSPI_BK1_IO0��, AF�������� */
    sys_gpio_af_set(QSPI_BK1_IO1_GPIO_PORT, QSPI_BK1_IO1_GPIO_PIN, QSPI_BK1_IO1_GPIO_AF);   /* QSPI_BK1_IO1��, AF�������� */
    sys_gpio_af_set(QSPI_BK1_IO2_GPIO_PORT, QSPI_BK1_IO2_GPIO_PIN, QSPI_BK1_IO2_GPIO_AF);   /* QSPI_BK1_IO2��, AF�������� */
    sys_gpio_af_set(QSPI_BK1_IO3_GPIO_PORT, QSPI_BK1_IO3_GPIO_PIN, QSPI_BK1_IO3_GPIO_AF);   /* QSPI_BK1_IO3��, AF�������� */

    RCC->AHB3RSTR |= 1 << 14;       /* ��λQSPI */
    RCC->AHB3RSTR &= ~(1 << 14);    /* ֹͣ��λQSPI */
    
    if (qspi_wait_flag(1 << 5, 0, 0XFFFF) == 0) /* �ȴ�BUSY���� */
    {
        tempreg = (2 - 1) << 24;    /* ����QSPIʱ��Ĭ��ΪAHB3ʱ�ӣ���������ʹ��2��Ƶ, ��200M / 2 = 100Mhz,10ns */
        tempreg |= (4 - 1) << 8;    /* ����FIFO��ֵΪ4���ֽ�(���Ϊ31,��ʾ32���ֽ�) */
        tempreg |= 0 << 7;          /* ѡ��FLASH1 */
        tempreg |= 0 << 6;          /* ��ֹ˫����ģʽ */
        tempreg |= 1 << 4;          /* ������λ�������(DDRģʽ��,��������Ϊ0) */
        QUADSPI->CR = tempreg;      /* ����CR�Ĵ��� */
        tempreg = (25 - 1) << 16;   /* ����FLASH��СΪ2^25=32MB(Ϊ������sys.c������ڴ�ӳ�����ã�ʵ������������16MҲ�ǿ��Ե�) */
        tempreg |= (5 - 1) << 8;    /* Ƭѡ�ߵ�ƽʱ��Ϊ3��ʱ��(10*3=30ns),���ֲ������tSHSL���� */
        tempreg |= 1 << 0;          /* Mode3,����ʱCLKΪ�ߵ�ƽ */
        QUADSPI->DCR = tempreg;     /* ����DCR�Ĵ��� */
        QUADSPI->CR |= 1 << 0;      /* ʹ��QSPI */
    }
    else
    {
        return 1;   /* ���ɹ� */
    }
    
    return 0;
}

/**
 * @brief       QSPI��������
 * @param       cmd : Ҫ���͵�ָ��
 * @param       addr: ���͵���Ŀ�ĵ�ַ
 * @param       mode: ģʽ,��ϸλ��������:
 *   @arg       mode[1:0]: ָ��ģʽ; 00,��ָ��;  01,���ߴ���ָ��; 10,˫�ߴ���ָ��; 11,���ߴ���ָ��.
 *   @arg       mode[3:2]: ��ַģʽ; 00,�޵�ַ;  01,���ߴ����ַ; 10,˫�ߴ����ַ; 11,���ߴ����ַ.
 *   @arg       mode[5:4]: ��ַ����; 00,8λ��ַ; 01,16λ��ַ;     10,24λ��ַ;     11,32λ��ַ.
 *   @arg       mode[7:6]: ����ģʽ; 00,������;  01,���ߴ�������; 10,˫�ߴ�������; 11,���ߴ�������.
 * @param       dmcycle: ��ָ��������
 * @retval      ��
 */
void qspi_send_cmd(uint8_t cmd, uint32_t addr, uint8_t mode, uint8_t dmcycle)
{
    uint32_t tempreg = 0;
    uint8_t status;

    if (qspi_wait_flag(1 << 5, 0, 0XFFFF) == 0) /* �ȴ�BUSY���� */
    {
        tempreg = 0 << 31;                      /* ��ֹDDRģʽ */
        tempreg |= 0 << 28;                     /* ÿ�ζ�����ָ�� */
        tempreg |= 0 << 26;                     /* ���дģʽ */
        tempreg |= ((uint32_t)mode >> 6) << 24; /* ��������ģʽ */
        tempreg |= (uint32_t)dmcycle << 18;     /* ���ÿ�ָ�������� */
        tempreg |= ((uint32_t)(mode >> 4) & 0X03) << 12;    /* ���õ�ַ���� */
        tempreg |= ((uint32_t)(mode >> 2) & 0X03) << 10;    /* ���õ�ַģʽ */
        tempreg |= ((uint32_t)(mode >> 0) & 0X03) << 8;     /* ����ָ��ģʽ */ 
        tempreg |= cmd;                     /* ����ָ�� */
        QUADSPI->CCR = tempreg;             /* ����CCR�Ĵ��� */

        if (mode & 0X0C)                    /* ��ָ��+��ַҪ���� */
        {
            QUADSPI->AR = addr;             /* ���õ�ַ�Ĵ��� */
        }

        if ((mode & 0XC0) == 0)             /* �����ݴ���,�ȴ�ָ������ */
        {
            status = qspi_wait_flag(1 << 1, 1, 0XFFFF); /* �ȴ�TCF,��������� */

            if (status == 0)
            {
                QUADSPI->FCR |= 1 << 1;     /* ���TCF��־λ */
            }
        }
    }
}

/**
 * @brief       QSPI����ָ�����ȵ�����
 * @param       buf     : �������ݻ������׵�ַ
 * @param       datalen : Ҫ��������ݳ���
 * @retval      0, �ɹ�; ����, �������.
 */
uint8_t qspi_receive(uint8_t *buf, uint32_t datalen)
{
    uint32_t tempreg = QUADSPI->CCR;
    uint32_t addrreg = QUADSPI->AR;
    uint8_t status;
    volatile uint32_t *data_reg = &QUADSPI->DR;
    
    QUADSPI->DLR = datalen - 1; /* �������ݴ��䳤�� */
    tempreg &= ~(3 << 26);      /* ���FMODEԭ�������� */
    tempreg |= 1 << 26;         /* ����FMODEΪ��Ӷ�ȡģʽ */
    QUADSPI->FCR |= 1 << 1;     /* ���TCF��־λ */
    QUADSPI->CCR = tempreg;     /* ��дCCR�Ĵ��� */
    QUADSPI->AR = addrreg;      /* ��дAR�Ĵ���,�������� */

    while (datalen)
    {
        status = qspi_wait_flag(3 << 1, 1, 0XFFFF); /* �ȵ�FTF��TCF,�����յ������� */

        if (status == 0)        /* �ȴ��ɹ� */
        {
            *buf++ = *(volatile uint8_t *)data_reg;
            datalen--;
        }
        else
        {
            break;
        }
    }

    if (status == 0)
    {
        QUADSPI->CR |= 1 << 2;  /* ��ֹ���� */
        status = qspi_wait_flag(1 << 1, 1, 0XFFFF); /* �ȴ�TCF,�����ݴ������ */

        if (status == 0)
        {
            QUADSPI->FCR |= 1 << 1; /* ���TCF��־λ */
            status = qspi_wait_flag(1 << 5, 0, 0XFFFF); /* �ȴ�BUSYλ���� */
        }
    }

    return status;
}

/**
 * @brief       QSPI����ָ�����ȵ�����
 * @param       buf     : �������ݻ������׵�ַ
 * @param       datalen : Ҫ��������ݳ���
 * @retval      0, �ɹ�; ����, �������.
 */
uint8_t qspi_transmit(uint8_t *buf, uint32_t datalen)
{
    uint32_t tempreg = QUADSPI->CCR;
    uint32_t addrreg = QUADSPI->AR;
    uint8_t status;
    volatile uint32_t *data_reg = &QUADSPI->DR;
    
    QUADSPI->DLR = datalen - 1; /* �������ݴ��䳤�� */
    tempreg &= ~(3 << 26);      /* ���FMODEԭ�������� */
    tempreg |= 0 << 26;         /* ����FMODEΪ���д��ģʽ */
    QUADSPI->FCR |= 1 << 1;     /* ���TCF��־λ */
    QUADSPI->CCR = tempreg;     /* ��дCCR�Ĵ��� */

    while (datalen)
    {
        status = qspi_wait_flag(1 << 2, 1, 0XFFFF); /* �ȵ�FTF */

        if (status != 0)        /* �ȴ��ɹ� */
        {
            break;
        }

        *(volatile uint8_t *)data_reg = *buf++;
        datalen--;
    }

    if (status == 0)
    {
        QUADSPI->CR |= 1 << 2;      /* ��ֹ���� */
        status = qspi_wait_flag(1 << 1, 1, 0XFFFF);     /* �ȴ�TCF,�����ݴ������ */

        if (status == 0)
        {
            QUADSPI->FCR |= 1 << 1; /* ���TCF��־λ */
            status = qspi_wait_flag(1 << 5, 0, 0XFFFF); /* �ȴ�BUSYλ���� */
        }
    }

    return status;
}




















