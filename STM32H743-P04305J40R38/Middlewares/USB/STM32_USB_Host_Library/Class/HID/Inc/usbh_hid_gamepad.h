/**
 ****************************************************************************************************
 * @file        usbh_hid_gamepad.h
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

#ifndef __USBH_HID_GAMEPAD_H
#define __USBH_HID_GAMEPAD_H

#include "usbh_hid.h"


/**
 * FC��Ϸ�ֱ����ݸ�ʽ����
 * 1,��ʾû�а���,0��ʾ����
 */
typedef union _FC_GamePad_TypeDef
{
    uint8_t ctrlval;
    struct
    {
        uint8_t a: 1;       /* A�� */
        uint8_t b: 1;       /* B�� */
        uint8_t select: 1;  /* Select�� */
        uint8_t start: 1;   /* Start�� */
        uint8_t up: 1;      /* �� */
        uint8_t down: 1;    /* �� */
        uint8_t left: 1;    /* �� */
        uint8_t right: 1;   /* �� */
    } b;
} FC_GamePad_TypeDef ;

extern FC_GamePad_TypeDef fcpad;    /* fc��Ϸ�ֱ�1 */
extern FC_GamePad_TypeDef fcpad1;   /* fc��Ϸ�ֱ�2 */




USBH_StatusTypeDef USBH_HID_GamePadInit(USBH_HandleTypeDef *phost);
void USBH_HID_GamePad_Decode(uint8_t *data);
FC_GamePad_TypeDef *USBH_HID_GetGamePadInfo(USBH_HandleTypeDef *phost);


void USR_GAMEPAD_Init(void);
void USR_GAMEPAD_ProcessData(uint8_t data);



#endif






















