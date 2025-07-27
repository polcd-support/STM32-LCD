#include "lcd_init.h"
#include "delay.h"

SRAM_HandleTypeDef TFTSRAM_Handler;    //SRAM句柄(用于控制LCD)

void LCD_GPIO_Init(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();			//开启GPIOB时钟

	GPIO_InitTypeDef GPIO_Initure;
	GPIO_Initure.Pin=GPIO_PIN_15;          	//PB15,背光控制
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(GPIOB,&GPIO_Initure); 
}


//写寄存器函数
//regval:寄存器值
void LCD_WR_REG(__IO uint16_t regval)
{   
	regval=regval;		//使用-O2优化的时候,必须插入的延时
	LCD->LCD_REG=regval;//写入要写的寄存器序号	 
}
//写LCD数据
//data:要写入的值
void LCD_WR_DATA(__IO uint16_t data)
{	  
	data=data;			//使用-O2优化的时候,必须插入的延时
	LCD->LCD_RAM=data;		 
}
//读LCD数据
//返回值:读到的值
uint16_t LCD_RD_DATA(void)
{
	__IO uint16_t ram;			//防止被优化
	ram=LCD->LCD_RAM;	
	return ram;	 
}					   
//写寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要写入的数据
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{	
	LCD->LCD_REG = LCD_Reg;		//写入要写的寄存器序号	 
	LCD->LCD_RAM = LCD_RegValue;//写入数据	    		 
}	   
//读寄存器
//LCD_Reg:寄存器地址
//返回值:读到的数据
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		//写入要读的寄存器序号
	delay_us(5);		  
	return LCD_RD_DATA();		//返回读到的值
}   
//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD->LCD_REG=0X2C;
}	 
//LCD写GRAM
//RGB_Code:颜色值
void LCD_WriteRAM(uint16_t RGB_Code)
{							    
	LCD->LCD_RAM = RGB_Code;//写十六位GRAM
}


void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint32_t color)
{
	LCD_WR_REG(0x2A); 
	LCD_WR_DATA(x>>8);	LCD_WR_DATA(x&0XFF); 
	LCD_WR_DATA((LCD_W-1)>>8);	LCD_WR_DATA((LCD_W-1)&0XFF);	
	LCD_WR_REG(0x2B); 
	LCD_WR_DATA(y>>8);	LCD_WR_DATA(y&0XFF);
	LCD_WR_DATA((LCD_H-1)>>8);	LCD_WR_DATA((LCD_H-1)&0XFF); 

	LCD->LCD_REG=0x2C; 
	LCD->LCD_RAM=color; 
}


void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD_WR_REG(0X2A); 
	LCD_WR_DATA(Xpos>>8);	LCD_WR_DATA(Xpos&0XFF); 
	LCD_WR_DATA((LCD_W-1)>>8);	LCD_WR_DATA((LCD_W-1)&0XFF);	
	LCD_WR_REG(0X2B); 
	LCD_WR_DATA(Ypos>>8);	LCD_WR_DATA(Ypos&0XFF);
	LCD_WR_DATA((LCD_H-1)>>8);	LCD_WR_DATA((LCD_H-1)&0XFF); 	
}

//SRAM底层驱动，时钟使能，引脚分配
//此函数会被HAL_SRAM_Init()调用
//hsram:SRAM句柄
void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{	
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_FSMC_CLK_ENABLE();			//使能FSMC时钟
	__HAL_RCC_GPIOD_CLK_ENABLE();			//使能GPIOD时钟
	__HAL_RCC_GPIOE_CLK_ENABLE();			//使能GPIOE时钟
	__HAL_RCC_GPIOF_CLK_ENABLE();			//使能GPIOF时钟
	__HAL_RCC_GPIOG_CLK_ENABLE();			//使能GPIOG时钟
	
	//初始化PD0,1,4,5,8,9,10,14,15
	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|\
					 GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_Initure.Mode=GPIO_MODE_AF_PP; 		//推挽复用
	GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;		//高速
	GPIO_Initure.Alternate=GPIO_AF12_FSMC;	//复用为FSMC
	HAL_GPIO_Init(GPIOD,&GPIO_Initure);     //初始化
	
	//初始化PE7,8,9,10,11,12,13,14,15
	GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|\
                     GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE,&GPIO_Initure);
	
	//初始化PF12
	GPIO_Initure.Pin=GPIO_PIN_12;
	HAL_GPIO_Init(GPIOF,&GPIO_Initure);
	
	//初始化PG12
	GPIO_Initure.Pin=GPIO_PIN_12;
	HAL_GPIO_Init(GPIOG,&GPIO_Initure);
}

void LCD_Init(void)
{

	LCD_GPIO_Init();


	FSMC_NORSRAM_TimingTypeDef FSMC_ReadWriteTim;
	FSMC_NORSRAM_TimingTypeDef FSMC_WriteTim;
    
	TFTSRAM_Handler.Instance=FSMC_NORSRAM_DEVICE;                
	TFTSRAM_Handler.Extended=FSMC_NORSRAM_EXTENDED_DEVICE;    
    
	TFTSRAM_Handler.Init.NSBank=FSMC_NORSRAM_BANK4;     				//使用NE4
	TFTSRAM_Handler.Init.DataAddressMux=FSMC_DATA_ADDRESS_MUX_DISABLE; 	//地址/数据线不复用
	TFTSRAM_Handler.Init.MemoryType=FSMC_MEMORY_TYPE_SRAM;   			//SRAM
	TFTSRAM_Handler.Init.MemoryDataWidth=FSMC_NORSRAM_MEM_BUS_WIDTH_16; //16位数据宽度
	TFTSRAM_Handler.Init.BurstAccessMode=FSMC_BURST_ACCESS_MODE_DISABLE; //是否使能突发访问,仅对同步突发存储器有效,此处未用到
	TFTSRAM_Handler.Init.WaitSignalPolarity=FSMC_WAIT_SIGNAL_POLARITY_LOW;//等待信号的极性,仅在突发模式访问下有用
	TFTSRAM_Handler.Init.WaitSignalActive=FSMC_WAIT_TIMING_BEFORE_WS;   //存储器是在等待周期之前的一个时钟周期还是等待周期期间使能NWAIT
	TFTSRAM_Handler.Init.WriteOperation=FSMC_WRITE_OPERATION_ENABLE;    //存储器写使能
	TFTSRAM_Handler.Init.WaitSignal=FSMC_WAIT_SIGNAL_DISABLE;           //等待使能位,此处未用到
	TFTSRAM_Handler.Init.ExtendedMode=FSMC_EXTENDED_MODE_ENABLE;        //读写使用不同的时序
	TFTSRAM_Handler.Init.AsynchronousWait=FSMC_ASYNCHRONOUS_WAIT_DISABLE;//是否使能同步传输模式下的等待信号,此处未用到
	TFTSRAM_Handler.Init.WriteBurst=FSMC_WRITE_BURST_DISABLE;           //禁止突发写
	TFTSRAM_Handler.Init.ContinuousClock=FSMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
    
	//FMC读时序控制寄存器
	FSMC_ReadWriteTim.AddressSetupTime=0x0F;       	//地址建立时间（ADDSET）为16个HCLK 1/168M=6ns*16=96ns
	FSMC_ReadWriteTim.AddressHoldTime=0;
	FSMC_ReadWriteTim.DataSetupTime=60;				//数据保存时间为60个HCLK	=6*60=360ns
	FSMC_ReadWriteTim.AccessMode=FSMC_ACCESS_MODE_A;//模式A
	//FMC写时序控制寄存器
	FSMC_WriteTim.BusTurnAroundDuration=0;			//总线周转阶段持续时间为0，此变量不赋值的话会莫名其妙的自动修改为4。导致程序运行正常
	FSMC_WriteTim.AddressSetupTime=9;          		//地址建立时间（ADDSET）为77个HCLK =54ns 
	FSMC_WriteTim.AddressHoldTime=0;
	FSMC_WriteTim.DataSetupTime=17;              	//数据保存时间为6ns*9个HCLK=54n
	FSMC_WriteTim.AccessMode=FSMC_ACCESS_MODE_A;    //模式A
	HAL_SRAM_Init(&TFTSRAM_Handler,&FSMC_ReadWriteTim,&FSMC_WriteTim);	
	
	delay_ms(50); // delay 50 ms 
 	LCD_WriteReg(0x0000,0x0001);
	delay_ms(50); // delay 50 ms 


	FSMC_Bank1E->BWTR[6]&=~(0XF<<0);//地址建立时间(ADDSET)清零 	 
	FSMC_Bank1E->BWTR[6]&=~(0XF<<8);//数据保存时间清零
	FSMC_Bank1E->BWTR[6]|=3<<0;		//地址建立时间(ADDSET)为3个HCLK =18ns  	 
	FSMC_Bank1E->BWTR[6]|=2<<8; 	//数据保存时间(DATAST)为6ns*3个HCLK=18ns
	

//	9488
	LCD_WR_REG(0xE0); //P-Gamma 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x1B); 
	LCD_WR_DATA(0x1F); 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x3C); 
	LCD_WR_DATA(0x38); 
	LCD_WR_DATA(0x49); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x0C); 
	LCD_WR_DATA(0x0B); 
	LCD_WR_DATA(0x15); 
	LCD_WR_DATA(0x17); 
	LCD_WR_DATA(0x0F); 
	
	LCD_WR_REG(0xE1); //N-Gamma 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x19); 
	LCD_WR_DATA(0x1B); 
	LCD_WR_DATA(0x04); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x04); 
	LCD_WR_DATA(0x30); 
	LCD_WR_DATA(0x32); 
	LCD_WR_DATA(0x40); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x0E); 
	LCD_WR_DATA(0x2B); 
	LCD_WR_DATA(0x32); 
	LCD_WR_DATA(0x0F); 

	LCD_WR_REG(0XF8);	LCD_WR_DATA(0x21);	LCD_WR_DATA(0x04);
	LCD_WR_REG(0XF9);	LCD_WR_DATA(0x00);	LCD_WR_DATA(0x08);
	LCD_WR_REG(0X36);	LCD_WR_DATA(0X08);	
	LCD_WR_REG(0X3A);	LCD_WR_DATA(0x05);
	LCD_WR_REG(0XB4);	LCD_WR_DATA(0x01);	
	LCD_WR_REG(0XB6);	LCD_WR_DATA(0X02);	LCD_WR_DATA(0x22);
	LCD_WR_REG(0XC1);	LCD_WR_DATA(0X41);
	LCD_WR_REG(0XC5);	LCD_WR_DATA(0X00);	LCD_WR_DATA(0x07);
	
	LCD_WR_REG(0x11); 	//Sleep out 
	delay_ms(120); 
	LCD_WR_REG(0x29); 	//Display on 	

} 








