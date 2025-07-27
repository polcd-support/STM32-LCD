#include "24cxx.h"
#include <string.h>

/**
  * @brief  ��AT24C02д��һ���ֽ�
  * @param  hi2c: I2C���
  * @param  devAddr: �豸��ַ(7λ��ַ)
  * @param  memAddr: �ڴ��ַ(0-255)
  * @param  data: Ҫд�������
  * @retval HAL״̬
  */
HAL_StatusTypeDef AT24C02_WriteByte(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t data)
{
    return HAL_I2C_Mem_Write(hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, &data, 1, AT24C02_I2C_TIMEOUT);
}

/**
  * @brief  ��AT24C02��ȡһ���ֽ�
  * @param  hi2c: I2C���
  * @param  devAddr: �豸��ַ(7λ��ַ)
  * @param  memAddr: �ڴ��ַ(0-255)
  * @param  data: ��ȡ��������ָ��
  * @retval HAL״̬
  */
HAL_StatusTypeDef AT24C02_ReadByte(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data)
{
    return HAL_I2C_Mem_Read(hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, data, 1, AT24C02_I2C_TIMEOUT);
}

/**
  * @brief  ��AT24C02д��һҳ����(���8�ֽ�)
  * @param  hi2c: I2C���
  * @param  devAddr: �豸��ַ(7λ��ַ)
  * @param  memAddr: �ڴ���ʼ��ַ(0-255)
  * @param  data: Ҫд�������ָ��
  * @param  len: ���ݳ���(1-8)
  * @retval HAL״̬
  */
HAL_StatusTypeDef AT24C02_WritePage(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data, uint8_t len)
{
    // AT24C02ҳ��СΪ8�ֽڣ�����Ƿ��ҳ
    if((memAddr % 8) + len > 8)
    {
        return HAL_ERROR; // ��ҳд�룬��Ҫ�ֶ��д��
    }
    
    return HAL_I2C_Mem_Write(hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, data, len, AT24C02_I2C_TIMEOUT);
}

/**
  * @brief  ��AT24C02��ȡ����ֽ�
  * @param  hi2c: I2C���
  * @param  devAddr: �豸��ַ(7λ��ַ)
  * @param  memAddr: �ڴ���ʼ��ַ(0-255)
  * @param  data: ��ȡ���ݻ�����ָ��
  * @param  len: Ҫ��ȡ�����ݳ���
  * @retval HAL״̬
  */
HAL_StatusTypeDef AT24C02_ReadBuffer(I2C_HandleTypeDef *hi2c, uint8_t devAddr, uint8_t memAddr, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, data, len, AT24C02_I2C_TIMEOUT);
}

/**
  * @brief  ���AT24C02�Ƿ����
  * @param  hi2c: I2C���
  * @param  devAddr: �豸��ַ(7λ��ַ)
  * @retval HAL״̬
  */
HAL_StatusTypeDef AT24C02_IsReady(I2C_HandleTypeDef *hi2c, uint8_t devAddr)
{
    return HAL_I2C_IsDeviceReady(hi2c, devAddr, 3, AT24C02_I2C_TIMEOUT);
}