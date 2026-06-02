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


uint8_t LCD_Read_Bus() 
{    
	uint8_t rx_data = 0;
    // 使用HAL库的SPI发送函数
	HAL_SPI_Receive(&hspi1,  &rx_data, 1, 1000);
	return rx_data;
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat)
{

	LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);
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

		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1 );
		LCD_WR_DATA(x2 );
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); //储存器写

	
}

static uint32_t read_lcd_id(void) {
	uint8_t id_buffer[4] = {0};
    uint32_t lcd_id = 0;
	LCD_DC_Clr(); // 写命令
    // 拉低片选
	LCD_CS_Clr();

    // 发送读取ID命令 (不同LCD命令可能不同，常见有0x04, 0xD3等)
    uint8_t cmd = 0x04; // 以ILI9341为例
	
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000);
    LCD_DC_Set();
    // 读取4字节ID (实际可能只需要2-3字节)
    
	HAL_SPI_Receive(&hspi1, id_buffer, 4, 1000);
    
	LCD_CS_Set();
//    printf("0x%02X\r\n",id_buffer[0]);
//	printf("0x%02X\r\n",id_buffer[1]);
//	printf("0x%02X\r\n",id_buffer[2]);
//	printf("0x%02X\r\n",id_buffer[3]);
    // 组合ID (根据实际LCD调整)
    lcd_id = (id_buffer[0] << 16) | (id_buffer[1] << 8) | id_buffer[2];
    
    return lcd_id;
}

void LCD_Init(void)
{
	LCD_GPIO_Init();//初始化GPIO
	
	LCD_RES_Clr();//复位
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);
	

	
	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); //Sleep out 
	delay_ms(120);              //Delay 120ms 


	//************* Start Initial Sequence **********// 
	LCD_WR_REG(0x36);
	LCD_WR_DATA8(0x00);

	LCD_WR_REG(0x3A);			
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);			
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x33); 
	LCD_WR_DATA8(0x33); 			

	LCD_WR_REG(0xB7);			
	LCD_WR_DATA8(0x35);

	LCD_WR_REG(0xBB);			
	LCD_WR_DATA8(0x32); //Vcom=1.35V
					
	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);			
	LCD_WR_DATA8(0x15); //GVDD=4.8V  颜色深度
				
	LCD_WR_REG(0xC4);			
	LCD_WR_DATA8(0x20); //VDV, 0x20:0v

	LCD_WR_REG(0xC6);			
	LCD_WR_DATA8(0x0F); //0x0F:60Hz        	

	LCD_WR_REG(0xD0);			
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1); 

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x33);   
	LCD_WR_DATA8(0x48);   
	LCD_WR_DATA8(0x17);   
	LCD_WR_DATA8(0x14);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x34);   

	LCD_WR_REG(0xE1);     
	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x33);   
	LCD_WR_DATA8(0x48);   
	LCD_WR_DATA8(0x17);   
	LCD_WR_DATA8(0x14);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x34);
	uint32_t id = read_lcd_id();
	printf("lcd id 0x%06X\r\n",id);
	if(id == 0x42C2A9){
		LCD_WR_REG(0x20); 
	}else{
		LCD_WR_REG(0x21); 
	}
	LCD_WR_REG(0x29);
	

	// 修改spi速率
	SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_2);
} 








