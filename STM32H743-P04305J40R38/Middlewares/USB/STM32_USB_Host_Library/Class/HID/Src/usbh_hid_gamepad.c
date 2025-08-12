/**
 ****************************************************************************************************
 * @file        usbh_hid_gamepad.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-01
 * @brief       USB��Ϸ�ֱ� ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220601
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "usbh_hid_gamepad.h"
#include "usbh_hid_parser.h"
#include "./SYSTEM/usart/usart.h"


/**
 * FC��Ϸ�ֱ�ģ��
 * ��ȡfcpad.ctrlval,���ɵõ���ǰ�ֱ���ʽת����FC�ֱ���ʽ.
 */
FC_GamePad_TypeDef fcpad;
FC_GamePad_TypeDef fcpad1;


/* �ۺ�ʵ��ʱ, ��Ҫ����2�ֽڶ���, ����, ��������ַ������, �����ֱ���-O2�Ż���ʱ��ͻ����� */
#if !(__ARMCC_VERSION >= 6010050)   /* ����AC6����������ʹ��AC5������ʱ */
#define __ALIGNED_4     __align(4)  /* AC5ʹ����� */
#else                               /* ʹ��AC6������ʱ */
#define __ALIGNED_4     __ALIGNED(4) /* AC6ʹ����� */
#endif

__ALIGNED_4 uint8_t gamepad_report_data[HID_QUEUE_SIZE];    /* ����ϱ����ݳ���,���HID_QUEUE_SIZE���ֽ� */


/**
 * @brief       USBH ��Ϸ�ֱ���ʼ��
 * @param       phost       : USBH���ָ��
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_HID_GamePadInit(USBH_HandleTypeDef *phost)
{
    HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;

    if (HID_Handle->length > sizeof(gamepad_report_data))
    {
        HID_Handle->length = sizeof(gamepad_report_data);
    }

    HID_Handle->pData = (uint8_t *)gamepad_report_data;
    USBH_HID_FifoInit(&HID_Handle->fifo, phost->device.Data, HID_QUEUE_SIZE);
    return USBH_OK;
}

/**
 * @brief       USBH ��ȡ�ֱ���Ϣ
 * @param       phost       : USBH���ָ��
 * @retval      �ֱ���Ϣ(FC_GamePad_TypeDef)
 */
FC_GamePad_TypeDef *USBH_HID_GetGamePadInfo(USBH_HandleTypeDef *phost)
{
    uint8_t len = 0;
    HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

    if (HID_Handle->length == 0)return NULL;

    len = USBH_HID_FifoRead(&HID_Handle->fifo, &gamepad_report_data, HID_Handle->length);

    if (len == HID_Handle->length)  /* ��ȡFIFO */
    {
        USBH_HID_GamePad_Decode(gamepad_report_data);
        return &fcpad;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief       ͨ��USB���������,��������fcpad����
 * @param       data0,data1 : USB�õ��ķ�������
 *  @note                     data0:00,�������;ff,�Ҽ�����,7F,û�а�������
 *                            data1:00,�������;ff,�Ҽ�����,7F,û�а�������
 * @retval      ��
 */
void USBH_HID_GamePad_nDir_Decode(uint8_t data0, uint8_t data1)
{
    switch (data0)
    {
        case 0X7F:
            fcpad.b.left = 0;
            fcpad.b.right = 0;
            break;

        case 0X00:
            fcpad.b.left = 1;
            break;

        case 0XFF:
            fcpad.b.right = 1;
            break;
    }

    switch (data1)
    {
        case 0X7F:
            fcpad.b.up = 0;
            fcpad.b.down = 0;
            break;

        case 0X00:
            fcpad.b.up = 1;
            break;

        case 0XFF:
            fcpad.b.down = 1;
            break;
    }
}

/**
 * @brief       ͨ��USB AB������,��������fcpad����
 * @param       data        : USB�õ���1/2/3/4������
 *  @note                     ���ݸ�ʽ����:
 *                            data:���4λ��Ч
 *                            b4=1,1������(FC:B������)
 *                            b5=1,2������(FC:A������)
 *                            b6=1,3������(FC:A��)
 *                            b7=1,4������(FC:B��)
 * @retval      ��
 */
void USBH_HID_GamePad_nAB_Decode(uint8_t data)
{
    if (data & 0X10)fcpad.b.b = !fcpad.b.b; /* B��ȡ�� */
    else
    {
        if (data & 0X80)fcpad.b.b = 1;      /* B������ */
        else fcpad.b.b = 0;                 /* B���ɿ� */
    }

    if (data & 0X20)fcpad.b.a = !fcpad.b.a; /* A��ȡ�� */
    else
    {
        if (data & 0X40)fcpad.b.a = 1;      /* A������ */
        else fcpad.b.a = 0;                 /* A���ɿ� */
    }
}

/**
 * @brief       ͨ��USB ���ܼ�����,��������fcpad����
 * @param       data        : USB�õ���Select/Start������
 *  @note                     ���ݸ�ʽ����:
 *                            data:b4,b5��Ч.
 *                            b4=1,Select������
 *                            b5=1,Start������
 * @retval      ��
 */
void USBH_HID_GamePad_nFun_Decode(uint8_t data)
{
    if (data & 0X10)fcpad.b.select = 1; /* Select������ */
    else fcpad.b.select = 0;            /* Select���ɿ� */

    if (data & 0X20)fcpad.b.start = 1;  /* Start������ */
    else fcpad.b.start = 0;             /* Start���ɿ� */
}

/**
 * @brief       Game Pad ���ݽ���
 * @param       data        : ��������
 * @retval      ��
 */
void USBH_HID_GamePad_Decode(uint8_t *data)
{
    //uint8_t i;
    if (data[2] == 0X7F)    /* ��ҡ���ֱ��ֱ�,�������ֽ�Ϊ0X7F */
    {
        USBH_HID_GamePad_nDir_Decode(data[3], data[4]); /* ���뷽��� */
        USBH_HID_GamePad_nAB_Decode(data[5]);           /* ����AB�� */
        USBH_HID_GamePad_nFun_Decode(data[6]);          /* ���빦�ܼ� */
        //USR_GAMEPAD_ProcessData(fcpad.ctrlval);       /* ��ʾ��� */
    }
    else    /* ��ҡ�˵��ֱ� */
    {
        USBH_HID_GamePad_nDir_Decode(data[0], data[1]); /* ���뷽��� */
        USBH_HID_GamePad_nAB_Decode(data[5]);           /* ����AB�� */
        USBH_HID_GamePad_nFun_Decode(data[6]);          /* ���빦�ܼ� */
        //USR_GAMEPAD_ProcessData(fcpad.ctrlval);       /* ��ʾ��� */
    }

    //for(i=0;i<8;i++)printf("%02x ",data[i]);
    //printf("\r\n");
}

    
uint8_t *const JOYPAD_SYMBOL_TBL[8] =
{"Right", "Left", "Down", "Up", "Start", "Select", "A", "B"}; /* �ֱ��������Ŷ��� */

/**
 * @brief       USB�������ݴ���
 * @param       data        : ��������
 * @retval      ��
 */
void  USR_GAMEPAD_ProcessData(uint8_t data)
{
    uint8_t i;

    if (data)
    {
        for (i = 0; i < 8; i++)
        {
            if (data & (0X80 >> i))
            {
                printf("key:%s\r\n", (uint8_t *)JOYPAD_SYMBOL_TBL[i]);
            }
        }
    }
}



















