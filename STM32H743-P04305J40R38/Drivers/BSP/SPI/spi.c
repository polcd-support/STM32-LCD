/**
 ****************************************************************************************************
 * @file        spi.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-23
 * @brief       SPI ��������
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
 * V1.0 20230323
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/SPI/spi.h"


/**
 * @brief       SPI��ʼ������
 * @note        ����ģʽ,8λ����,��ֹӲ��Ƭѡ
 * @param       ��
 * @retval      ��
 */
void spi2_init(void)
{
    uint32_t tempreg = 0;
    
    SPI2_SPI_CLK_ENABLE();          /* SPI2ʱ��ʹ�� */
    SPI2_SCK_GPIO_CLK_ENABLE();     /* SPI2_SCK��ʱ��ʹ�� */
    SPI2_MISO_GPIO_CLK_ENABLE();    /* SPI2_MISO��ʱ��ʹ�� */
    SPI2_MOSI_GPIO_CLK_ENABLE();    /* SPI2_MOSI��ʱ��ʹ�� */

    sys_gpio_set(SPI2_SCK_GPIO_PORT, SPI2_SCK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SCK����ģʽ����(�������) */

    sys_gpio_set(SPI2_MISO_GPIO_PORT, SPI2_MISO_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* MISO����ģʽ����(�������) */

    sys_gpio_set(SPI2_MOSI_GPIO_PORT, SPI2_MOSI_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* MOSI����ģʽ����(�������) */

    sys_gpio_af_set(SPI2_SCK_GPIO_PORT, SPI2_SCK_GPIO_PIN, SPI2_SCK_GPIO_AF);       /* SCK��, AF�������� */
    sys_gpio_af_set(SPI2_MISO_GPIO_PORT, SPI2_MISO_GPIO_PIN, SPI2_SCK_GPIO_AF);     /* MISO��, AF�������� */
    sys_gpio_af_set(SPI2_MOSI_GPIO_PORT, SPI2_MOSI_GPIO_PIN, SPI2_SCK_GPIO_AF);     /* MOSI��, AF�������� */

    /* ����SPI1/2/3��ʱ��Դ, ѡ��pll1_q_ck��Ϊʱ��Դ, 200Mhz */
    RCC->D2CCIP1R &= ~(7 << 12);    /* SPI123SEL[2:0]=0,���ԭ�������� */
    RCC->D2CCIP1R |= 0 << 12;       /* SPI123SEL[2:0]=1,ѡ��pll1_q_ck��ΪSPI1/2/3��ʱ��Դ,һ��Ϊ200Mhz */
 
    /* ����ֻ���SPI�ڳ�ʼ�� */
    RCC->APB1LRSTR |= 1 << 14;      /* ��λSPI2_SPI */
    RCC->APB1LRSTR &= ~(1 << 14);   /* ֹͣ��λSPI2_SPI */

    SPI2_SPI->CR1 |= 1 << 12;       /* SSI=1,�����ڲ�SS�ź�Ϊ�ߵ�ƽ */
    SPI2_SPI->CFG1 = 7 << 28;       /* MBR[2:0]=7,����spi_ker_ckΪ256��Ƶ. */
    SPI2_SPI->CFG1 |= 7 << 0;       /* DSIZE[4:0]=7,����SPI֡��ʽΪ8λ,���ֽڴ��� */
    tempreg = (uint32_t)1 << 31;    /* AFCNTR=1,SPI���ֶ�IO�ڵĿ��� */
    tempreg |= 0 << 29;             /* SSOE=0,��ֹӲ��NSS��� */
    tempreg |= 1 << 26;             /* SSM=1,�������NSS�� */
    tempreg |= 1 << 25;             /* CPOL=1,����״̬��,SCKΪ�ߵ�ƽ */
    tempreg |= 1 << 24;             /* CPHA=1,���ݲ����ӵ�2��ʱ����ؿ�ʼ */
    tempreg |= 0 << 23;             /* LSBFRST=0,MSB�ȴ��� */
    tempreg |= 1 << 22;             /* MASTER=1,����ģʽ */
    tempreg |= 0 << 19;             /* SP[2:0]=0,Ħ��������ʽ */
    tempreg |= 0 << 17;             /* COMM[1:0]=0,ȫ˫��ͨ�� */
    SPI2_SPI->CFG2 = tempreg;       /* ����CFG2�Ĵ��� */
    SPI2_SPI->I2SCFGR &= ~(1 << 0); /* ѡ��SPIģʽ */
    SPI2_SPI->CR1 |= 1 << 0;        /* SPE=1,ʹ��SPI2_SPI */

    spi2_read_write_byte(0xff);     /* �������� */
}

/**
 * @brief       SPI2�ٶ����ú���
 * @note        SPI2ʱ��ѡ������pll1_q_ck, Ϊ200Mhz
 *              SPI�ٶ� = spi_ker_ck / 2^(speed + 1)
 * @param       speed: SPI2ʱ�ӷ�Ƶϵ��
 * @retval      ��
 */
void spi2_set_speed(uint8_t speed)
{
    speed &= 0X07;                          /* ���Ʒ�Χ */
    SPI2_SPI->CR1 &= ~(1 << 0);             /* SPE=0,SPI�豸ʧ�� */
    SPI2_SPI->CFG1 &= ~(7 << 28);           /* MBR[2:0]=0,���ԭ���ķ�Ƶ���� */
    SPI2_SPI->CFG1 |= (uint32_t)speed << 28;/* MBR[2:0]=SpeedSet,����SPI2_SPI�ٶ� */
    SPI2_SPI->CR1 |= 1 << 0;                /* SPE=1,SPI�豸ʹ�� */
}

/**
 * @brief       SPI2��дһ���ֽ�����
 * @param       txdata: Ҫ���͵�����(1�ֽ�)
 * @retval      ���յ�������(1�ֽ�)
 */
uint8_t spi2_read_write_byte(uint8_t txdata)
{
    uint8_t rxdata = 0;
    SPI2_SPI->CR1 |= 1 << 0;    /* SPE=1,ʹ��SPI2_SPI */
    SPI2_SPI->CR1 |= 1 << 9;    /* CSTART=1,�������� */

    while ((SPI2_SPI->SR & 1 << 1) == 0);   /* �ȴ��������� */

    /* ����һ��byte,�Դ��䳤�ȷ���TXDR�Ĵ��� */
    *(volatile uint8_t *)&SPI2_SPI->TXDR = txdata;  


    while ((SPI2_SPI->SR & 1 << 0) == 0);   /* �ȴ�������һ��byte */

    /* ����һ��byte,�Դ��䳤�ȷ���RXDR�Ĵ��� */
    rxdata = *(volatile uint8_t *)&SPI2_SPI->RXDR;  

    SPI2_SPI->IFCR |= 3 << 3;   /* EOTC��TXTFC��1,���EOT��TXTFCλ */
    SPI2_SPI->CR1 &= ~(1 << 0); /* SPE=0,�ر�SPI2_SPI,��ִ��״̬����λ/FIFO���õȲ��� */
    return rxdata;              /* �����յ������� */
}






