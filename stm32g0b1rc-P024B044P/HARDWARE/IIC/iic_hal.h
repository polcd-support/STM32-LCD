#ifndef __IIC_HAL_H
#define __IIC_HAL_H

#include "main.h"

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

#endif


