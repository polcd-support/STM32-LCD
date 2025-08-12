/**
 ****************************************************************************************************
 * @file        rtc.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       RTC ��������
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

#include "./BSP/RTC/rtc.h"
#include "./BSP/LED/led.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "calendar.h"
#include "os.h"


/**
 * @brief       �ȴ�RSFͬ��
 * @param       ��
 * @retval      0,�ɹ�;1,ʧ��;
 */
static uint8_t rtc_wait_synchro(void)
{
    uint32_t retry = 0XFFFFF;
    /* �ر�RTC�Ĵ���д���� */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->ISR &= ~(1 << 5);  /* ���RSFλ */

    while (retry && ((RTC->ISR & (1 << 5)) == 0x00))
    {
        retry--;    /* �ȴ�Ӱ�ӼĴ���ͬ�� */
    }
    
    if (retry == 0)return 1;/* ͬ��ʧ�� */

    RTC->WPR = 0xFF;        /* ʹ��RTC�Ĵ���д���� */
    return 0;
}

/**
 * @brief       RTC�����ʼ��ģʽ
 * @param       ��
 * @retval      0,�ɹ�;1,ʧ��;
 */
static uint8_t rtc_init_mode(void)
{
    uint32_t retry = 0XFFFFF;

    if (RTC->ISR & (1 << 6))return 0;

    RTC->ISR |= 1 << 7; /* ����RTC��ʼ��ģʽ */

    while (retry && ((RTC->ISR & (1 << 6)) == 0x00))
    {
        retry--;    /* �ȴ�����RTC��ʼ��ģʽ�ɹ� */
    }
   
    if (retry == 0)
    {
        return 1;   /* ͬ��ʧ�� */
    }
    else 
    {
        return 0;   /* ͬ���ɹ� */
    }
}

/**
 * @brief       RTCд�������SRAM
 * @param       bkrx : �����Ĵ������,��Χ:0~31
 * @param       data : Ҫд�������,32λ����
 * @retval      ��
 */
void rtc_write_bkr(uint32_t bkrx, uint32_t data)
{
    uint32_t temp = 0;
    temp = RTC_BASE + 0x50 + bkrx * 4;
    (*(uint32_t *)temp) = data;
}

/**
 * @brief       RTC��ȡ������SRAM
 * @param       bkrx : �����Ĵ������,��Χ:0~31 
 * @retval      ��ȡ����ֵ
 */
uint32_t rtc_read_bkr(uint32_t bkrx)
{
    uint32_t temp = 0;
    temp = RTC_BASE + 0x50 + bkrx * 4;
    return (*(uint32_t *)temp); /* ���ض�ȡ����ֵ */
}

/**
 * @brief       ʮ����ת��ΪBCD��
 * @param       val : Ҫת����ʮ������ 
 * @retval      BCD��
 */
static uint8_t rtc_dec2bcd(uint8_t val)
{
    uint8_t bcdhigh = 0;

    while (val >= 10)
    {
        bcdhigh++;
        val -= 10;
    }

    return ((uint8_t)(bcdhigh << 4) | val);
}

/**
 * @brief       BCD��ת��Ϊʮ��������
 * @param       val : Ҫת����BCD�� 
 * @retval      ʮ��������
 */
static uint8_t rtc_bcd2dec(uint8_t val)
{
    uint8_t temp = 0;
    temp = (val >> 4) * 10;
    return (temp + (val & 0X0F));
}

/**
 * @brief       RTCʱ������
 * @param       hour,min,sec: Сʱ,����,���� 
 * @param       ampm        : AM/PM, 0=AM/24H; 1=PM/12H;
 * @retval      0,�ɹ�
 *              1,�����ʼ��ģʽʧ��
 */
uint8_t rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ampm)
{
    uint32_t temp = 0;
    /* �ر�RTC�Ĵ���д���� */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    if (rtc_init_mode())return 1;   /* ����RTC��ʼ��ģʽʧ�� */

    temp = (((uint32_t)ampm & 0X01) << 22) | ((uint32_t)rtc_dec2bcd(hour) << 16) | ((uint32_t)rtc_dec2bcd(min) << 8) | (rtc_dec2bcd(sec));
    RTC->TR = temp;
    RTC->ISR &= ~(1 << 7);  /* �˳�RTC��ʼ��ģʽ */
    return 0;
}

/**
 * @brief       RTC��������
 * @param       year,month,date : ��(0~99),��(1~12),��(0~31)
 * @param       week            : ����(1~7,0,�Ƿ�!)
 * @retval      0,�ɹ�
 *              1,�����ʼ��ģʽʧ��
 */
uint8_t rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week)
{
    uint32_t temp = 0;
    /* �ر�RTC�Ĵ���д���� */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    if (rtc_init_mode())return 1;   /* ����RTC��ʼ��ģʽʧ�� */

    temp = (((uint32_t)week & 0X07) << 13) | ((uint32_t)rtc_dec2bcd(year) << 16) | ((uint32_t)rtc_dec2bcd(month) << 8) | (rtc_dec2bcd(date));
    RTC->DR = temp;
    RTC->ISR &= ~(1 << 7);  /* �˳�RTC��ʼ��ģʽ */
    return 0;
}

/**
 * @brief       ��ȡRTCʱ��
 * @param       *hour,*min,*sec : Сʱ,����,����
 * @param       *ampm           : AM/PM,0=AM/24H,1=PM.
 * @retval      ��
 */
void rtc_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *ampm)
{
    uint32_t temp = 0;

    while (rtc_wait_synchro()); /* �ȴ�ͬ�� */

    temp = RTC->TR;
    *hour = rtc_bcd2dec((temp >> 16) & 0X3F);
    *min = rtc_bcd2dec((temp >> 8) & 0X7F);
    *sec = rtc_bcd2dec(temp & 0X7F);
    *ampm = temp >> 22;
}

/**
 * @brief       ��ȡRTC����
 * @param       *year,*mon,*date: ��,��,��
 * @param       *week           : ����
 * @retval      ��
 */
void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week)
{
    uint32_t temp = 0;

    while (rtc_wait_synchro());	/* �ȴ�ͬ�� */

    temp = RTC->DR;
    *year = rtc_bcd2dec((temp >> 16) & 0XFF);
    *month = rtc_bcd2dec((temp >> 8) & 0X1F);
    *date = rtc_bcd2dec(temp & 0X3F);
    *week = (temp >> 13) & 0X07;
}

/**
 * @brief       RTC��ʼ��
 *   @note 
 *              Ĭ�ϳ���ʹ��LSE,��LSE����ʧ�ܺ�,�л�ΪLSI.
 *              ͨ��BKP�Ĵ���0��ֵ,�����ж�RTCʹ�õ���LSE/LSI:
 *              ��BKP0==0X5050ʱ,ʹ�õ���LSE
 *              ��BKP0==0X5051ʱ,ʹ�õ���LSI
 *              ע��:�л�LSI/LSE������ʱ��/���ڶ�ʧ,�л�������������.
 *
 * @param       ��
 * @retval      0,�ɹ�
 *              1,�����ʼ��ģʽʧ��
 */
uint8_t rtc_init(void)
{
    uint16_t ssr;
    uint16_t bkpflag = 0;
    uint16_t retry = 200;
    uint32_t tempreg = 0;
 
    PWR->CR1 |= 1 << 8;         /* DBP=1,������дʹ�� */
    
    bkpflag = rtc_read_bkr(0);  /* ��ȡBKP0��ֵ */

    if (bkpflag != 0X5050)      /* ֮ǰʹ�õĲ���LSE */
    {
        RCC->CSR |= 1 << 0;     /* LSI����ʹ�� */

        while (!(RCC->CSR & 0x02)); /* �ȴ�LSI���� */

        RCC->BDCR |= 1 << 0;    /* ���Կ���LSE */

        while (retry && ((RCC->BDCR & 0X02) == 0))  /* �ȴ�LSE׼���� */
        {
            retry--;
            delay_ms(5);
        }

        tempreg = RCC->BDCR;    /* ��ȡBDCR��ֵ */
        tempreg &= ~(3 << 8);   /* ����8/9λ */

        if (retry == 0)tempreg |= 1 << 9;   /* LSE����ʧ��,����LSI. */
        else tempreg |= 1 << 8; /* ѡ��LSE,��ΪRTCʱ�� */

        tempreg |= 1 << 15;     /* ʹ��RTCʱ�� */
        RCC->BDCR = tempreg;    /* ��������BDCR�Ĵ��� */
        /* �ر�RTC�Ĵ���д���� */
        RTC->WPR = 0xCA;
        RTC->WPR = 0x53;
        RTC->CR = 0;

        if (rtc_init_mode())
        {
            RCC->BDCR = 1 << 16;/* ��λBDCR */
            delay_ms(10);
            RCC->BDCR = 0;      /*  ������λ */
            return 2;           /* ����RTC��ʼ��ģʽʧ�� */
        }

        RTC->PRER = 0XFF;       /* RTCͬ����Ƶϵ��(0~7FFF),����������ͬ����Ƶ,�������첽��Ƶ,Frtc=Fclks/((Sprec+1)*(Asprec+1)) */
        RTC->PRER |= 0X7F << 16;/* RTC�첽��Ƶϵ��(1~0X7F) */
        RTC->CR &= ~(1 << 6);   /* RTC����Ϊ,24Сʱ��ʽ */
        RTC->ISR &= ~(1 << 7);  /* �˳�RTC��ʼ��ģʽ */
        RTC->WPR = 0xFF;        /* ʹ��RTC�Ĵ���д���� */

        if (bkpflag != 0X5051)  /* BKP0�����ݼȲ���0X5050,Ҳ����0X5051,˵���ǵ�һ������,��Ҫ����ʱ������. */
        {
            rtc_set_time(23, 59, 56, 0);/* ����ʱ�� */
            rtc_set_date(20, 1, 13, 7); /* �������� */
            //rtc_set_alarma(7,0,0,10); /* ��������ʱ�� */
        }

        if (retry == 0)
        {
            rtc_write_bkr(0, 0X5051);   /* ����Ѿ���ʼ������,ʹ��LSI */
        }
        else 
        {
            rtc_write_bkr(0, 0X5050);   /* ����Ѿ���ʼ������,ʹ��LSE */
        }
    }
    else
    {
        retry = 10;     /* ����10��SSR��ֵ��û�仯,��LSE����. */
        ssr = RTC->SSR; /* ��ȡ��ʼֵ */

        while (retry)   /* ���ssr�Ĵ����Ķ�̬,���ж�LSE�Ƿ����� */
        {
            delay_ms(10);

            if (ssr == RTC->SSR)        /* �Ա� */
            {
                retry--;
            }
            else 
            {
                break;
            }
        }

        if (retry == 0) /* LSE����,������õȴ��´ν����������� */
        {
            rtc_write_bkr(0, 0XFFFF);   /* ��Ǵ����ֵ */
            RCC->BDCR = 1 << 16;        /* ��λBDCR */
            delay_ms(10);
            RCC->BDCR = 0;              /* ������λ */
        }
    }

    //rtc_set_wakeup(4,0);  /* ����WAKE UP�ж�,1�����ж�һ�� */
    return 0;
}

/**
 * @breif       ��������ʱ��(����������,24Сʱ��)
 * @param       week        : ���ڼ�(1~7) 
 * @param       hour,min,sec: Сʱ,����,����
 * @retval      ��
 */
void rtc_set_alarma(uint8_t week, uint8_t hour, uint8_t min, uint8_t sec)
{
    /* �ر�RTC�Ĵ���д���� */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR &= ~(1 << 8);       /* �ر�����A */

    while ((RTC->ISR & 0X01) == 0); /* �ȴ�����A�޸����� */

    RTC->ALRMAR = 0;            /* ���ԭ������ */
    RTC->ALRMAR |= 1 << 30;     /* ���������� */
    RTC->ALRMAR |= 0 << 22;     /* 24Сʱ�� */
    RTC->ALRMAR |= (uint32_t)rtc_dec2bcd(week) << 24;   /* �������� */
    RTC->ALRMAR |= (uint32_t)rtc_dec2bcd(hour) << 16;   /* Сʱ���� */
    RTC->ALRMAR |= (uint32_t)rtc_dec2bcd(min) << 8;     /* �������� */
    RTC->ALRMAR |= (uint32_t)rtc_dec2bcd(sec);          /* �������� */
    RTC->ALRMASSR = 0;          /* ��ʹ��SUB SEC */
    RTC->CR |= 1 << 12;         /* ��������A�ж� */
    RTC->CR |= 1 << 8;          /* ��������A */
    RTC->WPR = 0XFF;            /* ��ֹ�޸�RTC�Ĵ��� */

    RTC->ISR &= ~(1 << 8);      /* ���RTC����A�ı�־ */
    EXTI_D1->PR1 = 1 << 17;     /* ���LINE17�ϵ��жϱ�־λ */
    EXTI_D1->IMR1 |= 1 << 17;   /* ����line17�ϵ��ж� */
    EXTI->RTSR1 |= 1 << 17;     /* line17���¼��������ش��� */
    sys_nvic_init(2, 2, RTC_Alarm_IRQn, 2); /* ��ռ2�������ȼ�2����2 */
}

/**
 * @breif       �����Ի��Ѷ�ʱ������
 * @param       wksel
 *   @arg       000,RTC/16;001,RTC/8;010,RTC/4;011,RTC/2;
 *   @arg       10x,ck_spre,1Hz;11x,1Hz,��cntֵ����2^16(��cnt+2^16)
 * @note        ע��:RTC����RTC��ʱ��Ƶ��,��RTCCLK!
 * @param       cnt: �Զ���װ��ֵ.����0,�����ж�.
 * @retval      ��
 */
void rtc_set_wakeup(uint8_t wksel, uint16_t cnt)
{
    /* �ر�RTC�Ĵ���д���� */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR &= ~(1 << 10);      /* �ر�WAKE UP */

    while ((RTC->ISR & 0X04) == 0);	/* �ȴ�WAKE UP�޸����� */

    RTC->CR &= ~(7 << 0);       /* ���ԭ�������� */
    RTC->CR |= wksel & 0X07;    /* �����µ�ֵ */
    RTC->WUTR = cnt;            /* ����WAKE UP�Զ���װ�ؼĴ���ֵ */
    RTC->ISR &= ~(1 << 10);     /* ���RTC WAKE UP�ı�־ */
    RTC->CR |= 1 << 14;         /* ����WAKE UP ��ʱ���ж� */
    RTC->CR |= 1 << 10;         /* ����WAKE UP ��ʱ�� */
    RTC->WPR = 0XFF;            /* ��ֹ�޸�RTC�Ĵ��� */
    EXTI_D1->PR1 = 1 << 19;     /* ���LINE19�ϵ��жϱ�־λ */
    EXTI_D1->IMR1 |= 1 << 19;   /* ����line19�ϵ��ж� */
    EXTI->RTSR1 |= 1 << 19;     /* line19���¼��������ش��� */
    sys_nvic_init(2, 2, RTC_WKUP_IRQn, 2);  /* ��ռ2�������ȼ�2����2 */
}

/**
 * @breif       RTC�����жϷ�����
 * @param       ��
 * @retval      ��
 */
void RTC_Alarm_IRQHandler(void)
{
    OSIntEnter();
    if (RTC->ISR & (1 << 8))    /* ALARM A�ж�? */
    {
        RTC->ISR &= ~(1 << 8);  /* ����жϱ�־ */
        printf("ALARM A!\r\n");
        alarm.ringsta |= (1 << 7);
    }

    EXTI_D1->PR1 |= 1 << 17;    /* ����ж���17���жϱ�־ */
    OSIntExit();
}

/**
 * @breif       RTC WAKE UP�жϷ�����
 * @param       ��
 * @retval      ��
 */
void RTC_WKUP_IRQHandler(void)
{ 
    OSIntEnter();
    if (RTC->ISR & (1 << 10))   /* WK_UP�ж�? */
    {
        RTC->ISR &= ~(1 << 10); /* ����жϱ�־ */
        LED1_TOGGLE();
    }

    EXTI_D1->PR1 |= 1 << 19;    /* ����ж���19���жϱ�־ */
    OSIntExit();
}

/* ���������ݱ� */
uint8_t const table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};

/**
 * @breif       ������������ڼ�, ���빫�����ڵõ�����(ֻ����1901-2099��)
 * @param       year,month,day������������
 * @retval      ���ں�(1~7,������1~����)
 */
uint8_t rtc_get_week(uint16_t year, uint8_t month, uint8_t day)
{
    uint16_t temp2;
    uint8_t yearH, yearL;
    yearH = year / 100;
    yearL = year % 100;

    /*  ���Ϊ21����,�������100 */
    if (yearH > 19)yearL += 100;

    /*  ����������ֻ��1900��֮��� */
    temp2 = yearL + yearL / 4;
    temp2 = temp2 % 7;
    temp2 = temp2 + day + table_week[month - 1];

    if (yearL % 4 == 0 && month < 3)temp2--;

    temp2 %= 7;

    if (temp2 == 0)temp2 = 7;

    return temp2;
}













