#include "iic_hal.h"
#include "delay.h"

#define IIC_DELAY 5

void IICInit(iic_bus_t *bus)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // SCL
    GPIO_InitStruct.Pin = bus->IIC_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(bus->IIC_SCL_PORT, &GPIO_InitStruct);
    
    // SDA
    GPIO_InitStruct.Pin = bus->IIC_SDA_PIN;
    HAL_GPIO_Init(bus->IIC_SDA_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_SET);
}

void IICStart(iic_bus_t *bus)
{
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_SET);
    delay_us(IIC_DELAY);
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_RESET);
    delay_us(IIC_DELAY);
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
}

void IICStop(iic_bus_t *bus)
{
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_SET);
    delay_us(IIC_DELAY);
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_SET);
    delay_us(IIC_DELAY);
}

uint8_t IICWaitAck(iic_bus_t *bus)
{
    uint8_t ucErrTime = 0;
    
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_SET);
    delay_us(1);
    
    while(HAL_GPIO_ReadPin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN)) {
        ucErrTime++;
        if(ucErrTime > 250) {
            IICStop(bus);
            return 1;
        }
    }
    
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
    return 0;
}

void IICAck(iic_bus_t *bus)
{
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_RESET);
    delay_us(IIC_DELAY);
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_SET);
    delay_us(IIC_DELAY);
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
}

void IICNAck(iic_bus_t *bus)
{
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_SET);
    delay_us(IIC_DELAY);
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_SET);
    delay_us(IIC_DELAY);
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
}

void IICSendByte(iic_bus_t *bus, uint8_t txd)
{
    uint8_t t;
    
    HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
    
    for(t=0; t<8; t++) {
        if((txd&0x80)>>7)
            HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_RESET);
        
        txd <<= 1;
        delay_us(IIC_DELAY);
        HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_SET);
        delay_us(IIC_DELAY);
        HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
    }
}

uint8_t IICRecvByte(iic_bus_t *bus)
{
    uint8_t i,receive=0;
    
    HAL_GPIO_WritePin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN, GPIO_PIN_SET);
    
    for(i=0;i<8;i++) {
        receive <<= 1;
        HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_RESET);
        delay_us(IIC_DELAY);
        HAL_GPIO_WritePin(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN, GPIO_PIN_SET);
        
        if(HAL_GPIO_ReadPin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN))
            receive++;
            
        delay_us(IIC_DELAY);
    }
    
    return receive;
}

uint8_t IIC_Read_One_Byte(iic_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t res;
    
    IICStart(bus);
    IICSendByte(bus, dev_addr << 1);
    IICWaitAck(bus);
    IICSendByte(bus, reg_addr);
    IICWaitAck(bus);
    
    IICStart(bus);
    IICSendByte(bus, (dev_addr << 1) | 1);
    IICWaitAck(bus);
    res = IICRecvByte(bus);
    IICNAck(bus);
    IICStop(bus);
    
    return res;
}

void IIC_Write_One_Byte(iic_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t dat)
{
    IICStart(bus);
    IICSendByte(bus, dev_addr << 1);
    IICWaitAck(bus);
    IICSendByte(bus, reg_addr);
    IICWaitAck(bus);
    IICSendByte(bus, dat);
    IICWaitAck(bus);
    IICStop(bus);
}

void IIC_Read_Multi_Byte(iic_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf)
{
    IICStart(bus);
    IICSendByte(bus, dev_addr << 1);
    IICWaitAck(bus);
    IICSendByte(bus, reg_addr);
    IICWaitAck(bus);
    
    IICStart(bus);
    IICSendByte(bus, (dev_addr << 1) | 1);
    IICWaitAck(bus);
    
    while(len) {
        if(len == 1) {
            *buf = IICRecvByte(bus);
            IICNAck(bus);
        } else {
            *buf = IICRecvByte(bus);
            IICAck(bus);
        }
        buf++;
        len--;
    }
    
    IICStop(bus);
}

