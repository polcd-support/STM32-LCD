#include "lcd_init.h"
#include "delay.h"

SRAM_HandleTypeDef TFTSRAM_Handler;    //SRAM���(���ڿ���LCD)

void LCD_GPIO_Init(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOBʱ��

	GPIO_InitTypeDef GPIO_Initure;
	GPIO_Initure.Pin=GPIO_PIN_15;          	//PB15,�������
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
	GPIO_Initure.Pull=GPIO_PULLUP;          //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
	HAL_GPIO_Init(GPIOB,&GPIO_Initure); 
}


//д�Ĵ�������
//regval:�Ĵ���ֵ
void LCD_WR_REG(__IO uint16_t regval)
{   
	regval=regval;		//ʹ��-O2�Ż���ʱ��,����������ʱ
	LCD->LCD_REG=regval;//д��Ҫд�ļĴ������	 
}
//дLCD����
//data:Ҫд���ֵ
void LCD_WR_DATA(__IO uint16_t data)
{	  
	data=data;			//ʹ��-O2�Ż���ʱ��,����������ʱ
	LCD->LCD_RAM=data;		 
}
//��LCD����
//����ֵ:������ֵ
uint16_t LCD_RD_DATA(void)
{
	__IO uint16_t ram;			//��ֹ���Ż�
	ram=LCD->LCD_RAM;	
	return ram;	 
}					   
//д�Ĵ���
//LCD_Reg:�Ĵ�����ַ
//LCD_RegValue:Ҫд�������
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{	
	LCD->LCD_REG = LCD_Reg;		//д��Ҫд�ļĴ������	 
	LCD->LCD_RAM = LCD_RegValue;//д������	    		 
}	   
//���Ĵ���
//LCD_Reg:�Ĵ�����ַ
//����ֵ:����������
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		//д��Ҫ���ļĴ������
	delay_us(5);		  
	return LCD_RD_DATA();		//���ض�����ֵ
}   
//��ʼдGRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD->LCD_REG=0X2C;
}	 
//LCDдGRAM
//RGB_Code:��ɫֵ
void LCD_WriteRAM(uint16_t RGB_Code)
{							    
	LCD->LCD_RAM = RGB_Code;//дʮ��λGRAM
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

//SRAM�ײ�������ʱ��ʹ�ܣ����ŷ���
//�˺����ᱻHAL_SRAM_Init()����
//hsram:SRAM���
void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{	
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_FSMC_CLK_ENABLE();			//ʹ��FSMCʱ��
	__HAL_RCC_GPIOD_CLK_ENABLE();			//ʹ��GPIODʱ��
	__HAL_RCC_GPIOE_CLK_ENABLE();			//ʹ��GPIOEʱ��
	__HAL_RCC_GPIOF_CLK_ENABLE();			//ʹ��GPIOFʱ��
	__HAL_RCC_GPIOG_CLK_ENABLE();			//ʹ��GPIOGʱ��
	
	//��ʼ��PD0,1,4,5,8,9,10,14,15
	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|\
					 GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_Initure.Mode=GPIO_MODE_AF_PP; 		//���츴��
	GPIO_Initure.Pull=GPIO_PULLUP;			//����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;		//����
	GPIO_Initure.Alternate=GPIO_AF12_FSMC;	//����ΪFSMC
	HAL_GPIO_Init(GPIOD,&GPIO_Initure);     //��ʼ��
	
	//��ʼ��PE7,8,9,10,11,12,13,14,15
	GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|\
                     GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE,&GPIO_Initure);
	
	//��ʼ��PF12
	GPIO_Initure.Pin=GPIO_PIN_12;
	HAL_GPIO_Init(GPIOF,&GPIO_Initure);
	
	//��ʼ��PG12
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
    
	TFTSRAM_Handler.Init.NSBank=FSMC_NORSRAM_BANK4;     				//ʹ��NE4
	TFTSRAM_Handler.Init.DataAddressMux=FSMC_DATA_ADDRESS_MUX_DISABLE; 	//��ַ/�����߲�����
	TFTSRAM_Handler.Init.MemoryType=FSMC_MEMORY_TYPE_SRAM;   			//SRAM
	TFTSRAM_Handler.Init.MemoryDataWidth=FSMC_NORSRAM_MEM_BUS_WIDTH_16; //16λ���ݿ��
	TFTSRAM_Handler.Init.BurstAccessMode=FSMC_BURST_ACCESS_MODE_DISABLE; //�Ƿ�ʹ��ͻ������,����ͬ��ͻ���洢����Ч,�˴�δ�õ�
	TFTSRAM_Handler.Init.WaitSignalPolarity=FSMC_WAIT_SIGNAL_POLARITY_LOW;//�ȴ��źŵļ���,����ͻ��ģʽ����������
	TFTSRAM_Handler.Init.WaitSignalActive=FSMC_WAIT_TIMING_BEFORE_WS;   //�洢�����ڵȴ�����֮ǰ��һ��ʱ�����ڻ��ǵȴ������ڼ�ʹ��NWAIT
	TFTSRAM_Handler.Init.WriteOperation=FSMC_WRITE_OPERATION_ENABLE;    //�洢��дʹ��
	TFTSRAM_Handler.Init.WaitSignal=FSMC_WAIT_SIGNAL_DISABLE;           //�ȴ�ʹ��λ,�˴�δ�õ�
	TFTSRAM_Handler.Init.ExtendedMode=FSMC_EXTENDED_MODE_ENABLE;        //��дʹ�ò�ͬ��ʱ��
	TFTSRAM_Handler.Init.AsynchronousWait=FSMC_ASYNCHRONOUS_WAIT_DISABLE;//�Ƿ�ʹ��ͬ������ģʽ�µĵȴ��ź�,�˴�δ�õ�
	TFTSRAM_Handler.Init.WriteBurst=FSMC_WRITE_BURST_DISABLE;           //��ֹͻ��д
	TFTSRAM_Handler.Init.ContinuousClock=FSMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
    
	//FMC��ʱ����ƼĴ���
	FSMC_ReadWriteTim.AddressSetupTime=0x0F;       	//��ַ����ʱ�䣨ADDSET��Ϊ16��HCLK 1/168M=6ns*16=96ns
	FSMC_ReadWriteTim.AddressHoldTime=0;
	FSMC_ReadWriteTim.DataSetupTime=60;				//���ݱ���ʱ��Ϊ60��HCLK	=6*60=360ns
	FSMC_ReadWriteTim.AccessMode=FSMC_ACCESS_MODE_A;//ģʽA
	//FMCдʱ����ƼĴ���
	FSMC_WriteTim.BusTurnAroundDuration=0;			//������ת�׶γ���ʱ��Ϊ0���˱�������ֵ�Ļ���Ī��������Զ��޸�Ϊ4�����³�����������
	FSMC_WriteTim.AddressSetupTime=9;          		//��ַ����ʱ�䣨ADDSET��Ϊ77��HCLK =54ns 
	FSMC_WriteTim.AddressHoldTime=0;
	FSMC_WriteTim.DataSetupTime=17;              	//���ݱ���ʱ��Ϊ6ns*9��HCLK=54n
	FSMC_WriteTim.AccessMode=FSMC_ACCESS_MODE_A;    //ģʽA
	HAL_SRAM_Init(&TFTSRAM_Handler,&FSMC_ReadWriteTim,&FSMC_WriteTim);	
	
	delay_ms(50); // delay 50 ms 
 	LCD_WriteReg(0x0000,0x0001);
	delay_ms(50); // delay 50 ms 


	FSMC_Bank1E->BWTR[6]&=~(0XF<<0);//��ַ����ʱ��(ADDSET)���� 	 
	FSMC_Bank1E->BWTR[6]&=~(0XF<<8);//���ݱ���ʱ������
	FSMC_Bank1E->BWTR[6]|=3<<0;		//��ַ����ʱ��(ADDSET)Ϊ3��HCLK =18ns  	 
	FSMC_Bank1E->BWTR[6]|=2<<8; 	//���ݱ���ʱ��(DATAST)Ϊ6ns*3��HCLK=18ns
	

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








