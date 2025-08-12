/**
 ****************************************************************************************************
 * @file        ethernet.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-08-01
 * @brief       ETHERNET ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20211014
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/ETHERNET/ethernet.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./BSP/ETHERNET/ethernet_chip.h"
#include "lwip_comm.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "./BSP/MPU/mpu.h"
#include "os.h"


ETH_HandleTypeDef   g_eth_handler;                                                                  /* ��̫����� */
/* ��̫���������ͻ����� */
__attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT];                  /* ��̫��Rx DMA������,96�ֽ�,�������ڴ汣��,��ֹCACHE */
__attribute__((at(0x30040060))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT];                  /* ��̫��Tx DMA������,96�ֽ�,�������ڴ汣��,��ֹCACHE */
__attribute__((at(0x30040200))) uint8_t ETH_Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE];          /* ��̫�����ջ����� */

int32_t ETH_PHY_IO_Init(void);
int32_t ETH_PHY_IO_DeInit (void);
int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);
int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
int32_t ETH_PHY_IO_GetTick(void);

eth_chip_object_t ETHCHIP;

int32_t ETH_PHY_IO_Init(void);
int32_t ETH_PHY_IO_DeInit (void);
int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);
int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
int32_t ETH_PHY_IO_GetTick(void);

eth_chip_ioc_tx_t  ETH_CHIP_IOCtx = {   ETH_PHY_IO_Init,
                                        ETH_PHY_IO_DeInit,
                                        ETH_PHY_IO_WriteReg,
                                        ETH_PHY_IO_ReadReg,
                                        ETH_PHY_IO_GetTick};

/**
 * @brief       ��̫��оƬ��ʼ��
 * @param       ��
 * @retval      0,�ɹ�
 *              1,ʧ��
 */
uint8_t ethernet_init(void)
{
    uint8_t macaddress[6];
    uint32_t timeout = 0;
    uint32_t regval = 0;
    uint32_t phylink = 0;
    
    int status = ETH_CHIP_STATUS_OK;
    
    mpu_set_protection(0x30040000,256,8,0,MPU_REGION_FULL_ACCESS,0,0,1);                        /* ����SRAM3,��32K�ֽ�,��ֹ����,��ֹcache,������ */
  
    macaddress[0] = lwipdev.mac[0]; 
    macaddress[1] = lwipdev.mac[1]; 
    macaddress[2] = lwipdev.mac[2];
    macaddress[3] = lwipdev.mac[3];   
    macaddress[4] = lwipdev.mac[4];
    macaddress[5] = lwipdev.mac[5];
    
    g_eth_handler.Instance = ETH;                             /* ETH */
    g_eth_handler.Init.MACAddr = macaddress;                  /* mac��ַ */
    g_eth_handler.Init.MediaInterface = HAL_ETH_RMII_MODE;    /* RMII�ӿ� */
    g_eth_handler.Init.RxDesc = DMARxDscrTab;                 /* ���������� */
    g_eth_handler.Init.TxDesc = DMATxDscrTab;                 /* ���������� */
    g_eth_handler.Init.RxBuffLen = ETH_MAX_PACKET_SIZE;       /* ���ճ��� */
    
    HAL_ETH_Init(&g_eth_handler);
    
    /* ����PHY IO���� */
    status = eth_chip_regster_bus_io(&ETHCHIP, &ETH_CHIP_IOCtx);
    
    if (status != ETH_CHIP_STATUS_OK)
    {
        return 1;
    }

    /* ��ʼ��ETH PHY */
    status = eth_chip_init(&ETHCHIP);
    
    if (status != ETH_CHIP_STATUS_OK)
    {
        return 2;
    }
    
    /* ���뿪���Զ�Э�̹��� */
    status = eth_chip_start_auto_nego(&ETHCHIP);
    
    delay_ms(2000);        /* ����ȴ���ʼ�� */
    
    if (status != ETH_CHIP_STATUS_OK)
    {
        return 3;
    }
    
    sys_nvic_init(1,0,ETH_IRQn,2);                          /* ����ETH�еķ��� */
    
    return status;
}

extern void lwip_pkt_handle(void);
    
/**
 * @brief       ETH�жϷ�����
 * @param       ��
 * @retval      ��
 */
void ETH_IRQHandler(void)
{
    OSIntEnter();       /* �����ж� */
    lwip_pkt_handle();
    /* ����жϱ�־λ */
    __HAL_ETH_DMA_CLEAR_IT(&g_eth_handler,ETH_DMA_NORMAL_IT);   /* ���DMA�жϱ�־λ */
    __HAL_ETH_DMA_CLEAR_IT(&g_eth_handler,ETH_DMA_RX_IT);       /* ���DMA�����жϱ�־λ */
    __HAL_ETH_DMA_CLEAR_IT(&g_eth_handler,ETH_DMA_TX_IT);       /* ���DMA�����жϱ�־λ */
    OSIntExit();        /* ���������л����ж� */
} 

/**
 * @brief       ETH�ײ�������ʱ��ʹ�ܣ���������
 *    @note     �˺����ᱻHAL_ETH_Init()����
 * @param       heth:��̫�����
 * @retval      ��
 */
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    RCC->AHB1ENR |= 7 << 15;        /* ʹ��ETH MAC/MAC_Tx/MAC_Rxʱ�� */
    /* ETH IO�ӿڳ�ʼ�� */
    RCC->AHB4ENR |= 1 << 0;         /* ʹ��PORTAʱ�� */
    RCC->AHB4ENR |= 1 << 1;         /* ʹ��PORTBʱ�� */
    RCC->AHB4ENR |= 1 << 2;         /* ʹ��PORTCʱ�� */
    RCC->AHB4ENR |= 1 << 6;         /* ʹ��PORTGʱ�� */
    RCC->APB4ENR |= 1 << 1;         /* ʹ��SYSCFGʱ�� */
    SYSCFG->PMCR |= 4 << 21;        /* ʹ��RMII PHY�ӿ�. */
    
    /* ������������ RMII�ӿ�
     * ETH_MDIO -------------------------> PA2
     * ETH_MDC --------------------------> PC1
     * ETH_RMII_REF_CLK------------------> PA1
     * ETH_RMII_CRS_DV ------------------> PA7
     * ETH_RMII_RXD0 --------------------> PC4
     * ETH_RMII_RXD1 --------------------> PC5
     * ETH_RMII_TX_EN -------------------> PB11
     * ETH_RMII_TXD0 --------------------> PG13
     * ETH_RMII_TXD1 --------------------> PG14
     */

    sys_gpio_set(GPIOA, SYS_GPIO_PIN1 | SYS_GPIO_PIN2 | SYS_GPIO_PIN7,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PA1,2,7 ������� */
    
    sys_gpio_set(GPIOC, SYS_GPIO_PIN1 | SYS_GPIO_PIN4 | SYS_GPIO_PIN5,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PC1,4,5 ������� */
    
    sys_gpio_set(GPIOG, SYS_GPIO_PIN13 | SYS_GPIO_PIN14,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PG13,14 ������� */
                 
    sys_gpio_set(GPIOB, SYS_GPIO_PIN11,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PB11 ������� */

    /* GPIO�������ù��ܹ������� */
    sys_gpio_af_set(GPIOA, SYS_GPIO_PIN1 | SYS_GPIO_PIN2 | SYS_GPIO_PIN7, 11);      /* PA1,2,7 AF11 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN1 | SYS_GPIO_PIN4 | SYS_GPIO_PIN5, 11);      /* PC1,4,5 AF11 */
    sys_gpio_af_set(GPIOG, SYS_GPIO_PIN13 | SYS_GPIO_PIN14, 11);                    /* PG13,14 AF11 */
    sys_gpio_af_set(GPIOB, SYS_GPIO_PIN11 , 11);                                    /* PB11 */
    
    uint32_t regval;

    sys_intx_disable();                                     /* �ر������жϣ���λ���̲��ܱ���ϣ� */
    /* �жϿ������Ƿ��Ǿɰ汾(�ϰ忨���ص���LAN8720A�����°忨���ص���YT8512C) */
    regval = ethernet_read_phy(2);
    
    if (regval && 0xFFF == 0xFFF)                           /* �ɰ忨��LAN8720A�����Ÿ�λ */
    {
        pcf8574_write_bit(ETH_RESET_IO,1);                  /* Ӳ����λ */
        delay_ms(100);
        pcf8574_write_bit(ETH_RESET_IO,0);                  /* ��λ���� */
        delay_ms(100);
    }
    else                                                    /* �°忨��YT8512C�����Ÿ�λ */
    {
        pcf8574_write_bit(ETH_RESET_IO,0);                  /* Ӳ����λ */
        delay_ms(100);
        pcf8574_write_bit(ETH_RESET_IO,1);                  /* ��λ���� */
        delay_ms(100);
    }
    
    sys_intx_enable();                                      /* ���������ж� */
}

/**
 * @breif       ��ȡ��̫��оƬ�Ĵ���ֵ
 * @param       reg����ȡ�ļĴ�����ַ
 * @retval      regval�����ض�ȡ�ļĴ���ֵ
 */
uint32_t ethernet_read_phy(uint16_t reg)
{
    uint32_t regval;

    HAL_ETH_ReadPHYRegister(&g_eth_handler, ETH_CHIP_ADDR,reg, &regval);
    return regval;
}

/**
 * @breif       ����̫��оƬָ����ַд��Ĵ���ֵ
 * @param       reg   : Ҫд��ļĴ���
 * @param       value : Ҫд��ļĴ���
 * @retval      ��
 */
void ethernet_write_phy(uint16_t reg, uint16_t value)
{
    uint32_t temp = value;
    
    HAL_ETH_WritePHYRegister(&g_eth_handler, ETH_CHIP_ADDR,reg, temp);
}

/**
 * @breif       �������оƬ���ٶ�ģʽ
 * @param       ��
 * @retval      1:100M
                0:10M
 */
uint8_t ethernet_chip_get_speed(void)
{
    uint8_t speed;
    if(PHY_TYPE == LAN8720) 
    speed = ~((ethernet_read_phy(ETH_CHIP_PHYSCSR) & ETH_CHIP_SPEED_STATUS));         /* ��LAN8720��31�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ */
    else if(PHY_TYPE == SR8201F)
    speed = ((ethernet_read_phy(ETH_CHIP_PHYSCSR) & ETH_CHIP_SPEED_STATUS) >> 13);    /* ��SR8201F��0�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ */
    else if(PHY_TYPE == YT8512C)
    speed = ((ethernet_read_phy(ETH_CHIP_PHYSCSR) & ETH_CHIP_SPEED_STATUS) >> 14);    /* ��YT8512C��17�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ */
    else if(PHY_TYPE == RTL8201)
    speed = ((ethernet_read_phy(ETH_CHIP_PHYSCSR) & ETH_CHIP_SPEED_STATUS) >> 1);     /* ��RTL8201��16�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ */
    return speed;
}

/*******************************************************************************
                       PHI IO Functions
*******************************************************************************/
/**
  * @brief  Initializes the MDIO interface GPIO and clocks.
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */
int32_t ETH_PHY_IO_Init(void)
{  
    /* We assume that MDIO GPIO configuration is already done
     in the ETH_MspInit() else it should be done here 
    */

    /* Configure the MDIO Clock */
    HAL_ETH_SetMDIOClockRange(&g_eth_handler);

    return 0;
}

/**
  * @brief  De-Initializes the MDIO interface .
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */
int32_t ETH_PHY_IO_DeInit (void)
{
    return 0;
}

/**
  * @brief  Read a PHY register through the MDIO interface.
  * @param  DevAddr: PHY port address
  * @param  RegAddr: PHY register address
  * @param  pRegVal: pointer to hold the register value 
  * @retval 0 if OK -1 if Error
  */
int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal)
{
    if (HAL_ETH_ReadPHYRegister(&g_eth_handler, DevAddr, RegAddr, pRegVal) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

/**
  * @brief  Write a value to a PHY register through the MDIO interface.
  * @param  DevAddr: PHY port address
  * @param  RegAddr: PHY register address
  * @param  RegVal: Value to be written 
  * @retval 0 if OK -1 if Error
  */
int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal)
{
    if (HAL_ETH_WritePHYRegister(&g_eth_handler, DevAddr, RegAddr, RegVal) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

/**
  * @brief  Get the time in millisecons used for internal PHY driver process.
  * @retval Time value
  */
int32_t ETH_PHY_IO_GetTick(void)
{
    return OSTimeGet();
}
