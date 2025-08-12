/**
 ****************************************************************************************************
 * @file        dcmi.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       DCMI ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20230324
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/OV5640/ov5640.h"
#include "os.h"


uint8_t g_ov_frame = 0;                 /* ֡�� */
extern void jpeg_data_process(void);    /* JPEG���ݴ����� */

/**
 * @brief       DCMI�жϷ�����
 * @param       ��
 * @retval      ��
 */
void DCMI_IRQHandler(void)
{
    OSIntEnter();
    
    if (DCMI->MISR & 0X01)      /* ����һ֡ͼ�� */
    {
        jpeg_data_process();    /* jpeg���ݴ��� */
        DCMI->ICR |= 1 << 0;    /* ���֡�ж� */
//        LED1_TOGGLE();          /* LED1��˸ */
        g_ov_frame++;
    }
    
    OSIntExit();
}

/**
 * @brief       DCMI DMA����
 * @param       mem0addr: �洢����ַ0     ��Ҫ�洢����ͷ���ݵ��ڴ��ַ(Ҳ�����������ַ)
 * @param       mem1addr: �洢����ַ1     ��ֻʹ��mem0addr��ʱ��,��ֵ����Ϊ0
 * @param       memsize : �洢������      0~65535
 * @param       memblen : �洢��λ��      0,8λ,1,16λ,2,32λ
 * @param       meminc  : �洢��������ʽ  0,������; 1,����
 * @retval      ��
 */
void dcmi_dma_init(uint32_t mem0addr, uint32_t mem1addr, uint16_t memsize, uint8_t memblen, uint8_t meminc)
{
    uint32_t tempreg = 0;
    RCC->AHB1ENR |= 1 << 0;             /* DMA1ʱ��ʹ�� */
    RCC->D3AMR |= 1 << 0;               /* DMAMUXʱ��ʹ�� */

    while (DMA1_Stream1->CR & 0X01);    /* �ȴ�DMA1������ */

    /* ͨ��ѡ�� ���<<STM32H7xx�ο��ֲ�>>16.3.2��,Table 116 
     * DMAMUX1_Channel1~7,  ��Ӧ DMA1_Stream1~7
     * DMAMUX1_Channel8~15, ��Ӧ DMA2_Stream1~7
     * ���DMA���̵�˵��
     */
    DMAMUX1_Channel1->CCR = 75;         /* DMA1_stream1��ͨ��ѡ��: 75,��DCMI��Ӧ��ͨ�� */

    DMA1->LIFCR |= 0X3D << 6 * 1;       /* ���ͨ��1�������жϱ�־ */
    DMA1_Stream1->FCR = 0X0000021;      /* ����ΪĬ��ֵ */

    DMA1_Stream1->PAR = (uint32_t)&DCMI->DR; /* �����ַΪ:DCMI->DR */
    DMA1_Stream1->M0AR = mem0addr;      /* mem0addr��ΪĿ���ַ0 */
    DMA1_Stream1->M1AR = mem1addr;      /* mem1addr��ΪĿ���ַ1 */
    DMA1_Stream1->NDTR = memsize;       /* ���䳤��Ϊmemsize */
    tempreg |= 0 << 6;                  /* ���赽�洢��ģʽ */
    tempreg |= 1 << 8;                  /* ѭ��ģʽ */
    tempreg |= 0 << 9;                  /* ���������ģʽ */
    tempreg |= meminc << 10;            /* �洢������ģʽ */
    tempreg |= 2 << 11;                 /* �������ݳ���:32λ */
    tempreg |= memblen << 13;           /* �洢��λ��,8/16/32bit */
    tempreg |= 2 << 16;                 /* �����ȼ� */
    tempreg |= 0 << 21;                 /* ����ͻ�����δ��� */
    tempreg |= 0 << 23;                 /* �洢��ͻ�����δ��� */

    if (mem1addr)   /* ˫�����ʱ��,����Ҫ���� */
    {
        tempreg |= 1 << 18;             /* ˫����ģʽ */
        tempreg |= 1 << 4;              /* ������������ж� */
        sys_nvic_init(2, 3, DMA1_Stream1_IRQn, 2);   /* ��ռ1�������ȼ�3����2 */
    }

    DMA1_Stream1->CR = tempreg;         /* ����CR�Ĵ��� */
}

/* DCMI DMA���ջص�����, ��˫����ģʽ�õ�, ����жϷ�����ʹ�� */
void (*dcmi_rx_callback)(void);

/**
 * @brief       DMA1_Stream1�жϷ�����(��˫����ģʽ���õ�)
 * @param       ��
 * @retval      ��
 */
void DMA1_Stream1_IRQHandler(void)
{
    OSIntEnter();
    
    if (DMA1->LISR & (1 << 11)) /* DMA1_Steam1,������ɱ�־ */
    {
        DMA1->LIFCR |= 1 << 11; /* �����������ж� */
        dcmi_rx_callback();     /* ִ������ͷ���ջص�����,��ȡ���ݵȲ����������洦�� */
        SCB_CleanInvalidateDCache();    /* �����Ч��D-Cache */
    }
    
    OSIntExit();
}

/**
 * @brief       DCMI ��ʼ��
 * @note        IO��Ӧ��ϵ����:
 *              ����ͷģ�� ------------ STM32������
 *               OV_D0~D7  ------------  PC6/PC7/PC8/PC9/PC11/PB6/PE5/PE6
 *               OV_SCL    ------------  PD6
 *               OV_SDA    ------------  PD7
 *               OV_VSYNC  ------------  PB7
 *               OV_HREF   ------------  PA4
 *               OV_PCLK   ------------  PA6
 *               OV_PWDN   ------------  PG9
 *               OV_RESET  ------------  PG15
 *               OV_XLCK   ------------  PA8
 *              ����������ʼ��OV_D0~D7/OV_VSYNC/OV_HREF/OV_PCLK���ź�(11��).
 *
 * @param       ��
 * @retval      ��
 */
void dcmi_init(void)
{
    uint32_t tempreg = 0;
    /* ����IO */
    RCC->AHB4ENR |= 1 << 0;     /* ʹ������PORTAʱ�� */
    RCC->AHB4ENR |= 1 << 1;     /* ʹ������PORTBʱ�� */
    RCC->AHB4ENR |= 1 << 2;     /* ʹ������PORTCʱ�� */
    RCC->AHB4ENR |= 1 << 3;     /* ʹ������PORTDʱ�� */
    RCC->AHB4ENR |= 1 << 7;     /* ʹ������PORTHʱ�� */
    RCC->AHB2ENR |= 1 << 0;     /* ��DCMIʱ�� */

    /* PA6���ù������ */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN6,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);

    /* PB7/8/9���ù������ */
    sys_gpio_set(GPIOB, SYS_GPIO_PIN7 | SYS_GPIO_PIN8 | SYS_GPIO_PIN9,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);

    /* PC6/7/8/9/11���ù������ */
    sys_gpio_set(GPIOC, SYS_GPIO_PIN6 | SYS_GPIO_PIN7 | SYS_GPIO_PIN8 | SYS_GPIO_PIN9 | SYS_GPIO_PIN11,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);

    /* PD13���ù������ */
    sys_gpio_set(GPIOD, SYS_GPIO_PIN3,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);

    /* PH8���ù������ */
    sys_gpio_set(GPIOH, SYS_GPIO_PIN8,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);

    sys_gpio_af_set(GPIOH, SYS_GPIO_PIN8, 13);  /* PH8,AF13  DCMI_HSYNC */
    sys_gpio_af_set(GPIOA, SYS_GPIO_PIN6, 13);  /* PA6,AF13  DCMI_PCLK */
    sys_gpio_af_set(GPIOB, SYS_GPIO_PIN7, 13);  /* PB7,AF13  DCMI_VSYNC */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN6, 13);  /* PC6,AF13  DCMI_D0 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN7, 13);  /* PC7,AF13  DCMI_D1 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 13);  /* PC8,AF13  DCMI_D2 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 13);  /* PC9,AF13  DCMI_D3 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 13); /* PC11,AF13 DCMI_D4 */
    sys_gpio_af_set(GPIOD, SYS_GPIO_PIN3, 13);  /* PD3,AF13  DCMI_D5 */
    sys_gpio_af_set(GPIOB, SYS_GPIO_PIN8, 13);  /* PB8,AF13  DCMI_D6 */
    sys_gpio_af_set(GPIOB, SYS_GPIO_PIN9, 13);  /* PB9,AF13  DCMI_D7 */
    
    /* ���ԭ�������� */
    DCMI->IER = 0x0;
    DCMI->ICR = 0x1F;
    DCMI->ESCR = 0x0;
    DCMI->ESUR = 0x0;
    DCMI->CWSTRTR = 0x0;
    DCMI->CWSIZER = 0x0;
    tempreg |= 0 << 1;      /* ����ģʽ */
    tempreg |= 0 << 2;      /* ȫ֡���� */
    tempreg |= 0 << 4;      /* Ӳ��ͬ��HSYNC,VSYNC */
    tempreg |= 1 << 5;      /* PCLK ��������Ч */
    tempreg |= 0 << 6;      /* HSYNC �͵�ƽ��Ч */
    tempreg |= 0 << 7;      /* VSYNC �͵�ƽ��Ч */
    tempreg |= 0 << 8;      /* �������е�֡ */
    tempreg |= 0 << 10;     /* 8λ���ݸ�ʽ */
    DCMI->IER |= 1 << 0;    /* ����֡�ж� */
    tempreg |= 1 << 14;     /* DCMIʹ�� */
    DCMI->CR = tempreg;     /* ����CR�Ĵ��� */
    
    sys_nvic_init(2, 2, DCMI_IRQn, 2);  /* ��ռ1�������ȼ�2����2 */
}

/**
 * @brief       DCMI,��������
 * @param       ��
 * @retval      ��
 */
void dcmi_start(void)
{
    if (lcddev.wramcmd != 0)    /* MCU�� */
    {
        lcd_set_cursor(0, 0);   /* �������굽ԭ�� */
        lcd_write_ram_prepare();/* ��ʼд��GRAM */
    }
    
    DMA1_Stream1->CR |= 1 << 0; /* ����DMA1,Stream1 */
    DCMI->CR |= 1 << 0;         /* DCMI����ʹ�� */
}

/**
 * @brief       DCMI,�رմ���
 * @param       ��
 * @retval      ��
 */
void dcmi_stop(void)
{
    DCMI->CR &= ~(1 << 0);          /* DCMI����ر� */

    while (DCMI->CR & 0X01);        /* �ȴ�������� */

    DMA1_Stream1->CR &= ~(1 << 0);  /* �ر�DMA1,Stream1 */
}

/******************************************************************************************/
/* ������������,��usmart����,���ڵ��Դ��� */

/**
 * @brief       DCMI������ʾ����
 * @param       sx,sy       : LCD����ʼ����
 * @param       width,height: LCD��ʾ��Χ.
 * @retval      ��
 */
void dcmi_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    dcmi_stop();
    lcd_clear(WHITE);
    lcd_set_window(sx, sy, width, height);
    ov5640_outsize_set(0, 0, width, height);
    lcd_set_cursor(0, 0);
    lcd_write_ram_prepare();        /* ��ʼд��GRAM */
    DMA1_Stream1->CR |= 1 << 0;     /* ����DMA1,Stream1 */
    DCMI->CR |= 1 << 0;             /* DCMI����ʹ�� */
}

/**
 * @brief       ͨ��usmart����,����������.
 * @param       pclk/hsync/vsync : �����źŵ���Ч��ƽ����
 * @retval      ��
 */
void dcmi_cr_set(uint8_t pclk, uint8_t hsync, uint8_t vsync)
{
    DCMI->CR = 0;
    DCMI->CR |= pclk << 5;      /* PCLK ��Ч�������� */
    DCMI->CR |= hsync << 6;     /* HSYNC ��Ч��ƽ���� */
    DCMI->CR |= vsync << 7;     /* VSYNC ��Ч��ƽ���� */
    DCMI->CR |= 1 << 14;        /* DCMIʹ�� */
    DCMI->CR |= 1 << 0;         /* DCMI����ʹ�� */
}

/******************************************************************************************/







