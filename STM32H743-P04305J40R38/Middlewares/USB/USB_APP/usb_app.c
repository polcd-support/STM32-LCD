/**
 ****************************************************************************************************
 * @file        usb_app.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-11-29
 * @brief       USB-APP ����
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
 * V1.1 20221129
 * ���usbapp_check_enumedead������USBö���������¿���������
 *
 ****************************************************************************************************
 */

#include "usb_app.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "./FATFS/exfuns/exfuns.h"
#include "os.h"


USBH_HandleTypeDef  hUSBHost;       /* USB Host����ṹ�� */
USBD_HandleTypeDef  hUSBDevice;     /* USB Device����ṹ�� */

_usb_app usbx;                      /* USB APP������ */


/**
 * @brief       USB OTG �жϷ�����
 * @param       ��
 * @retval      ��
 */
void OTG_FS_IRQHandler(void)
{
    OSIntEnter();

    if (usbx.mode == 2) /* USB�ӻ�ģʽ��USB�������� */
    {
        HAL_PCD_IRQHandler(&g_hpcd);
    }
    else                /* USB����ģʽ */
    {
        HAL_HCD_IRQHandler(&g_hhcd);
    }

    OSIntExit();
}

/**
 * @brief       USB����ͨ�Ŵ�����
 * @param       phost  : USBH���ָ��
 * @param       id     : USBH��ǰ���û�״̬
 * @retval      ��
 */
void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    uint8_t dev_type = 0XFF;    /* �豸����,1,����;2,���;����,��֧�ֵ��豸 */

    switch (id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            break;

        case HOST_USER_CLASS_ACTIVE:        /* ��ִ�е���һ����˵��USB HOST�Բ����豸�Ѿ�ʶ��ɹ� */
            usbx.bDeviceState |= 0X80;      /* ��������� */

            if (usbx.mode == USBH_HID_MODE) /* HIDģʽ��,���Խ���:���/����/�ֱ� */
            {
                dev_type = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bInterfaceProtocol;

                if (dev_type == HID_KEYBRD_BOOT_CODE)       /* �����豸 */
                {
                    usbx.hdevclass = 3;     /* ��⵽���̲��� */
                }
                else if (dev_type == HID_MOUSE_BOOT_CODE)   /* ����豸 */
                {
                    usbx.hdevclass = 2;     /* ��⵽������ */
                }
                else if (dev_type == 0)     /* �ֱ��豸 */
                {
                    usbx.hdevclass = 4;     /* ��⵽�ֱ����� */
                }
                else usbx.hdevclass = 0;    /* ��֧�ֵ��豸 */
            }
            else usbx.hdevclass = 1;        /* ��⵽U�̲��� */

            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }
}

/**
 * @brief       USB����/�����ʾdemo���ݴ���
 * @param       phost  : USBH���ָ��
 * @retval      ��
 */
void USB_Demo(USBH_HandleTypeDef *phost)
{
    if (usbx.hdevclass == 3)            /* �����豸 */
    {
        USBH_HID_GetKeybdInfo(phost);   /* ת����ASCII�� */
    }
    else if (usbx.hdevclass == 4)       /* USB��Ϸ�ֱ��豸 */
    {
        USBH_HID_GetGamePadInfo(phost); /* ��ȡ�ֱ���Ϣ */
    }
}

/**
 * @brief       ��ʼ��USB
 * @param       ��
 * @retval      ��
 */
void usbapp_init(void)
{
    usbx.bDeviceState = 0;
    usbx.hdevclass = 0;
    usbx.mode = 0XFF;   /* ����Ϊһ���Ƿ���ģʽ,�����ȵ���usbapp_mode_set����ģʽ,������������ */
}

/**
 * @brief       USBö��״̬�������,��ֹUSBö��ʧ�ܵ��µ�����
 * @param       phost  : USBH���ָ��
 * @retval      0,û������
 *              1,������,�ⲿ������������USB����.
 */
uint8_t usbapp_check_enumedead(USBH_HandleTypeDef *phost)
{
    static uint16_t errcnt = 0;

    /* ���״̬,�����������,��˵��USB������ */
    if (phost->gState == HOST_ENUMERATION && (phost->EnumState == ENUM_IDLE || phost->EnumState == ENUM_GET_FULL_DEV_DESC))
    {
        errcnt++;

        if (errcnt > 2000)  /* ������ */
        {
            //printf("usb dead\r\n");
            errcnt = 0;
            RCC->AHB2RSTR |= 1 << 7;    /* USB OTG FS ��λ */
            delay_ms(5);
            RCC->AHB2RSTR &= ~(1 << 7); /* ��λ���� */
            return 1;
        }
    }
    else errcnt = 0;

    return 0;
}

/**
 * @brief       USB��ѯ����
 *  @note       �˺������������Եı�����
 * @param       ��
 * @retval      ��
 */
void usbapp_pulling(void)
{
    switch (usbx.mode)
    {
        case USBH_MSC_MODE:     /* USB HOST U�������� */
            USBH_Process(&hUSBHost);

            if (hUSBHost.gState == HOST_ABORT_STATE)    /* ��ֹ�����ֱ�/����,����usbapp_pulling�Ͳ�ѯ */
            {
                usbx.bDeviceState = 0;
                usbx.hdevclass = 0;
            }

            break;

        case USBH_HID_MODE:     /* USB HOST HID������ */
            USBH_Process(&hUSBHost);
            USB_Demo(&hUSBHost);
            break;

        case USBD_MSC_MODE:
            break;
    }
    
    if (usbapp_check_enumedead(&hUSBHost))  /* ���USB HOST �Ƿ�������?������,�����³�ʼ�� */
    {
        usbapp_mode_set(usbx.mode);         /* ���� */
    }
            
}

/**
 * @brief       USB������ǰ����ģʽ
 * @param       ��
 * @retval      ��
 */
void usbapp_mode_stop(void)
{
    USBH_ClassTypeDef *pac =  hUSBHost.pActiveClass;

    switch (usbx.mode)
    {
        case USBH_MSC_MODE:
            if (pac)USBH_MSC_InterfaceDeInit(&hUSBHost);    /* ��λUSB HOST MSC�� */

            USBH_DeInit(&hUSBHost);     /* ��λUSB HOST */
            break;

        case USBH_HID_MODE:
            if (pac)USBH_HID_InterfaceDeInit(&hUSBHost);    /* ��λUSB HOST HID�� */

            USBH_DeInit(&hUSBHost);     /* ��λUSB HOST */
            break;

        case USBD_MSC_MODE:
            USBD_MSC_DeInit(&hUSBDevice, 0);    /* ��λUSB SLAVE MSC�� */
            USBD_DeInit(&hUSBDevice);           /* ��λUSB SLAVE */
            break;
    }

    RCC->AHB2RSTR |= 1 << 7;        /* USB OTG FS ��λ */
    delay_ms(5);
    RCC->AHB2RSTR &= ~(1 << 7);    /* ��λ���� */
    memset(&hUSBHost, 0, sizeof(USBH_HandleTypeDef));
    memset(&hUSBDevice, 0, sizeof(USBD_HandleTypeDef));
    usbx.mode = 0XFF;
    usbx.bDeviceState = 0;
    usbx.hdevclass = 0;
}

#include "usbd_desc.h"

/**
 * @brief       ����USB����ģʽ
 * @param       mode   : ����ģʽ
 *                       0,USB HOST MSCģʽ(Ĭ��ģʽ,��U��)
 *                       1,USB HOST HIDģʽ(���������̵�)
 *                       2,USB Device MSCģʽ(USB������)
 * @retval      ��
 */
void usbapp_mode_set(uint8_t mode)
{
    usbapp_mode_stop();/* ��ֹͣ��ǰUSB����ģʽ */
    usbx.mode = mode;

    switch (usbx.mode)
    {
        case USBH_MSC_MODE:
            USBH_Init(&hUSBHost, USBH_UserProcess, 0);      /* ��ʼ��USB HOST */
            USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);  /* ���USB HOST MSC�� */
            USBH_Start(&hUSBHost);                          /* ����USB HOST */
            break;

        case USBH_HID_MODE:
            USBH_Init(&hUSBHost, USBH_UserProcess, 0);      /* ��ʼ��USB HOST */
            USBH_RegisterClass(&hUSBHost, USBH_HID_CLASS);  /* ���USB HOST HID�� */
            USBH_Start(&hUSBHost);                          /* ����USB HOST */
            break;

        case USBD_MSC_MODE:
            USBD_Init(&hUSBDevice, &MSC_Desc, 0);           /* ��ʼ��USB SLAVE */
            USBD_RegisterClass(&hUSBDevice, USBD_MSC_CLASS);/* ���USB SLAVE MSC�� */
            USBD_MSC_RegisterStorage(&hUSBDevice, &USBD_DISK_fops); /* ΪUSB SLAVE MSC����ӻص����� */
            USBD_Start(&hUSBDevice);                        /* ����USB SLAVE */
            break;
    }
}
