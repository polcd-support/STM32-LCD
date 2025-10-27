#include "24cxx.h"
#include <string.h>

/**
  * @brief  向AT24C02写入一个字节
  * @param  hi2c: I2C句柄
  * @param  devAddr: 设备地址(7位地址)
  * @param  memAddr: 内存地址(0-255)
  * @param  data: 要写入的数据
  * @retval HAL状态
  */
HAL_StatusTypeDef AT24C02_WriteByte(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t data)
{
    return HAL_I2C_Mem_Write(hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, &data, 1, AT24C02_I2C_TIMEOUT);
}

/**
  * @brief  从AT24C02读取一个字节
  * @param  hi2c: I2C句柄
  * @param  devAddr: 设备地址(7位地址)
  * @param  memAddr: 内存地址(0-255)
  * @param  data: 读取到的数据指针
  * @retval HAL状态
  */
HAL_StatusTypeDef AT24C02_ReadByte(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data)
{
    return HAL_I2C_Mem_Read(hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, data, 1, AT24C02_I2C_TIMEOUT);
}

/**
  * @brief  向AT24C02写入一页数据(最多8字节)
  * @param  hi2c: I2C句柄
  * @param  devAddr: 设备地址(7位地址)
  * @param  memAddr: 内存起始地址(0-255)
  * @param  data: 要写入的数据指针
  * @param  len: 数据长度(1-8)
  * @retval HAL状态
  */
HAL_StatusTypeDef AT24C02_WritePage(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data, uint8_t len)
{
    // AT24C02页大小为8字节，检查是否跨页
    if((memAddr % 8) + len > 8)
    {
        return HAL_ERROR; // 跨页写入，需要分多次写入
    }
    
    return HAL_I2C_Mem_Write(hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, data, len, AT24C02_I2C_TIMEOUT);
}

/**
  * @brief  从AT24C02读取多个字节
  * @param  hi2c: I2C句柄
  * @param  devAddr: 设备地址(7位地址)
  * @param  memAddr: 内存起始地址(0-255)
  * @param  data: 读取数据缓冲区指针
  * @param  len: 要读取的数据长度
  * @retval HAL状态
  */
HAL_StatusTypeDef AT24C02_ReadBuffer(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, data, len, AT24C02_I2C_TIMEOUT);
}

/**
  * @brief  检查AT24C02是否就绪
  * @param  hi2c: I2C句柄
  * @param  devAddr: 设备地址(7位地址)
  * @retval HAL状态
  */
HAL_StatusTypeDef AT24C02_IsReady(I2C_HandleTypeDef *hi2c, uint8_t devAddr)
{
    return HAL_I2C_IsDeviceReady(hi2c, devAddr, 3, AT24C02_I2C_TIMEOUT);
}