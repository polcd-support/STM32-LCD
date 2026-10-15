#ifndef AT24C02_H
#define AT24C02_H

#include "main.h"

#define AT24C02_I2C_TIMEOUT     100
#define AT24C02_ADDRESS         0xA0  // 默认I2C地址(A0-A2引脚接地)

// 函数声明
HAL_StatusTypeDef AT24C02_WriteByte(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t data);
HAL_StatusTypeDef AT24C02_ReadByte(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data);
HAL_StatusTypeDef AT24C02_WritePage(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data, uint8_t len);
HAL_StatusTypeDef AT24C02_ReadBuffer(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data, uint16_t len);
HAL_StatusTypeDef AT24C02_IsReady(I2C_HandleTypeDef *hi2c, uint8_t devAddr);

#endif // AT24C02_H

