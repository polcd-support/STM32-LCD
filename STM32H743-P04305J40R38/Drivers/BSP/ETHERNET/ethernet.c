/**
 ****************************************************************************************************
 * @file        ethernet.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-08-01
 * @brief       ETHERNET 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211014
 * 第一次发布
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


ETH_HandleTypeDef   g_eth_handler;                                                                  /* 以太网句柄 */
/* 以太网描述符和缓冲区 */
__attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT];                  /* 以太网Rx DMA描述符,96字节,必须做内存保护,禁止CACHE */
__attribute__((at(0x30040060))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT];                  /* 以太网Tx DMA描述符,96字节,必须做内存保护,禁止CACHE */
__attribute__((at(0x30040200))) uint8_t ETH_Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE];          /* 以太网接收缓冲区 */

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
 * @brief       以太网芯片初始化
 * @param       无
 * @retval      0,成功
 *              1,失败
 */
uint8_t ethernet_init(void)
{
    uint8_t macaddress[6];
    uint32_t timeout = 0;
    uint32_t regval = 0;
    uint32_t phylink = 0;
    
    int status = ETH_CHIP_STATUS_OK;
    
    mpu_set_protection(0x30040000,256,8,0,MPU_REGION_FULL_ACCESS,0,0,1);                        /* 保护SRAM3,共32K字节,禁止共用,禁止cache,允许缓冲 */
  
    macaddress[0] = lwipdev.mac[0]; 
    macaddress[1] = lwipdev.mac[1]; 
    macaddress[2] = lwipdev.mac[2];
    macaddress[3] = lwipdev.mac[3];   
    macaddress[4] = lwipdev.mac[4];
    macaddress[5] = lwipdev.mac[5];
    
    g_eth_handler.Instance = ETH;                             /* ETH */
    g_eth_handler.Init.MACAddr = macaddress;                  /* mac地址 */
    g_eth_handler.Init.MediaInterface = HAL_ETH_RMII_MODE;    /* RMII接口 */
    g_eth_handler.Init.RxDesc = DMARxDscrTab;                 /* 发送描述符 */
    g_eth_handler.Init.TxDesc = DMATxDscrTab;                 /* 接收描述如 */
    g_eth_handler.Init.RxBuffLen = ETH_MAX_PACKET_SIZE;       /* 接收长度 */
    
    HAL_ETH_Init(&g_eth_handler);
    
    /* 设置PHY IO功能 */
    status = eth_chip_regster_bus_io(&ETHCHIP, &ETH_CHIP_IOCtx);
    
    if (status != ETH_CHIP_STATUS_OK)
    {
        return 1;
    }

    /* 初始化ETH PHY */
    status = eth_chip_init(&ETHCHIP);
    
    if (status != ETH_CHIP_STATUS_OK)
    {
        return 2;
    }
    
    /* 必须开启自动协商功能 */
    status = eth_chip_start_auto_nego(&ETHCHIP);
    
    delay_ms(2000);        /* 必须等待初始化 */
    
    if (status != ETH_CHIP_STATUS_OK)
    {
        return 3;
    }
    
    sys_nvic_init(1,0,ETH_IRQn,2);                          /* 配置ETH中的分组 */
    
    return status;
}

extern void lwip_pkt_handle(void);
    
/**
 * @brief       ETH中断服务函数
 * @param       无
 * @retval      无
 */
void ETH_IRQHandler(void)
{
    OSIntEnter();       /* 进入中断 */
    lwip_pkt_handle();
    /* 清除中断标志位 */
    __HAL_ETH_DMA_CLEAR_IT(&g_eth_handler,ETH_DMA_NORMAL_IT);   /* 清除DMA中断标志位 */
    __HAL_ETH_DMA_CLEAR_IT(&g_eth_handler,ETH_DMA_RX_IT);       /* 清除DMA接收中断标志位 */
    __HAL_ETH_DMA_CLEAR_IT(&g_eth_handler,ETH_DMA_TX_IT);       /* 清除DMA接收中断标志位 */
    OSIntExit();        /* 触发任务切换软中断 */
} 

/**
 * @brief       ETH底层驱动，时钟使能，引脚配置
 *    @note     此函数会被HAL_ETH_Init()调用
 * @param       heth:以太网句柄
 * @retval      无
 */
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    RCC->AHB1ENR |= 7 << 15;        /* 使能ETH MAC/MAC_Tx/MAC_Rx时钟 */
    /* ETH IO接口初始化 */
    RCC->AHB4ENR |= 1 << 0;         /* 使能PORTA时钟 */
    RCC->AHB4ENR |= 1 << 1;         /* 使能PORTB时钟 */
    RCC->AHB4ENR |= 1 << 2;         /* 使能PORTC时钟 */
    RCC->AHB4ENR |= 1 << 6;         /* 使能PORTG时钟 */
    RCC->APB4ENR |= 1 << 1;         /* 使能SYSCFG时钟 */
    SYSCFG->PMCR |= 4 << 21;        /* 使用RMII PHY接口. */
    
    /* 网络引脚设置 RMII接口
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
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PA1,2,7 复用输出 */
    
    sys_gpio_set(GPIOC, SYS_GPIO_PIN1 | SYS_GPIO_PIN4 | SYS_GPIO_PIN5,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PC1,4,5 复用输出 */
    
    sys_gpio_set(GPIOG, SYS_GPIO_PIN13 | SYS_GPIO_PIN14,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PG13,14 复用输出 */
                 
    sys_gpio_set(GPIOB, SYS_GPIO_PIN11,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PB11 复用输出 */

    /* GPIO工作复用功能功能设置 */
    sys_gpio_af_set(GPIOA, SYS_GPIO_PIN1 | SYS_GPIO_PIN2 | SYS_GPIO_PIN7, 11);      /* PA1,2,7 AF11 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN1 | SYS_GPIO_PIN4 | SYS_GPIO_PIN5, 11);      /* PC1,4,5 AF11 */
    sys_gpio_af_set(GPIOG, SYS_GPIO_PIN13 | SYS_GPIO_PIN14, 11);                    /* PG13,14 AF11 */
    sys_gpio_af_set(GPIOB, SYS_GPIO_PIN11 , 11);                                    /* PB11 */
    
    uint32_t regval;

    sys_intx_disable();                                     /* 关闭所有中断，复位过程不能被打断！ */
    /* 判断开发板是否是旧版本(老板卡板载的是LAN8720A，而新板卡板载的是YT8512C) */
    regval = ethernet_read_phy(2);
    
    if (regval && 0xFFF == 0xFFF)                           /* 旧板卡（LAN8720A）引脚复位 */
    {
        pcf8574_write_bit(ETH_RESET_IO,1);                  /* 硬件复位 */
        delay_ms(100);
        pcf8574_write_bit(ETH_RESET_IO,0);                  /* 复位结束 */
        delay_ms(100);
    }
    else                                                    /* 新板卡（YT8512C）引脚复位 */
    {
        pcf8574_write_bit(ETH_RESET_IO,0);                  /* 硬件复位 */
        delay_ms(100);
        pcf8574_write_bit(ETH_RESET_IO,1);                  /* 复位结束 */
        delay_ms(100);
    }
    
    sys_intx_enable();                                      /* 开启所有中断 */
}

/**
 * @breif       读取以太网芯片寄存器值
 * @param       reg：读取的寄存器地址
 * @retval      regval：返回读取的寄存器值
 */
uint32_t ethernet_read_phy(uint16_t reg)
{
    uint32_t regval;

    HAL_ETH_ReadPHYRegister(&g_eth_handler, ETH_CHIP_ADDR,reg, &regval);
    return regval;
}

/**
 * @breif       向以太网芯片指定地址写入寄存器值
 * @param       reg   : 要写入的寄存器
 * @param       value : 要写入的寄存器
 * @retval      无
 */
void ethernet_write_phy(uint16_t reg, uint16_t value)
{
    uint32_t temp = value;
    
    HAL_ETH_WritePHYRegister(&g_eth_handler, ETH_CHIP_ADDR,reg, temp);
}

/**
 * @breif       获得网络芯片的速度模式
 * @param       无
 * @retval      1:100M
                0:10M
 */
uint8_t ethernet_chip_get_speed(void)
{
    uint8_t speed;
    if(PHY_TYPE == LAN8720) 
    speed = ~((ethernet_read_phy(ETH_CHIP_PHYSCSR) & ETH_CHIP_SPEED_STATUS));         /* 从LAN8720的31号寄存器中读取网络速度和双工模式 */
    else if(PHY_TYPE == SR8201F)
    speed = ((ethernet_read_phy(ETH_CHIP_PHYSCSR) & ETH_CHIP_SPEED_STATUS) >> 13);    /* 从SR8201F的0号寄存器中读取网络速度和双工模式 */
    else if(PHY_TYPE == YT8512C)
    speed = ((ethernet_read_phy(ETH_CHIP_PHYSCSR) & ETH_CHIP_SPEED_STATUS) >> 14);    /* 从YT8512C的17号寄存器中读取网络速度和双工模式 */
    else if(PHY_TYPE == RTL8201)
    speed = ((ethernet_read_phy(ETH_CHIP_PHYSCSR) & ETH_CHIP_SPEED_STATUS) >> 1);     /* 从RTL8201的16号寄存器中读取网络速度和双工模式 */
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
