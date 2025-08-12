/**
 ****************************************************************************************************
 * @file        httpd_cgi_ssi.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-05
 * @brief       lwipͨ������ ����
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221205
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "lwip_comm.h"
#include "./BSP/LED/led.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/ADC/adc3.h"
#include "./BSP/RTC/rtc.h"
#include "./BSP/LCD/lcd.h"
#include <string.h>
#include <stdlib.h>


#define NUM_CONFIG_CGI_URIS     2
#define NUM_CONFIG_SSI_TAGS     4

extern short adc_get_temperature(void);   /* ����Get_Temperate()���� */
extern void rtc_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *ampm);     /* ����RTC_Get_Timer()���� */
extern void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week);  /* ����rtc_get_date()���� */

/* ����LED��CGI handler */
const char *LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *BEEP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

static const char *ppcTAGs[] =            /* SSI��Tag */
{
    "t",    /* ADCֵ */
    "w",    /* �¶�ֵ */
    "h",    /* ʱ�� */
    "y"     /* ���� */
};

static const tCGI ppcURLs[] =              /* cgi���� */
{
    {"/leds.cgi", LEDS_CGI_Handler},
    {"/beep.cgi", BEEP_CGI_Handler},
};

/**
 * @brief       ����CGI����
 *  @note       ��web�ͻ��������������ʱ��,ʹ�ô˺�����CGI handler����
 * @param       pcToFind        : ���������ݻ�����
 * @param       pcParam         : ���ҵĲ����ϼ�
 * @param       iNumParams      : ��������
 * @retval      -1, ʧ��;
 *              ����, �ɹ�(�ҵ���λ��)
 */
static int FindCGIParameter(const char *pcToFind, char *pcParam[], int iNumParams)
{
    int iLoop;

    for (iLoop = 0; iLoop < iNumParams; iLoop ++ )
    {
        if (strcmp(pcToFind, pcParam[iLoop]) == 0)
        {
            return (iLoop);     /* ����iLOOP */
        }
    }

    return (-1);
}

/**
 * @brief       SSIHandler����Ҫ�õ��Ĵ���ADC�ĺ���
 * @param       pcInsert        : �������ݻ�����
 * @retval      ��
 */
void ADC_Handler(char *pcInsert)
{
    char Digit1 = 0, Digit2 = 0, Digit3 = 0, Digit4 = 0;
    uint32_t ADCVal = 0;

    /* ��ȡADC��ֵ */
    ADCVal = adc_get_result_average(ADC_ADCX_CHY, 10); /* ��ȡADC1_CH5�ĵ�ѹֵ */


    /* ת��Ϊ��ѹ ADCVval * 0.8mv */
    ADCVal = (uint32_t)(ADCVal * 0.8);

    Digit1 = ADCVal / 1000;
    Digit2 = (ADCVal - (Digit1 * 1000)) / 100 ;
    Digit3 = (ADCVal - ((Digit1 * 1000) + (Digit2 * 100))) / 10;
    Digit4 = ADCVal - ((Digit1 * 1000) + (Digit2 * 100) + (Digit3 * 10));

    /* ׼����ӵ�html�е����� */
    *pcInsert       = (char)(Digit1 + 0x30);
    *(pcInsert + 1) = (char)(Digit2 + 0x30);
    *(pcInsert + 2) = (char)(Digit3 + 0x30);
    *(pcInsert + 3) = (char)(Digit4 + 0x30);
}

/**
 * @brief       SSIHandler����Ҫ�õ��Ĵ����ڲ��¶ȴ������ĺ���
 * @param       pcInsert        : �������ݻ�����
 * @retval      ��
 */
void Temperate_Handler(char *pcInsert)
{
    char Digit1 = 0, Digit2 = 0, Digit3 = 0, Digit4 = 0, Digit5 = 0;
    short Temperate = 0;
    /* ��ȡ�ڲ��¶�ֵ */
    Temperate = adc3_get_temperature(); /* ��ȡ�¶�ֵ �˴�������100�� */
    Digit1 = Temperate / 10000;
    Digit2 = (Temperate % 10000) / 1000;
    Digit3 = (Temperate % 1000) / 100 ;
    Digit4 = (Temperate % 100) / 10;
    Digit5 = Temperate % 10;
    /* ��ӵ�html�е����� */
    *pcInsert       = (char)(Digit1 + 0x30);
    *(pcInsert + 1) = (char)(Digit2 + 0x30);
    *(pcInsert + 2) = (char)(Digit3 + 0x30);
    *(pcInsert + 3) = '.';
    *(pcInsert + 4) = (char)(Digit4 + 0x30);
    *(pcInsert + 5) = (char)(Digit5 + 0x30);
}

/**
 * @brief       SSIHandler����Ҫ�õ��Ĵ���RTCʱ��ĺ���
 * @param       pcInsert        : �������ݻ�����
 * @retval      ��
 */
void RTCTime_Handler(char *pcInsert)
{
    uint8_t hour, min, sec, ampm;

    rtc_get_time(&hour, &min, &sec, &ampm);

    *pcInsert = (char)((hour / 10) + 0x30);
    *(pcInsert + 1) = (char)((hour % 10) + 0x30);
    *(pcInsert + 2) = ':';
    *(pcInsert + 3) = (char)((min / 10) + 0x30);
    *(pcInsert + 4) = (char)((min % 10) + 0x30);
    *(pcInsert + 5) = ':';
    *(pcInsert + 6) = (char)((sec / 10) + 0x30);
    *(pcInsert + 7) = (char)((sec % 10) + 0x30);
}

/**
 * @brief       SSIHandler����Ҫ�õ��Ĵ���RTC���ڵĺ���
 * @param       pcInsert        : �������ݻ�����
 * @retval      ��
 */
void RTCdate_Handler(char *pcInsert)
{
    uint8_t year, month, date, week;
    rtc_get_date(&year, &month, &date, &week);

    *pcInsert = '2';
    *(pcInsert + 1) = '0';
    *(pcInsert + 2) = (char)((year / 10) + 0x30);
    *(pcInsert + 3) = (char)((year % 10) + 0x30);
    *(pcInsert + 4) = '-';
    *(pcInsert + 5) = (char)((month / 10) + 0x30);
    *(pcInsert + 6) = (char)((month % 10) + 0x30);
    *(pcInsert + 7) = '-';
    *(pcInsert + 8) = (char)((date / 10) + 0x30);
    *(pcInsert + 9) = (char)((date % 10) + 0x30);
    *(pcInsert + 10) = ' ';
    *(pcInsert + 11) = 'w';
    *(pcInsert + 12) = 'e';
    *(pcInsert + 13) = 'e';
    *(pcInsert + 14) = 'k';
    *(pcInsert + 15) = ':';
    *(pcInsert + 16) = (char)(week + 0x30);

}

/**
 * @brief       SSI��Handler���
 * @param       pcInsert        : �������ݻ�����
 * @param       iInsertLen      : �������ݻ���������
 * @retval      ��
 */
static u16_t SSIHandler(int iIndex, char *pcInsert, int iInsertLen)
{
    switch (iIndex)
    {
        case 0:
            ADC_Handler(pcInsert);
            break;

        case 1:
            Temperate_Handler(pcInsert);
            break;

        case 2:
            RTCTime_Handler(pcInsert);
            break;

        case 3:
            RTCdate_Handler(pcInsert);
            break;
    }

    return strlen(pcInsert);
}

/**
 * @brief       CGI LED���ƾ������
 * @param       iIndex          : ������
 * @param       iNumParams      : �����������
 * @param       pcParam         : �����ϼ�
 * @param       pcValue         : ����ֵ�ϼ�
 * @retval      ��ҳ�ַ���
 */
const char *LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t i = 0; /* ע������Լ���GET�Ĳ����Ķ�����ѡ��iֵ��Χ */
    uint8_t led1_sta = 0;
    uint8_t beep_sta = 0;

    iIndex = FindCGIParameter("LED1", pcParam, iNumParams); /* �ҵ�led�������� */

    /* ֻ��һ��CGI��� iIndex=0 */
    if (iIndex != -1)
    {
        LED1(1);    /* �ر�LED1�� */


        for (i = 0; i < iNumParams; i++)               /* ���CGI����: example GET /leds.cgi?led=2&led=4 */
        {
            if (strcmp(pcParam[i], "LED1") == 0)       /* ������"led" */
            {
                if (strcmp(pcValue[i], "LED1ON") == 0) /* �ı�LED1״̬ */
                    LED1(0);                           /* ��LED1 */
                else if (strcmp(pcValue[i], "LED1OFF") == 0)
                    LED1(1);                            /* �ر�LED1 */
            }
        }
    }
    

    beep_sta = pcf8574_read_bit(BEEP_IO);
    
    if (led1_sta == 0 && beep_sta == 0)         return "/STM32_LED_ON_BEEP_OFF.shtml";  /* LED1��,BEEP�� */
    else if (led1_sta == 0 && beep_sta == 1)    return "/STM32_LED_ON_BEEP_ON.shtml";   /* LED1��,BEEP�� */
    else if (led1_sta == 1 && beep_sta == 1)    return "/STM32_LED_OFF_BEEP_ON.shtml";  /* LED1��,BEEP�� */
    else return "/STM32_LED_OFF_BEEP_OFF.shtml";                                        /* LED1��,BEEP�� */
}

/**
 * @brief       BEEP��CGI���ƾ�� 
 * @param       iIndex          : ������
 * @param       iNumParams      : �����������
 * @param       pcParam         : �����ϼ�
 * @param       pcValue         : ����ֵ�ϼ�
 * @retval      ��ҳ�ַ���
 */
const char *BEEP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t i = 0;
    uint8_t led1_sta = 0;
    uint8_t beep_sta = 0;
    
    iIndex = FindCGIParameter("BEEP", pcParam, iNumParams);  /* �ҵ�BEEP�������� */

    if (iIndex != -1) /* �ҵ�BEEP������ */
    {
        pcf8574_write_bit(BEEP_IO,0);      /* �ر� */

        for (i = 0; i < iNumParams; i++)
        {
            if (strcmp(pcParam[i], "BEEP") == 0)             /* ����CGI���� */
            {
                if (strcmp(pcValue[i], "BEEPON") == 0)       /* ��BEEP */
                    pcf8574_write_bit(BEEP_IO,0);
                else if (strcmp(pcValue[i], "BEEPOFF") == 0) /* �ر�BEEP */
                    pcf8574_write_bit(BEEP_IO,1);
            }
        }
    }
    
    beep_sta =!pcf8574_read_bit(BEEP_IO);

    if (led1_sta == 0 && beep_sta == 0)         return "/STM32_LED_ON_BEEP_OFF.shtml";  /* LED1��,BEEP�� */
    else if (led1_sta == 0 && beep_sta == 1)    return "/STM32_LED_ON_BEEP_ON.shtml";   /* LED1��,BEEP�� */
    else if (led1_sta == 1 && beep_sta == 1)    return "/STM32_LED_OFF_BEEP_ON.shtml";  /* LED1��,BEEP�� */
    else return "/STM32_LED_OFF_BEEP_OFF.shtml";  /* LED1��,BEEP�� */

}

/**
 * @brief       SSI�����ʼ��
 * @param       ��
 * @retval      ��
 */
void httpd_ssi_init(void)
{
    /* �����ڲ��¶ȴ�������SSI��� */
    http_set_ssi_handler(SSIHandler, ppcTAGs, NUM_CONFIG_SSI_TAGS);
}

/**
 * @brief       CGI�����ʼ��
 * @param       ��
 * @retval      ��
 */
void httpd_cgi_init(void)
{
    /* ����CGI���LEDs control CGI) */
    http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}
