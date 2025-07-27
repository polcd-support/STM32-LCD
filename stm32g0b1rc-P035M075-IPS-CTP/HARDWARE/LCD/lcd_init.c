#include "lcd_init.h"
#include "delay.h"
#include "spi.h"

void LCD_GPIO_Init(void)
{
    // GPIO初始化通常在CubeMX生成的代码中完成
    // 如果手动初始化，可以这样配置：
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // 配置RES, DC, CS, BLK引脚为输出
    GPIO_InitStruct.Pin =  GPIO_PIN_3 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin =  GPIO_PIN_1 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//    GPIO_InitStruct.Pin =  GPIO_PIN_2 ;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}
/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(uint8_t dat) 
{    
	
	LCD_CS_Clr();
    // 使用HAL库的SPI发送函数
    HAL_SPI_Transmit(&hspi1, &dat, 1,1000);
//	delay_us(1);
//    while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
    // 等待传输完成（HAL_SPI_Transmit已经包含等待）
	LCD_CS_Set();
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(uint8_t dat)
{
    LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat)
{
	LCD_Writ_Bus((dat>>8)&0xF8);//RED
	LCD_Writ_Bus((dat>>3)&0xFC);//GREEN
	LCD_Writ_Bus(dat<<3);//BLUE
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(uint8_t dat)
{

    LCD_DC_Clr(); // 写命令
    LCD_Writ_Bus(dat);
	LCD_DC_Set();
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_WR_REG(0x2a);//列地址设置
	LCD_WR_DATA8(x1>>8);
	LCD_WR_DATA8(x1);
	LCD_WR_DATA8(x2>>8);
	LCD_WR_DATA8(x2);
	LCD_WR_REG(0x2b);//行地址设置
	LCD_WR_DATA8(y1>>8);
	LCD_WR_DATA8(y1);
	LCD_WR_DATA8(y2>>8);
	LCD_WR_DATA8(y2);
	LCD_WR_REG(0x2c);//储存器写
}

void LCD_Init(void)
{
	LCD_GPIO_Init();//初始化GPIO
	
	LCD_RES_Clr();//复位
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);
	

	
	LCD_WR_REG(0xE0); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x07); 
	LCD_WR_DATA8(0x0f); 
	LCD_WR_DATA8(0x0D); 
	LCD_WR_DATA8(0x1B); 
	LCD_WR_DATA8(0x0A); 
	LCD_WR_DATA8(0x3c); 
	LCD_WR_DATA8(0x78); 
	LCD_WR_DATA8(0x4A); 
	LCD_WR_DATA8(0x07); 
	LCD_WR_DATA8(0x0E); 
	LCD_WR_DATA8(0x09); 
	LCD_WR_DATA8(0x1B); 
	LCD_WR_DATA8(0x1e); 
	LCD_WR_DATA8(0x0f);  
	
	LCD_WR_REG(0xE1); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x22); 
	LCD_WR_DATA8(0x24); 
	LCD_WR_DATA8(0x06); 
	LCD_WR_DATA8(0x12); 
	LCD_WR_DATA8(0x07); 
	LCD_WR_DATA8(0x36); 
	LCD_WR_DATA8(0x47); 
	LCD_WR_DATA8(0x47); 
	LCD_WR_DATA8(0x06); 
	LCD_WR_DATA8(0x0a); 
	LCD_WR_DATA8(0x07); 
	LCD_WR_DATA8(0x30); 
	LCD_WR_DATA8(0x37); 
	LCD_WR_DATA8(0x0f); 
	
	LCD_WR_REG(0xC0); 
	LCD_WR_DATA8(0x10); 
	LCD_WR_DATA8(0x10); 
	
	LCD_WR_REG(0xC1); 
	LCD_WR_DATA8(0x41); 
	
	LCD_WR_REG(0xC5); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x22); 
	LCD_WR_DATA8(0x80); 
	
	LCD_WR_REG(0x36);    // Memory Access Control 
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x48);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0x88);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x28);
	else LCD_WR_DATA8(0xE8);

	
	LCD_WR_REG(0x3A); //Interface Mode Control，此处ILI9486为0X55
	LCD_WR_DATA8(0x66);
		
	LCD_WR_REG(0XB0);  //Interface Mode Control  
	LCD_WR_DATA8(0x00); 
	LCD_WR_REG(0xB1);   //Frame rate 70HZ  
	LCD_WR_DATA8(0xB0); 
	LCD_WR_DATA8(0x11); 
	LCD_WR_REG(0xB4); 
	LCD_WR_DATA8(0x02);   
	LCD_WR_REG(0xB6); //RGB/MCU Interface Control
	LCD_WR_DATA8(0x02); 
	LCD_WR_DATA8(0x02); 
	
	LCD_WR_REG(0xB7); 
	LCD_WR_DATA8(0xC6); 
	LCD_WR_REG(0xE9); 
	LCD_WR_DATA8(0x00);
	
	LCD_WR_REG(0XF7);    
	LCD_WR_DATA8(0xA9); 
	LCD_WR_DATA8(0x51); 
	LCD_WR_DATA8(0x2C); 
	LCD_WR_DATA8(0x82);
	
	LCD_WR_REG(0x11); 
	delay_ms(120); 

	LCD_WR_REG(0x21); 
	LCD_WR_REG(0x29); 
} 








