#ifndef __IIC_HAL_HARD_H
#define __IIC_HAL_HARD_H

#include "main.h"

typedef I2C_HandleTypeDef iic_bus_t;

void IICInit(iic_bus_t *hi2c);
uint8_t IIC_Read_One_Byte(iic_bus_t *hi2c, uint8_t dev_addr, uint8_t reg_addr);
void IIC_Write_One_Byte(iic_bus_t *hi2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t dat);
void IIC_Read_Multi_Byte(iic_bus_t *hi2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf);

#endif

