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
    
    // 配置DC(RS), CS引脚为输出
    GPIO_InitStruct.Pin =  GPIO_PIN_3 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置BLK(可选), RES引脚为输出
    GPIO_InitStruct.Pin =  GPIO_PIN_1 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
      函数说明：LCD写入数据(16位)
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

void LCD_Init(void)
{
	LCD_GPIO_Init();//初始化GPIO
	
	LCD_RES_Clr();//复位
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);
	
	//************* ST7789P3-HSD1.54IPS 240x240 初始化序列 **********//
	LCD_WR_REG(0x11);   //Sleep out
	delay_ms(120);

	LCD_WR_REG(0xB2);   //Porch control
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0x35);   //TE on
	LCD_WR_DATA8(0x00);

	LCD_WR_REG(0x36);   //MADCTL: 0x00 竖屏正向(显示上下颠倒时改为0xC0)
	LCD_WR_DATA8(0x00);

	LCD_WR_REG(0x3A);   //像素格式: 0x05 = RGB565
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB7);   //Gate control
	LCD_WR_DATA8(0x00);

	LCD_WR_REG(0xBB);   //VCOM
	LCD_WR_DATA8(0x35);

	LCD_WR_REG(0xC0);   //Power control 2
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);   //Power control 3
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);   //Power control 4
	LCD_WR_DATA8(0x13);

	LCD_WR_REG(0xC4);   //Power control 5
	LCD_WR_DATA8(0x20);

	LCD_WR_REG(0xC6);   //帧率控制
	LCD_WR_DATA8(0x13);

	LCD_WR_REG(0xD6);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xD0);   //Power control 1
	LCD_WR_DATA8(0xA7);
	LCD_WR_DATA8(0xA1);

	delay_ms(10);

	LCD_WR_REG(0xD0);
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xD6);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);   //Gamma 正极性
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x0E);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x30);
	LCD_WR_DATA8(0x43);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x39);
	LCD_WR_DATA8(0x12);
	LCD_WR_DATA8(0x12);
	LCD_WR_DATA8(0x29);
	LCD_WR_DATA8(0x30);

	LCD_WR_REG(0xE1);   //Gamma 负极性
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x10);
	LCD_WR_DATA8(0x12);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x07);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x36);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x31);

	LCD_WR_REG(0xE4);   //Gate control: (0x1D+1)*8 = 240 gate
	LCD_WR_DATA8(0x1D);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);

	LCD_WR_REG(0x21);   //Display inversion on

	LCD_WR_REG(0x29);   //Display on

	LCD_WR_REG(0x2A);   //列地址 0~239
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0xEF);

	LCD_WR_REG(0x2B);   //行地址 0~239
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0xEF);

	LCD_WR_REG(0x2C);   //储存器写

	// 修改spi速率
	SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_2);
}
