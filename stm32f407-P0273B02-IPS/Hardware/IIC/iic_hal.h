#ifndef __IIC_HAL_H
#define __IIC_HAL_H

#include "main.h"

// 软件I2C引脚定义 - 根据实际电路修改
#define SCL_PIN    GPIO_PIN_0
#define SCL_PORT   GPIOB
#define SDA_PIN    GPIO_PIN_11
#define SDA_PORT   GPIOF

// I2C时序延迟 - 根据实际MCU时钟调整
#define IIC_DELAY 5

typedef struct {
    GPIO_TypeDef *IIC_SDA_PORT;
    GPIO_TypeDef *IIC_SCL_PORT;
    uint16_t IIC_SDA_PIN;
    uint16_t IIC_SCL_PIN;
} iic_bus_t;

void IICInit(iic_bus_t *bus);
void IICStart(iic_bus_t *bus);
void IICStop(iic_bus_t *bus);
uint8_t IICWaitAck(iic_bus_t *bus);
void IICAck(iic_bus_t *bus);
void IICNAck(iic_bus_t *bus);
void IICSendByte(iic_bus_t *bus, uint8_t txd);
uint8_t IICRecvByte(iic_bus_t *bus);
uint8_t IIC_Read_One_Byte(iic_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr);
void IIC_Write_One_Byte(iic_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t dat);
void IIC_Read_Multi_Byte(iic_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf);
uint8_t IIC_Scan(iic_bus_t *bus, uint8_t found_addr[]);

#endif


