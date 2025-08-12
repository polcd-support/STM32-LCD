/**
 ****************************************************************************************************
 * @file        usbh_conf.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-28
 * @brief       usbh_conf ��������
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
 * V1.0 20230328
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "usbh_conf.h"
#include "usbh_core.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "usb_app.h"


HCD_HandleTypeDef g_hhcd;

/**
 * @brief       ��ʼ��HSI48��CRSУ׼,ѡ��HSI48��ΪUSBʱ��Դ(��CRS�Զ�У׼)
 * @param       ��
 * @retval      ��
 */
void usbh_otg_hsi48_crs_init(void)
{
    RCC->CR |= 1 << 12;             /* HSI480N=1,ʹ��HSI48 */

    while ((RCC->CR & (1 << 13)) == 0); /* �ȴ�HSI48RDY=1,�ȴ�HSI48�ȶ� */

    RCC->APB1HENR |= 1 << 1;        /* CRSEN=1,ʹ��CRS */
    RCC->APB1HRSTR |= 1 << 1;       /* CRSRST=1,��λCRS */
    RCC->APB1HRSTR &= ~(1 << 1);    /* CRSRST=0,ȡ����λ */
    CRS->CFGR &= ~(3 << 28);        /* SYNCSRC[1:0]=0,ѡ��USB2 SOF��ΪSYNC�ź� */
    CRS->CR |= 3 << 5;              /* CEN��AUTIOTRIMEN��Ϊ1,ʹ���Զ�У׼�Լ������� */
    RCC->D2CCIP2R &= ~(3 << 20);    /* USBSEL[1:0]=0,����ԭ�������� */
    RCC->D2CCIP2R |= 3 << 20;       /* USBSEL[1:0]=3,USBʱ��Դ����hsi48_ck */
}

/**
 * @brief       ��ʼ��HCD MSP
 *   @note      ����һ���ص�����, ��stm32f4xx_hal_hcd.c�������
 * @param       hhcd        : HCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_HCD_MspInit(HCD_HandleTypeDef *hhcd)
{
    if (hhcd->Instance == USB_OTG_FS)
    {

        RCC->AHB4ENR |= 1 << 0;             /* ʹ��PORTAʱ�� */
        RCC->AHB1ENR |= 1 << 27;            /* ʹ��USB2 OTGʱ�� */
        PWR->CR3 |= 1 << 24;                /* ʹ��USB VDD3��ѹ��� */
        usbh_otg_hsi48_crs_init();          /* ����USBʱ������hsi48_ck,ʹ��CRS */

        sys_gpio_set(GPIOA, SYS_GPIO_PIN11,
                     SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PA11����ģʽ����,���ù��� */

        sys_gpio_set(GPIOA, SYS_GPIO_PIN12,
                     SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PA12����ģʽ����,���ù��� */

        sys_gpio_af_set(GPIOA, SYS_GPIO_PIN11, 10);     /* PA11, AF10(USB) */
        sys_gpio_af_set(GPIOA, SYS_GPIO_PIN12, 10);     /* PA12, AF10(USB) */

        pcf8574_write_bit(USB_PWR_IO, 0);               /* USB_PWR����ر�, �ر�USB HOST���� */
        delay_ms(500);
        pcf8574_write_bit(USB_PWR_IO, 1);               /* USB_PWR�������, �ָ�USB HOST���� */

        sys_nvic_init(0, 3, OTG_FS_IRQn, 2);            /* ���ȼ�����Ϊ��ռ0,�����ȼ�3����2 */
    }
    else if (hhcd->Instance == USB_OTG_HS)
    {
        /* USB OTG HS������û�õ�,�ʲ������� */
    }
}

/**
 * @brief       USB OTG �жϷ�����
 *   @note      ��������USB�ж�
 * @param       ��
 * @retval      ��
 */
//void OTG_FS_IRQHandler(void)
//{
//    HAL_HCD_IRQHandler(&g_hhcd);
//}

/******************************************************************************************/
/* ������: USBH LL HCD �����Ļص�����(HCD->USB Host Library) */

/**
 * @brief       USBH SOF�ص�����
 * @param       hhcd        : HCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd)
{
    USBH_LL_IncTimer(hhcd->pData);
}

/**
 * @brief       USBH ���ӳɹ��ص�����
 * @param       hhcd        : HCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd)
{
    usbx.bDeviceState |= 1 << 6;    /* ��⵽USB���� */
    printf("Connected!\r\n\r\n");
    USBH_LL_Connect(hhcd->pData);
}

/**
 * @brief       USBH �Ͽ����ӻص�����
 * @param       hhcd        : HCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd)
{
    usbx.bDeviceState = 0;  /* USB�γ� */
    usbx.hdevclass = 0;       /* �豸������� */
    printf("Disconnected!\r\n\r\n");
    USBH_LL_Disconnect(hhcd->pData);
}

/**
 * @brief       USBH �ӿ�ʹ�ܻص�����(V3.3.3 USB�������ص�����)
 * @param       hhcd        : HCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_HCD_PortEnabled_Callback(HCD_HandleTypeDef *hhcd)
{
    USBH_LL_PortEnabled(hhcd->pData);
}

/**
 * @brief       USBH �ӿڹرջص�����(V3.3.3 USB�������ص�����)
 * @param       hhcd        : HCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_HCD_PortDisabled_Callback(HCD_HandleTypeDef *hhcd)
{
    USBH_LL_PortDisabled(hhcd->pData);
}

/**
 * @brief       USBH ֪ͨURB�仯�ص�����
 * @param       hhcd        : HCD�ṹ��ָ��
 * @param       chnum       : �˵���
 * @param       urb_state   : URB״̬
 * @retval      ��
 */
void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state)
{
    /* To be used with OS to sync URB state with the global state machine */
}


/******************************************************************************************/
/* ������: USBH LL HCD �����ӿں���(HCD->USB Host Library) */

/**
 * @brief       USBH �ײ��ʼ������
 * @param       phost       : USBH���ָ��
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_Init(USBH_HandleTypeDef *phost)
{
#ifdef USE_USB_FS
    /* ����LL������ز��� */
    g_hhcd.Instance = USB2_OTG_FS;              /* ʹ��USB OTG */
    g_hhcd.Init.Host_channels = 11;             /* ����ͨ����Ϊ11�� */
    g_hhcd.Init.dma_enable = 0;                 /* ��ʹ��DMA */
    g_hhcd.Init.low_power_enable = 0;           /* ��ʹ�ܵ͹���ģʽ */
    g_hhcd.Init.phy_itface = HCD_PHY_EMBEDDED;  /* ʹ���ڲ�PHY */
    g_hhcd.Init.Sof_enable = 0;                 /* ��ֹSOF�ж� */
    g_hhcd.Init.speed = HCD_SPEED_FULL;         /* USBȫ��(12Mbps) */
    g_hhcd.Init.vbus_sensing_enable = 0;        /* ��ʹ��VBUS��� */
//    g_hhcd.Init.lpm_enable = 0;                 /* ʹ�����ӵ�Դ���� */

    g_hhcd.pData = phost;                       /* g_hhcd��pDataָ��phost */
    phost->pData = &g_hhcd;                     /* phost��pDataָ��g_hhcd */

    HAL_HCD_Init(&g_hhcd);                      /* ��ʼ��LL���� */
#endif

#ifdef USE_USB_HS
    /* δʵ�� */
#endif
    USBH_LL_SetTimer(phost, HAL_HCD_GetCurrentFrame(&g_hhcd));
    return USBH_OK;
}

/**
 * @brief       USBH �ײ�ȡ����ʼ��(�ָ�Ĭ�ϸ�λ״̬)����
 * @param       phost       : USBH���ָ��
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_DeInit(USBH_HandleTypeDef *phost)
{
    HAL_HCD_DeInit(phost->pData);
    return USBH_OK;
}

/**
 * @brief       USBH �ײ�������ʼ����
 * @param       phost       : USBH���ָ��
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_Start(USBH_HandleTypeDef *phost)
{
    HAL_HCD_Start(phost->pData);
    return USBH_OK;
}

/**
 * @brief       USBH �ײ�����ֹͣ����
 * @param       phost       : USBH���ָ��
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_Stop(USBH_HandleTypeDef *phost)
{
    HAL_HCD_Stop(phost->pData);
    return USBH_OK;
}

/**
 * @brief       USBH ��ȡUSB�豸���ٶ�
 * @param       phost       : USBH���ָ��
 * @retval      USBH�豸�ٶ�
 */
USBH_SpeedTypeDef USBH_LL_GetSpeed(USBH_HandleTypeDef *phost)
{
    USBH_SpeedTypeDef speed = USBH_SPEED_FULL;

    switch (HAL_HCD_GetCurrentSpeed(phost->pData))
    {
        case 0:
            speed = USBH_SPEED_HIGH;
            printf("USB Host [HS]\r\n");
            break;

        case 1:
            speed = USBH_SPEED_FULL;
            printf("USB Host [FS]\r\n");
            break;

        case 2:
            speed = USBH_SPEED_LOW;
            printf("USB Host [LS]\r\n");
            break;

        default:
            speed = USBH_SPEED_FULL;
            printf("USB Host [FS]\r\n");
            break;
    }

    return speed;
}

/**
 * @brief       USBH ��λUSB HOST�˿�
 * @param       phost       : USBH���ָ��
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_ResetPort(USBH_HandleTypeDef *phost)
{
    HAL_HCD_ResetPort(phost->pData);
    printf("USB Reset Port\r\n");
    return USBH_OK;
}

/**
 * @brief       USBH ��ȡ���һ�δ���İ���С
 * @param       phost       : USBH���ָ��
 * @param       pipe        : �ܵ����
 * @retval      ����С
 */
uint32_t USBH_LL_GetLastXferSize(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

/**
 * @brief       USBH ��һ���ܵ�(ͨ��)
 * @param       phost       : USBH���ָ��
 * @param       pipe        : �ܵ����
 * @param       epnum       : �˵��
 * @param       dev_address : �豸��ַ
 * @param       speed       : �豸�ٶ�
 * @param       ep_type     : �˵�����
 * @param       mps         : �˵�������С
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_OpenPipe(USBH_HandleTypeDef *phost,
                                    uint8_t pipe,
                                    uint8_t epnum,
                                    uint8_t dev_address,
                                    uint8_t speed,
                                    uint8_t ep_type, uint16_t mps)
{
    HAL_HCD_HC_Init(phost->pData, pipe, epnum, dev_address, speed, ep_type, mps);
    return USBH_OK;
}

/**
 * @brief       USBH �ر�һ���ܵ�(ͨ��)
 * @param       phost       : USBH���ָ��
 * @param       pipe        : �ܵ����
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_ClosePipe(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    HAL_HCD_HC_Halt(phost->pData, pipe);
    return USBH_OK;
}

/**
 * @brief       USBH �ύһ���µ�URB
 * @param       phost       : USBH���ָ��
 * @param       pipe        : �ܵ����
 * @param       direction   : ��������
 * @param       ep_type     : �˵�����
 * @param       token       : �˵��־
 * @param       pbuff       : URB�������׵�ַ
 * @param       length      : URB���ݳ���
 * @param       do_ping     : ����do ping protocol,USB HS���õ�
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_SubmitURB(USBH_HandleTypeDef *phost,
                                     uint8_t pipe,
                                     uint8_t direction,
                                     uint8_t ep_type,
                                     uint8_t token,
                                     uint8_t *pbuff,
                                     uint16_t length, uint8_t do_ping)
{
    HAL_HCD_HC_SubmitRequest(phost->pData, pipe, direction, ep_type, token, pbuff, length, do_ping);
    return USBH_OK;
}

/**
 * @brief       USBH ��ȡURB״̬
 * @param       phost       : USBH���ָ��
 * @param       pipe        : �ܵ����
 * @retval      URB״̬
 */
USBH_URBStateTypeDef USBH_LL_GetURBState(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    return (USBH_URBStateTypeDef) HAL_HCD_HC_GetURBState(phost->pData, pipe);
}

/**
 * @brief       USBH ����VBUS״̬
 * @param       phost       : USBH���ָ��
 * @param       state       : vbus״̬. 0,����; 1,������
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_DriverVBUS(USBH_HandleTypeDef *phost, uint8_t state)
{
#ifdef USE_USB_FS

    if (state == 0)
    {
    }
    else
    {
    }

#endif
    HAL_Delay(500);
    return USBH_OK;
}

/**
 * @brief       USBH ���ùܵ��ķ�ת
 * @param       phost       : USBH���ָ��
 * @param       pipe        : �ܵ����
 * @param       toggle      : ��ת״̬
 * @retval      USB״̬
 *   @arg       USBH_OK(0)   , ����;
 *   @arg       USBH_BUSY(1) , æ;
 *   @arg       ����         , ʧ��;
 */
USBH_StatusTypeDef USBH_LL_SetToggle(USBH_HandleTypeDef *phost, uint8_t pipe, uint8_t toggle)
{
    if (g_hhcd.hc[pipe].ep_is_in)
    {
        g_hhcd.hc[pipe].toggle_in = toggle;
    }
    else
    {
        g_hhcd.hc[pipe].toggle_out = toggle;
    }

    return USBH_OK;
}

/**
 * @brief       USBH ��ȡ�ܵ���ת״̬
 * @param       phost       : USBH���ָ��
 * @param       pipe        : �ܵ����
 * @retval      ��ת״̬
 */
uint8_t USBH_LL_GetToggle(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    uint8_t toggle = 0;

    if (g_hhcd.hc[pipe].ep_is_in)
    {
        toggle = g_hhcd.hc[pipe].toggle_in;
    }
    else
    {
        toggle = g_hhcd.hc[pipe].toggle_out;
    }

    return toggle;
}

/**
 * @brief       USBH ��ʱ����(��msΪ��λ)
 * @param       Delay       : ��ʱ��ms��
 * @retval      ��ת״̬
 */
void USBH_Delay(uint32_t Delay)
{
    delay_ms(Delay);
}
























