/**
 ****************************************************************************************************
 * @file        sai.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2023-03-25
 * @brief       SAI ��������
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
 * V1.0 20230325
 * ��һ�η���
 * V1.1 20230325
 * ���sai1_saib_init/sai1_rx_dma_init/sai1_rec_start/sai1_rec_stop�Ⱥ���,֧��¼��
 *
 ****************************************************************************************************
 */

#include "./BSP/SAI/sai.h"
#include "./BSP/LCD/ltdc.h"
#include "os.h"


/**
 * @brief       SAI1 Block A��ʼ��, I2S,�����ֱ�׼
 * @param       mode    : 00,��������;01,��������;10,�ӷ�����;11,�ӽ�����
 * @param       cpol    : 0,ʱ���½���ѡͨ;1,ʱ��������ѡͨ
 * @param       datalen : ���ݴ�С,0/1,δ�õ�,2,8λ;3,10λ;4,16λ;5,20λ;6,24λ;7,32λ.
 * @retval      ��
 */
void sai1_saia_init(uint8_t mode, uint8_t cpol, uint8_t datalen)
{
    uint32_t tempreg = 0;

    SAI1_SAI_CLK_ENABLE();              /* SAI ʱ��ʹ�� */
    SAI1_MCLK_GPIO_CLK_ENABLE();        /* SAI MCLK GPIO ʱ��ʹ�� */
    SAI1_SCK_GPIO_CLK_ENABLE();         /* SAI SCK  GPIO ʱ��ʹ�� */
    SAI1_FS_GPIO_CLK_ENABLE();          /* SAI FS GPIO ʱ��ʹ�� */
    SAI1_SDA_GPIO_CLK_ENABLE();         /* SAI SDA GPIO ʱ��ʹ�� */
    SAI1_SDB_GPIO_CLK_ENABLE();         /* SAI SDB GPIO ʱ��ʹ�� */

    RCC->D2CCIP1R &= ~(7 << 0);         /* SAI1SEL[2:0]���� */
    RCC->D2CCIP1R |= 2 << 0;            /* SAI1SEL[2:0]=2,ѡ��pll3_p_ck��ΪSAI1��ʱ��Դ */
    
    RCC->APB2RSTR |= 1 << 22;           /* ��λSAI1 */
    RCC->APB2RSTR &= ~(1 << 22);        /* ������λ */

    sys_gpio_set(SAI1_MCLK_GPIO_PORT, SAI1_MCLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* MCLK ����ģʽ����(�������) */

    sys_gpio_set(SAI1_SCK_GPIO_PORT, SAI1_SCK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SCK ����ģʽ����(�������) */

    sys_gpio_set(SAI1_FS_GPIO_PORT, SAI1_FS_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* FS ����ģʽ����(�������) */
    
    sys_gpio_set(SAI1_SDA_GPIO_PORT, SAI1_SDA_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SDA ����ģʽ����(�������) */
    
    sys_gpio_set(SAI1_SDB_GPIO_PORT, SAI1_SDB_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SDB ����ģʽ����(�������) */

    sys_gpio_af_set(SAI1_MCLK_GPIO_PORT,  SAI1_MCLK_GPIO_PIN,  SAI1_MCLK_GPIO_AF);  /* MCLK��, AF�������� */
    sys_gpio_af_set(SAI1_SCK_GPIO_PORT,  SAI1_SCK_GPIO_PIN,  SAI1_SCK_GPIO_AF);     /* SCK��, AF�������� */
    sys_gpio_af_set(SAI1_FS_GPIO_PORT, SAI1_FS_GPIO_PIN, SAI1_FS_GPIO_AF);          /* FS��, AF�������� */
    sys_gpio_af_set(SAI1_SDA_GPIO_PORT,  SAI1_SDA_GPIO_PIN,  SAI1_SDA_GPIO_AF);     /* SDA��, AF�������� */
    sys_gpio_af_set(SAI1_SDB_GPIO_PORT,  SAI1_SDB_GPIO_PIN,  SAI1_SDB_GPIO_AF);     /* SDB��, AF�������� */

    tempreg |= mode << 0;           /* ����SAI1����ģʽ */
    tempreg |= 0 << 2;              /* ����SAI1Э��Ϊ:����Э��(֧��I2S/LSB/MSB/TDM/PCM/DSP��Э��) */
    tempreg |= datalen << 5;        /* �������ݴ�С */
    tempreg |= 0 << 8;              /* ����MSBλ���� */
    tempreg |= (uint16_t)cpol << 9; /* ������ʱ�ӵ�����/�½���ѡͨ */
    tempreg |= 0 << 10;             /* ��Ƶģ���첽 */
    tempreg |= 0 << 12;             /* ������ģʽ */
    tempreg |= 1 << 13;             /* ����������Ƶģ����� */
    tempreg |= 0 << 19;             /* ʹ����ʱ�ӷ�Ƶ��(MCKDIV) */
    SAI1_SAIA->CR1 = tempreg;       /* ����CR1�Ĵ��� */

    tempreg = (64 - 1) << 0;        /* ����֡����Ϊ64,��ͨ��32��SCK,��ͨ��32��SCK */
    tempreg |= (32 - 1) << 8;       /* ����֡ͬ����Ч��ƽ����,��I2Sģʽ��=1/2֡�� */
    tempreg |= 1 << 16;             /* FS�ź�ΪSOF�ź�+ͨ��ʶ���ź� */
    tempreg |= 0 << 17;             /* FS�͵�ƽ��Ч(�½���) */
    tempreg |= 1 << 18;             /* ��slot0�ĵ�һλ��ǰһλʹ��FS,��ƥ������ֱ�׼ */
    SAI1_SAIA->FRCR = tempreg;

    tempreg = 0 << 0;               /* slotƫ��(FBOFF)Ϊ0 */
    tempreg |= 2 << 6;              /* slot��СΪ32λ */
    tempreg |= (2 - 1) << 8;        /* slot��Ϊ2�� */
    tempreg |= (1 << 17) | (1 << 16);   /* ʹ��slot0��slot1 */
    SAI1_SAIA->SLOTR = tempreg;     /* ����slot */

    SAI1_SAIA->CR2 = 1 << 0;        /* ����FIFO��ֵ:1/4 FIFO */
    SAI1_SAIA->CR2 |= 1 << 3;       /* FIFOˢ�� */
}

/**
 * @brief       SAI1 Block B��ʼ��, I2S,�����ֱ�׼, ͬ��ģʽ, ����ʼ��IO
 * @param       mode    : 00,��������;01,��������;10,�ӷ�����;11,�ӽ�����
 * @param       cpol    : 0,ʱ���½���ѡͨ;1,ʱ��������ѡͨ
 * @param       datalen : ���ݴ�С,0/1,δ�õ�,2,8λ;3,10λ;4,16λ;5,20λ;6,24λ;7,32λ.
 * @retval      ��
 */
void sai1_saib_init(uint8_t mode, uint8_t cpol, uint8_t datalen)
{
    uint32_t tempreg = 0;

    tempreg |= mode << 0;           /* ����SAI1����ģʽ */
    tempreg |= 0 << 2;              /* ����SAI1Э��Ϊ:����Э��(֧��I2S/LSB/MSB/TDM/PCM/DSP��Э��) */
    tempreg |= datalen << 5;        /* �������ݴ�С */
    tempreg |= 0 << 8;              /* ����MSBλ���� */
    tempreg |= (uint16_t)cpol << 9; /* ������ʱ�ӵ�����/�½���ѡͨ */
    tempreg |= 1 << 10;             /* ʹ��ͬ��ģʽ */
    tempreg |= 0 << 12;             /* ������ģʽ */
    tempreg |= 1 << 13;             /* ����������Ƶģ����� */
    SAI1_Block_B->CR1 = tempreg;    /* ����CR1�Ĵ��� */

    tempreg = (64 - 1) << 0;        /* ����֡����Ϊ64,��ͨ��32��SCK,��ͨ��32��SCK */
    tempreg |= (32 - 1) << 8;       /* ����֡ͬ����Ч��ƽ����,��I2Sģʽ��=1/2֡�� */
    tempreg |= 1 << 16;             /* FS�ź�ΪSOF�ź�+ͨ��ʶ���ź� */
    tempreg |= 0 << 17;             /* FS�͵�ƽ��Ч(�½���) */
    tempreg |= 1 << 18;             /* ��slot0�ĵ�һλ��ǰһλʹ��FS,��ƥ������ֱ�׼ */
    SAI1_Block_B->FRCR = tempreg;

    tempreg = 0 << 0;               /* slotƫ��(FBOFF)Ϊ0 */
    tempreg |= 2 << 6;              /* slot��СΪ32λ */
    tempreg |= (2 - 1) << 8;        /* slot��Ϊ2�� */
    tempreg |= (1 << 17) | (1 << 16);   /* ʹ��slot0��slot1 */
    SAI1_Block_B->SLOTR = tempreg;  /* ����slot */

    SAI1_Block_B->CR2 = 1 << 0;     /* ����FIFO��ֵ:1/4 FIFO */
    SAI1_Block_B->CR2 |= 1 << 3;    /* FIFOˢ�� */
    SAI1_Block_B->CR1 |= 1 << 17;   /* ʹ��DMA */
    SAI1_Block_B->CR1 |= 1 << 16;   /* ʹ��SAI1 Block B */
}

/**
 * SAI Block A����������
 * �����ʼ��㹫ʽ:
 * MCKDIV!=0: Fs=SAI_CK_x/[512*MCKDIV]
 * MCKDIV==0: Fs=SAI_CK_x/256
 * SAI_CK_x=(HSE/pllm)*PLLI2SN/PLLI2SQ/(PLLI2SDIVQ+1)
 * һ��HSE=25Mhz
 * pllm:��sys_stm32_clock_init���õ�ʱ��ȷ����һ����25
 * PLLI2SN      : 192~432
 * PLLI2SQ      : 2~15
 * PLLI2SDIVQ   : 0~31
 * MCKDIV       : 0~15
 * SAI A��Ƶϵ����@pllm = 25, HSE = 25Mhz,��vco����Ƶ��Ϊ1Mhz
 */

/* SAI Block A���������� 
 * �����ʼ��㹫ʽ(��NOMCK=0,OSR=0Ϊǰ��):
 * Fmclk = 256*Fs = sai_x_ker_ck / MCKDIV[5:0]
 * Fs = sai_x_ker_ck / (256 * MCKDIV[5:0])
 * Fsck = Fmclk * (FRL[7:0]+1) / 256 = Fs * (FRL[7:0] + 1)
 * ����:
 * sai_x_ker_ck = (HSE / PLL3DIVM) * (PLL3DIVN + 1) / (PLL3DIVP + 1)
 * HSE:һ��Ϊ25Mhz
 * PLL3DIVM     :1~63,��ʾ1~63��Ƶ
 * PLL3DIVN     :3~511,��ʾ4~512��Ƶ
 * PLL3DIVP     :0~127,��ʾ1~128��Ƶ
 * MCKDIV       :0~63,��ʾ1~63��Ƶ,0Ҳ��1��Ƶ,�Ƽ�����Ϊż��
 * SAI A��Ƶϵ����@PLL3DIVM=25,HSE=25Mhz,��vco����Ƶ��Ϊ1Mhz
 * ���ʽ:
 * ������|(PLL3DIVN+1)|(PLL3DIVP+1)|MCKDIV
 */
const uint16_t SAI_PSC_TBL[][5] =
{
    {800, 256, 5, 25},      /* 8Khz������ */
    {1102, 302, 107, 0},    /* 11.025Khz������ */
    {1600, 426, 2, 52},     /* 16Khz������ */
    {2205, 429, 38, 2},     /* 22.05Khz������ */
    {3200, 426, 1, 52},     /* 32Khz������ */
    {4410, 429, 1, 38},     /* 44.1Khz������ */
    {4800, 467, 1, 38},     /* 48Khz������ */
    {8820, 429, 1, 19},     /* 88.2Khz������ */
    {9600, 467, 1, 19},     /* 96Khz������ */
    {17640, 271, 1, 6},     /* 176.4Khz������ */
    {19200, 295, 6, 0},     /* 192Khz������ */
};


/******************************************************************************************/
/* ���´�����������RGB��������ʱ�ӣ�����SAI�ı�PLL3���ú�����ʱ�ӹ��ߵ����쳣������
 * ���ʹ�õ���MCU������ͻ������ⲿ�ִ��� 
 */

/* �ֱ���:����ʱ�Ӷ�Ӧ��,ǰ��Ϊ����ֱ���,����ΪĿ������ʱ��Ƶ�� */
const uint16_t ltdc_clk_table[4][2] =
{
    480, 9,
    800, 33,
    1024, 50,
    1280, 50,
};

/**
 * @brief       ר�����LTDC��һ��ʱ�����ú���
 * @param       pll3n     : ��ǰ��pll3��Ƶϵ��
 * @retval      ��
 */
void sai_ltdc_clkset(uint16_t pll3n)
{
    uint8_t i = 0;
    uint8_t pll3r = 0;

    if (lcdltdc.pwidth == 0)return;         /* ����RGB��,����Ҫ���� */

    for (i = 0; i < 4; i++)                 /* �����Ƿ��ж�ӦRGB���ķֱ��� */
    {
        if (lcdltdc.pwidth == ltdc_clk_table[i][0])
        {
            break;                          /* �ҵ�����ֱ���˳� */
        }
    }

    if (i == 4)return;                      /* �ұ���Ҳû�ҵ���Ӧ�ķֱ��� */

    pll3r = pll3n / ltdc_clk_table[i][1];   /* �õ�������pll3r��ֵ */

    if (pll3n > (pll3r * ltdc_clk_table[i][1]))
    {
        pll3r += 1;
    }
    
    ltdc_clk_set(pll3n, 25, pll3r);         /* ��������PLL3��R��Ƶ */
}
/******************************************************************************************/

/**
 * @brief       ����SAI1�Ĳ�����(@MCKEN)
 * @param       samplerate  : ������, ��λ:Hz
 * @retval      0,���óɹ�
 *              1,�޷�����
 */
uint8_t sai1_samplerate_set(uint32_t samplerate)
{
    uint16_t retry = 0;
    uint8_t i = 0;
    uint32_t tempreg = 0;
    samplerate /= 10;   /* ��С10�� */

    for (i = 0; i < (sizeof(SAI_PSC_TBL) / 10); i++)    /* �����Ĳ������Ƿ����֧�� */
    {
        if (samplerate == SAI_PSC_TBL[i][0])break;
    }

    RCC->CR &= ~(1 << 28);                              /* �ȹر�PLLI3 */

    if (i == (sizeof(SAI_PSC_TBL) / 10))return 1;       /* �ѱ���Ҳ�Ҳ��� */

    while (((RCC->CR & (1 << 29))) && (retry < 0X1FFF)) /* �ȴ�PLL3ʱ��ʧ�� */
    {
        retry++; 
    }
    
    RCC->PLLCKSELR &= ~(0X3F << 20);                    /* ���DIVM3[5:0]ԭ�������� */
    RCC->PLLCKSELR |= 25 << 20;                         /* DIVM3[5:0]=25,����PLL3��Ԥ��Ƶϵ�� */
    tempreg = RCC->PLL3DIVR;                            /* ��ȡPLL3DIVR��ֵ */
    tempreg &= 0XFFFF0000;                              /* ���DIVN��PLL3DIVPԭ�������� */
    tempreg |= (uint32_t)(SAI_PSC_TBL[i][1] - 1) << 0;  /* ����DIVN[8:0] */
    tempreg |= (uint32_t)(SAI_PSC_TBL[i][2] - 1) << 9;  /* ����DIVP[6:0] */
    RCC->PLL3DIVR = tempreg;                            /* ����PLL3DIVR�Ĵ��� */
    RCC->PLLCFGR |= 1 << 22;                            /* DIVP3EN=1,ʹ��pll3_p_ck */
    RCC->CR |= 1 << 28;                                 /* ����PLL3 */

    while ((RCC->CR & 1 << 29) == 0);                   /* �ȴ�PLL3�����ɹ� */

    tempreg = SAI1_Block_A->CR1;
    tempreg &= ~(0X3F << 20);                           /* ���MCKDIV[5:0]���� */
    tempreg |= (uint32_t)SAI_PSC_TBL[i][3] << 20;       /* ����MCKDIV[5:0] */
    tempreg |= 1 << 16;                                 /* ʹ��SAI1 Block A */
    tempreg |= 1 << 17;                                 /* ʹ��DMA */
    SAI1_Block_A->CR1 = tempreg;                        /* ����MCKDIV[5:0],ͬʱʹ��SAI1 Block A */

    sai_ltdc_clkset(SAI_PSC_TBL[i][1]);                 /* ���RGB��,��Ҫ�����޸�PLL3R��ֵ */
    
    return 0;
}

/**
 * @brief       SAI1 TX DMA����
 *  @note       ����Ϊ˫����ģʽ,������DMA��������ж�
 * @param       buf0    : M0AR��ַ.
 * @param       buf1    : M1AR��ַ.
 * @param       num     : ÿ�δ���������
 * @param       width   : λ��(�洢��������,ͬʱ����),0,8λ;1,16λ;2,32λ;
 * @retval      ��
 */
void sai1_tx_dma_init(uint8_t *buf0, uint8_t *buf1, uint16_t num, uint8_t width)
{
    SAI1_TX_DMA_CLK_ENABLE();           /* SAI1 DMAʱ��ʹ�� */

    while (SAI1_TX_DMASx->CR & 0X01);   /* �ȴ�SAI1_TX_DMASx������ */

    /* ���<<STM32H7xx�ο��ֲ�>>16.3.2��,Table 119 */
    DMAMUX1_Channel11->CCR = SAI1_TX_DMASx_Channel; /* DMA2_Stream3��ͨ��ѡ��: 87,��SAI1_A��Ӧ��ͨ�� */

    SAI1_TX_DMASx_CLR_TC();             /* ���һ���ж� */ 
    SAI1_TX_DMASx->FCR = 0X0000021;     /* ����ΪĬ��ֵ */ 

    SAI1_TX_DMASx->PAR = (uint32_t)&SAI1_SAIA->DR;  /* �����ַΪ:SAI1_SAI->DR */ 
    SAI1_TX_DMASx->M0AR = (uint32_t)buf0;           /* �ڴ�1��ַ */ 
    SAI1_TX_DMASx->M1AR = (uint32_t)buf1;           /* �ڴ�2��ַ */ 
    SAI1_TX_DMASx->NDTR = num;          /* ��ʱ���ó���Ϊ1 */ 
    SAI1_TX_DMASx->CR = 0;              /* ��ȫ����λCR�Ĵ���ֵ */ 
    SAI1_TX_DMASx->CR |= 1 << 6;        /* �洢��������ģʽ */ 
    SAI1_TX_DMASx->CR |= 1 << 8;        /* ѭ��ģʽ */ 
    SAI1_TX_DMASx->CR |= 0 << 9;        /* ���������ģʽ */ 
    SAI1_TX_DMASx->CR |= 1 << 10;       /* �洢������ģʽ */ 
    SAI1_TX_DMASx->CR |= (uint16_t)width << 11;     /* �������ݳ���:16λ/32λ */ 
    SAI1_TX_DMASx->CR |= (uint16_t)width << 13;     /* �洢�����ݳ���:16λ/32λ */ 
    SAI1_TX_DMASx->CR |= 2 << 16;       /* �����ȼ� */ 
    SAI1_TX_DMASx->CR |= 1 << 18;       /* ˫����ģʽ */ 
    SAI1_TX_DMASx->CR |= 0 << 21;       /* ����ͻ�����δ��� */ 
    SAI1_TX_DMASx->CR |= 0 << 23;       /* �洢��ͻ�����δ��� */ 
    SAI1_TX_DMASx->CR |= 0 << 25;       /* ѡ��ͨ��0 SAI1_Aͨ�� */ 

    SAI1_TX_DMASx->FCR &= ~(1 << 2);    /* ��ʹ��FIFOģʽ */ 
    SAI1_TX_DMASx->FCR &= ~(3 << 0);    /* ��FIFO ���� */ 

    SAI1_TX_DMASx->CR |= 1 << 4;        /* ������������ж� */ 
    sys_nvic_init(0, 0, SAI1_TX_DMASx_IRQn, 2);     /* ��ռ1�������ȼ�0����2 */ 
}

/**
 * @brief       SAI1 TX DMA����
 *  @note       ����Ϊ˫����ģʽ,������DMA��������ж�
 * @param       buf0    : M0AR��ַ.
 * @param       buf1    : M1AR��ַ.
 * @param       num     : ÿ�δ���������
 * @param       width   : λ��(�洢��������,ͬʱ����),0,8λ;1,16λ;2,32λ;
 * @retval      ��
 */
void sai1_rx_dma_init(uint8_t *buf0, uint8_t *buf1, uint16_t num, uint8_t width)
{
    SAI1_RX_DMA_CLK_ENABLE();           /* SAI1 RX DMAʱ��ʹ�� */

    while (SAI1_RX_DMASx->CR & 0X01);   /* �ȴ�SAI1_RX_DMASx������ */
    
    /* ���<<STM32H7xx�ο��ֲ�>>16.3.2��,Table 119 */
    DMAMUX1_Channel13->CCR = SAI1_RX_DMASx_Channel; /* DMA2_Stream5��ͨ��ѡ��: 88,��SAI1_B��Ӧ��ͨ�� */

    SAI1_RX_DMASx_CLR_TC();             /* ���һ���ж� */ 
    SAI1_RX_DMASx->FCR = 0X0000021;     /* ����ΪĬ��ֵ */ 

    SAI1_RX_DMASx->PAR = (uint32_t)&SAI1_SAIB->DR;  /* �����ַΪ:SAI1_SAI->DR */ 
    SAI1_RX_DMASx->M0AR = (uint32_t)buf0;           /* �ڴ�1��ַ */ 
    SAI1_RX_DMASx->M1AR = (uint32_t)buf1;           /* �ڴ�2��ַ */ 
    SAI1_RX_DMASx->NDTR = num;          /* ��ʱ���ó���Ϊ1 */ 
    SAI1_RX_DMASx->CR = 0;              /* ��ȫ����λCR�Ĵ���ֵ */ 
    SAI1_RX_DMASx->CR |= 0 << 6;        /* ���赽�洢��ģʽ */ 
    SAI1_RX_DMASx->CR |= 1 << 8;        /* ѭ��ģʽ */ 
    SAI1_RX_DMASx->CR |= 0 << 9;        /* ���������ģʽ */ 
    SAI1_RX_DMASx->CR |= 1 << 10;       /* �洢������ģʽ */ 
    SAI1_RX_DMASx->CR |= (uint16_t)width << 11;     /* �������ݳ���:16λ/32λ */ 
    SAI1_RX_DMASx->CR |= (uint16_t)width << 13;     /* �洢�����ݳ���:16λ/32λ */ 
    SAI1_RX_DMASx->CR |= 1 << 16;       /* �е����ȼ� */ 
    SAI1_RX_DMASx->CR |= 1 << 18;       /* ˫����ģʽ */ 
    SAI1_RX_DMASx->CR |= 0 << 21;       /* ����ͻ�����δ��� */ 
    SAI1_RX_DMASx->CR |= 0 << 23;       /* �洢��ͻ�����δ��� */ 
    SAI1_RX_DMASx->CR |= 0 << 25;       /* ѡ��ͨ��0 SAI1_Aͨ�� */ 

    SAI1_RX_DMASx->FCR &= ~(1 << 2);    /* ��ʹ��FIFOģʽ */ 
    SAI1_RX_DMASx->FCR &= ~(3 << 0);    /* ��FIFO ���� */ 

    SAI1_RX_DMASx->CR |= 1 << 4;        /* ������������ж� */ 
    sys_nvic_init(0, 1, SAI1_RX_DMASx_IRQn, 2);     /* ��ռ0�������ȼ�1����2 */ 
}

/* SAI1 DMA�ص�����ָ�� */ 
void (*sai1_tx_callback)(void); /* TX�ص����� */ 
void (*sai1_rx_callback)(void); /* RX�ص����� */ 

/**
 * @brief       SAI1 TX DMA �жϷ�����
 * @param       ��
 * @retval      ��
 */
void SAI1_TX_DMASx_IRQHandler(void)
{
    OSIntEnter();
    if (SAI1_TX_DMASx_IS_TC())      /* SAI1_TX_DMASx,������ɱ�־ */ 
    {
        SAI1_TX_DMASx_CLR_TC();     /* �����������ж� */ 
        sai1_tx_callback();         /* ִ�лص�����,��ȡ���ݵȲ����������洦�� */
        SCB_CleanInvalidateDCache();/* �����Ч��D-Cache */
    }
    OSIntExit();
}

/**
 * @brief       SAI1 RX DMA �жϷ�����
 * @param       ��
 * @retval      ��
 */
void SAI1_RX_DMASx_IRQHandler(void)
{
    OSIntEnter();
    if (SAI1_RX_DMASx_IS_TC())      /* SAI1_RX_DMASx,������ɱ�־ */ 
    {
        SAI1_RX_DMASx_CLR_TC();     /* �����������ж� */ 
        sai1_rx_callback();         /* ִ�лص�����,��ȡ���ݵȲ����������洦�� */ 
        SCB_CleanInvalidateDCache();/* �����Ч��D-Cache */
    }
    OSIntExit();
}

/**
 * @brief       SAI��ʼ����
 * @param       ��
 * @retval      ��
 */
void sai1_play_start(void)
{
    SAI1_TX_DMASx->CR |= 1 << 0;    /* ����DMA TX���� */ 
}

/**
 * @brief       SAIֹͣ����
 * @param       ��
 * @retval      ��
 */
void sai1_play_stop(void)
{
    SAI1_TX_DMASx->CR &= ~(1 << 0); /* �������� */ 
}

/**
 * @brief       SAI��ʼ¼��
 * @param       ��
 * @retval      ��
 */
void sai1_rec_start(void)
{
    SAI1_RX_DMASx->CR |= 1 << 0;    /* ����DMA RX���� */ 
}

/**
 * @brief       SAI�ر�¼��
 * @param       ��
 * @retval      ��
 */
void sai1_rec_stop(void)
{
    SAI1_RX_DMASx->CR &= ~(1 << 0); /* ����¼�� */ 
}

















