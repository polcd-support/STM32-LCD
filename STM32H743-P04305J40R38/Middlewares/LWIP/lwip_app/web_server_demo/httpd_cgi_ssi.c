/**
 ****************************************************************************************************
 * @file        httpd_cgi_ssi.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-05
 * @brief       lwip通用驱动 代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20221205
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
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

extern short adc_get_temperature(void);   /* 声明Get_Temperate()函数 */
extern void rtc_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *ampm);     /* 声明RTC_Get_Timer()函数 */
extern void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week);  /* 声明rtc_get_date()函数 */

/* 控制LED的CGI handler */
const char *LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *BEEP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

static const char *ppcTAGs[] =            /* SSI的Tag */
{
    "t",    /* ADC值 */
    "w",    /* 温度值 */
    "h",    /* 时间 */
    "y"     /* 日期 */
};

static const tCGI ppcURLs[] =              /* cgi程序 */
{
    {"/leds.cgi", LEDS_CGI_Handler},
    {"/beep.cgi", BEEP_CGI_Handler},
};

/**
 * @brief       查找CGI参数
 *  @note       当web客户端请求浏览器的时候,使用此函数被CGI handler调用
 * @param       pcToFind        : 带查找数据缓冲区
 * @param       pcParam         : 待找的参数合集
 * @param       iNumParams      : 参数个数
 * @retval      -1, 失败;
 *              其他, 成功(找到的位置)
 */
static int FindCGIParameter(const char *pcToFind, char *pcParam[], int iNumParams)
{
    int iLoop;

    for (iLoop = 0; iLoop < iNumParams; iLoop ++ )
    {
        if (strcmp(pcToFind, pcParam[iLoop]) == 0)
        {
            return (iLoop);     /* 返回iLOOP */
        }
    }

    return (-1);
}

/**
 * @brief       SSIHandler中需要用到的处理ADC的函数
 * @param       pcInsert        : 输入数据缓冲区
 * @retval      无
 */
void ADC_Handler(char *pcInsert)
{
    char Digit1 = 0, Digit2 = 0, Digit3 = 0, Digit4 = 0;
    uint32_t ADCVal = 0;

    /* 获取ADC的值 */
    ADCVal = adc_get_result_average(ADC_ADCX_CHY, 10); /* 获取ADC1_CH5的电压值 */


    /* 转换为电压 ADCVval * 0.8mv */
    ADCVal = (uint32_t)(ADCVal * 0.8);

    Digit1 = ADCVal / 1000;
    Digit2 = (ADCVal - (Digit1 * 1000)) / 100 ;
    Digit3 = (ADCVal - ((Digit1 * 1000) + (Digit2 * 100))) / 10;
    Digit4 = ADCVal - ((Digit1 * 1000) + (Digit2 * 100) + (Digit3 * 10));

    /* 准备添加到html中的数据 */
    *pcInsert       = (char)(Digit1 + 0x30);
    *(pcInsert + 1) = (char)(Digit2 + 0x30);
    *(pcInsert + 2) = (char)(Digit3 + 0x30);
    *(pcInsert + 3) = (char)(Digit4 + 0x30);
}

/**
 * @brief       SSIHandler中需要用到的处理内部温度传感器的函数
 * @param       pcInsert        : 输入数据缓冲区
 * @retval      无
 */
void Temperate_Handler(char *pcInsert)
{
    char Digit1 = 0, Digit2 = 0, Digit3 = 0, Digit4 = 0, Digit5 = 0;
    short Temperate = 0;
    /* 获取内部温度值 */
    Temperate = adc3_get_temperature(); /* 获取温度值 此处扩大了100倍 */
    Digit1 = Temperate / 10000;
    Digit2 = (Temperate % 10000) / 1000;
    Digit3 = (Temperate % 1000) / 100 ;
    Digit4 = (Temperate % 100) / 10;
    Digit5 = Temperate % 10;
    /* 添加到html中的数据 */
    *pcInsert       = (char)(Digit1 + 0x30);
    *(pcInsert + 1) = (char)(Digit2 + 0x30);
    *(pcInsert + 2) = (char)(Digit3 + 0x30);
    *(pcInsert + 3) = '.';
    *(pcInsert + 4) = (char)(Digit4 + 0x30);
    *(pcInsert + 5) = (char)(Digit5 + 0x30);
}

/**
 * @brief       SSIHandler中需要用到的处理RTC时间的函数
 * @param       pcInsert        : 输入数据缓冲区
 * @retval      无
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
 * @brief       SSIHandler中需要用到的处理RTC日期的函数
 * @param       pcInsert        : 输入数据缓冲区
 * @retval      无
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
 * @brief       SSI的Handler句柄
 * @param       pcInsert        : 输入数据缓冲区
 * @param       iInsertLen      : 输入数据缓冲区长度
 * @retval      无
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
 * @brief       CGI LED控制句柄函数
 * @param       iIndex          : 索引号
 * @param       iNumParams      : 输入参数个数
 * @param       pcParam         : 参数合集
 * @param       pcValue         : 参数值合集
 * @retval      网页字符串
 */
const char *LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t i = 0; /* 注意根据自己的GET的参数的多少来选择i值范围 */
    uint8_t led1_sta = 0;
    uint8_t beep_sta = 0;

    iIndex = FindCGIParameter("LED1", pcParam, iNumParams); /* 找到led的索引号 */

    /* 只有一个CGI句柄 iIndex=0 */
    if (iIndex != -1)
    {
        LED1(1);    /* 关闭LED1灯 */


        for (i = 0; i < iNumParams; i++)               /* 检查CGI参数: example GET /leds.cgi?led=2&led=4 */
        {
            if (strcmp(pcParam[i], "LED1") == 0)       /* 检查参数"led" */
            {
                if (strcmp(pcValue[i], "LED1ON") == 0) /* 改变LED1状态 */
                    LED1(0);                           /* 打开LED1 */
                else if (strcmp(pcValue[i], "LED1OFF") == 0)
                    LED1(1);                            /* 关闭LED1 */
            }
        }
    }
    

    beep_sta = pcf8574_read_bit(BEEP_IO);
    
    if (led1_sta == 0 && beep_sta == 0)         return "/STM32_LED_ON_BEEP_OFF.shtml";  /* LED1开,BEEP关 */
    else if (led1_sta == 0 && beep_sta == 1)    return "/STM32_LED_ON_BEEP_ON.shtml";   /* LED1开,BEEP开 */
    else if (led1_sta == 1 && beep_sta == 1)    return "/STM32_LED_OFF_BEEP_ON.shtml";  /* LED1关,BEEP开 */
    else return "/STM32_LED_OFF_BEEP_OFF.shtml";                                        /* LED1关,BEEP关 */
}

/**
 * @brief       BEEP的CGI控制句柄 
 * @param       iIndex          : 索引号
 * @param       iNumParams      : 输入参数个数
 * @param       pcParam         : 参数合集
 * @param       pcValue         : 参数值合集
 * @retval      网页字符串
 */
const char *BEEP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t i = 0;
    uint8_t led1_sta = 0;
    uint8_t beep_sta = 0;
    
    iIndex = FindCGIParameter("BEEP", pcParam, iNumParams);  /* 找到BEEP的索引号 */

    if (iIndex != -1) /* 找到BEEP索引号 */
    {
        pcf8574_write_bit(BEEP_IO,0);      /* 关闭 */

        for (i = 0; i < iNumParams; i++)
        {
            if (strcmp(pcParam[i], "BEEP") == 0)             /* 查找CGI参数 */
            {
                if (strcmp(pcValue[i], "BEEPON") == 0)       /* 打开BEEP */
                    pcf8574_write_bit(BEEP_IO,0);
                else if (strcmp(pcValue[i], "BEEPOFF") == 0) /* 关闭BEEP */
                    pcf8574_write_bit(BEEP_IO,1);
            }
        }
    }
    
    beep_sta =!pcf8574_read_bit(BEEP_IO);

    if (led1_sta == 0 && beep_sta == 0)         return "/STM32_LED_ON_BEEP_OFF.shtml";  /* LED1开,BEEP关 */
    else if (led1_sta == 0 && beep_sta == 1)    return "/STM32_LED_ON_BEEP_ON.shtml";   /* LED1开,BEEP开 */
    else if (led1_sta == 1 && beep_sta == 1)    return "/STM32_LED_OFF_BEEP_ON.shtml";  /* LED1关,BEEP开 */
    else return "/STM32_LED_OFF_BEEP_OFF.shtml";  /* LED1关,BEEP关 */

}

/**
 * @brief       SSI句柄初始化
 * @param       无
 * @retval      无
 */
void httpd_ssi_init(void)
{
    /* 配置内部温度传感器的SSI句柄 */
    http_set_ssi_handler(SSIHandler, ppcTAGs, NUM_CONFIG_SSI_TAGS);
}

/**
 * @brief       CGI句柄初始化
 * @param       无
 * @retval      无
 */
void httpd_cgi_init(void)
{
    /* 配置CGI句柄LEDs control CGI) */
    http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}
