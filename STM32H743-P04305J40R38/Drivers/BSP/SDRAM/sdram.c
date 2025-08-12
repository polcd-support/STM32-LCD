/**
 ****************************************************************************************************
 * @file        sdram.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       SDRAM ��������
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
 
#include "./SYSTEM/delay/delay.h"
#include "./BSP/SDRAM/sdram.h"


/**
 * @brief       ��SDRAM��������
 * @param       bankx       : 0,��BANK5�����SDRAM����ָ��
 * @param                     1,��BANK6�����SDRAM����ָ��
 * @param       cmd         : ָ��(0,����ģʽ/1,ʱ������ʹ��/2,Ԥ������д洢��/3,�Զ�ˢ��/4,����ģʽ�Ĵ���/5,��ˢ��/6,����)
 * @param       refresh     : ��ˢ�´���(cmd=3ʱ��Ч)
 * @param       regval      : ģʽ�Ĵ���ֵ
 * @retval      ����ֵ       :0,����; 1,ʧ��.
 */
uint8_t sdram_send_cmd(uint8_t bankx, uint8_t cmd, uint8_t refresh, uint16_t regval)
{
    uint32_t retry = 0;
    uint32_t tempreg = 0;
    
    tempreg |= cmd << 0;            /* ����ָ�� */
    tempreg |= 1 << (4 - bankx);    /* ���÷���ָ�bank5����6 */
    tempreg |= refresh << 5;        /* ������ˢ�´��� */
    tempreg |= regval << 9;         /* ����ģʽ�Ĵ�����ֵ */
    FMC_Bank5_6_R->SDCMR = tempreg; /* ���üĴ��� */

    while ((FMC_Bank5_6_R->SDSR & (1 << 5)))    /* �ȴ�ָ������ */
    {
        retry++;

        if (retry > 0X1FFFFF)return 1;
    }

    return 0;
}

/**
 * @brief       ��ʼ��SDRAM
 * @param       ��
 * @retval      ��
 */
void sdram_init(void)
{
    uint32_t sdctrlreg = 0, sdtimereg = 0;
    uint16_t mregval = 0;
    
    RCC->AHB4ENR |= 0X1F << 2;  /* ʹ��PC/PD/PE/PF/PGʱ�� */
    RCC->AHB3ENR |= 1 << 12;    /* ʹ��FMCʱ�� */

    
    sys_gpio_set(GPIOC, SYS_GPIO_PIN0 | SYS_GPIO_PIN2 | SYS_GPIO_PIN3,                          /* PC0/2/3 ���ù������ */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 
                 
    sys_gpio_set(GPIOD, 3 << 0 | 7 << 8 | 3 << 14,                                              /* PD0/1/8/9/10/14/15 ���ù������ */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 

    sys_gpio_set(GPIOE, 3 << 0 | 0X1FF << 7,                                                    /* PE0/1/7~15 ���ù������ */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 

    sys_gpio_set(GPIOF, 0X3F << 0 | 0X1F << 11,                                                 /* PF0~5/11~15 ���ù������ */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 

    sys_gpio_set(GPIOG, 7 << 0 | 3 << 4 | SYS_GPIO_PIN8 | SYS_GPIO_PIN15,                       /* PG0~2/4/5/8/15 ���ù������ */
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU); 

    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN0 | SYS_GPIO_PIN2 | SYS_GPIO_PIN3, 12);      /* PC0,2,3              AF12 */
    sys_gpio_af_set(GPIOD, 3 << 0 | 7 << 8 | 3 << 14, 12);                          /* PD0/1/8/9/10/14/15   AF12 */
    sys_gpio_af_set(GPIOE, 3 << 0 | 0X1FF << 7, 12);                                /* PE0/1/7~15           AF12 */
    sys_gpio_af_set(GPIOF, 0X3F << 0 | 0X1F << 11, 12);                             /* PF0~5/11~15          AF12 */
    sys_gpio_af_set(GPIOG, 7 << 0 | 3 << 4 | SYS_GPIO_PIN8 | SYS_GPIO_PIN15, 12);   /* PG0~2/4/5/8/15       AF12 */


    sdctrlreg |= 1 << 0;                /* 9λ�е�ַ */
    sdctrlreg |= 2 << 2;                /* 13λ�е�ַ */
    sdctrlreg |= 1 << 4;                /* 16λ����λ�� */
    sdctrlreg |= 1 << 6;                /* 4���ڲ�����(4 BANKS) */
    sdctrlreg |= 2 << 7;                /* 2��CAS�ӳ� */
    sdctrlreg |= 0 << 9;                /* ����д���� */
    sdctrlreg |= 2 << 10;               /* SDRAMʱ��=pll2_r_ck/2=220M/2=110M=9.1ns */
    sdctrlreg |= 1 << 12;               /* ʹ��ͻ������ */
    sdctrlreg |= 0 << 13;               /* ��ͨ���ӳ�0��pll2_r_ck */
    FMC_Bank5_6_R->SDCR[0] = sdctrlreg; /* ����FMC BANK5 SDRAM���ƼĴ���(BANK5��6���ڹ���SDRAM) */

    sdtimereg |= 1 << 0;                /* ����ģʽ�Ĵ���������ʱ����ӳ�Ϊ2��ʱ������ */
    sdtimereg |= 7 << 4;                /* �˳���ˢ���ӳ�Ϊ8��ʱ������ */
    sdtimereg |= 4 << 8;                /* ��ˢ��ʱ��Ϊ7��ʱ������ */
    sdtimereg |= 6 << 12;               /* ��ѭ���ӳ�Ϊ7��ʱ������ */
    sdtimereg |= 1 << 16;               /* �ָ��ӳ�Ϊ2��ʱ������ */
    sdtimereg |= 1 << 20;               /* ��Ԥ����ӳ�Ϊ2��ʱ������ */
    sdtimereg |= 1 << 24;               /* �е����ӳ�Ϊ2��ʱ������ */
    FMC_Bank5_6_R->SDTR[0] = sdtimereg; /* ����FMC BANK5 SDRAMʱ��Ĵ��� */
    FMC_Bank1_R->BTCR[0] |= (uint32_t)1 << 31;  /* ʹ��FMC */

    sdram_send_cmd(0, 1, 0, 0);         /* ʱ������ʹ�� */
    delay_us(500);                      /* �����ӳ�200us. */
    sdram_send_cmd(0, 2, 0, 0);         /* �����д洢��Ԥ��� */
    sdram_send_cmd(0, 3, 8, 0);         /* ������ˢ�´��� */
    mregval |= 1 << 0;                  /* ����ͻ������:1(������1/2/4/8) */
    mregval |= 0 << 3;                  /* ����ͻ������:����(����������/����) */
    mregval |= 2 << 4;                  /* ����CASֵ:2(������2/3) */
    mregval |= 0 << 7;                  /* ���ò���ģʽ:0,��׼ģʽ */
    mregval |= 1 << 9;                  /* ����ͻ��дģʽ:1,������� */
    sdram_send_cmd(0, 4, 0, mregval);   /* ����SDRAM��ģʽ�Ĵ��� */

    /**
     * ˢ��Ƶ�ʼ�����(��SDCLKƵ�ʼ���),���㷽��:
     * COUNT=SDRAMˢ������/����-20=SDRAMˢ������(us)*SDCLKƵ��(Mhz)/����
     * ����ʹ�õ�SDRAMˢ������Ϊ64ms,SDCLK=220/2=110Mhz,����Ϊ8192(2^13).
     * ����,COUNT=64*1000*110/8192-20=839
     */
    FMC_Bank5_6_R->SDRTR = 839 << 1;    /* ����ˢ��Ƶ�ʼ����� */
}

/**
 * @brief       ��ָ����ַ(writeaddr + Bank5_SDRAM_ADDR)��ʼ,����д��n���ֽ�
 * @param       pbuf        : �ֽ�ָ��
 * @param       writeaddr   : Ҫд��ĵ�ַ
 * @param       n           : Ҫд����ֽ���
 * @retval      ��
*/
void fmc_sdram_write_buffer(uint8_t *pbuf, uint32_t writeaddr, uint32_t n)
{
    for (; n != 0; n--)
    {
        *(volatile uint8_t *)(BANK5_SDRAM_ADDR + writeaddr) = *pbuf;
        writeaddr++;
        pbuf++;
    }
}

/**
 * @brief       ��ָ����ַ(readaddr + Bank5_SDRAM_ADDR)��ʼ,������ȡn���ֽ�
 * @param       pbuf        : �ֽ�ָ��
 * @param       readaddr    : Ҫ��ȡ�ĵ�ַ
 * @param       n           : Ҫ��ȡ���ֽ���
 * @retval      ��
*/
void fmc_sdram_read_buffer(uint8_t *pbuf, uint32_t readaddr, uint32_t n)
{
    for (; n != 0; n--)
    {
        *pbuf++ = *(volatile uint8_t *)(BANK5_SDRAM_ADDR + readaddr);
        readaddr++;
    }
}






























