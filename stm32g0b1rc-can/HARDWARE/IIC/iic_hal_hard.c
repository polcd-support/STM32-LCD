#include "iic_hal_hard.h"

void IICInit(iic_bus_t *hi2c)
{
    // 硬件I2C初始化由HAL库完成，这里不需要额外操作
}

uint8_t IIC_Read_One_Byte(iic_bus_t *hi2c, uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t value = 0;
    HAL_I2C_Mem_Read(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
    return value;
}

void IIC_Write_One_Byte(iic_bus_t *hi2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t dat)
{
    HAL_I2C_Mem_Write(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, &dat, 1, HAL_MAX_DELAY);
}

void IIC_Read_Multi_Byte(iic_bus_t *hi2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf)
{
    HAL_I2C_Mem_Read(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, buf, len, HAL_MAX_DELAY);
}
