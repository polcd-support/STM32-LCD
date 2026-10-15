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
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);   // 面板RS, 三线模式不使用
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}

/* 单次CS内最多发送的帧数 */
#define LCD_BULK_CHUNK  4096

/* DMA通路状态: 0=正常 1=寄存器写入未生效 2=传输超时(已退回阻塞发送) */
uint8_t lcd_dma_status;

static void LCD_DMA_ConfigSpi(void);

/******************************************************************************
      函数说明：按9bit帧要求配置SPI1的DMA通路(寄存器级, 不依赖CubeMX生成值):
                DMA1_Channel1 的请求源=SPI1_TX; SPI置为1线发送并使能
      返回值：  无
******************************************************************************/
static void LCD_DMA_ConfigSpi(void)
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	DMAMUX1_Channel0->CCR = (uint32_t)DMA_REQUEST_SPI1_TX;   /* DMA1_Channel1 对应 DMAMUX 通道0 */
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);
	SET_BIT(SPI1->CR1, SPI_CR1_SPE | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE);
}

/******************************************************************************
      函数说明：DMA发送一段帧(寄存器级, 不经HAL的DMA状态机):
                每次显式重写 CPAR/CMAR/CNDTR/CCR, 等DMA搬完+SPI移位完成
                回读不符或超时时记录lcd_dma_status, 复位SPI后用阻塞方式补发
      入口数据：frames 帧首地址, count 帧数
      返回值：  无
******************************************************************************/
static void LCD_Send_DMA(const lcd_frame_t *frames, uint32_t count)
{
	const uint32_t cfg = (uint32_t)(DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0 | DMA_CCR_DIR);
	uint32_t start;
	uint8_t done;

	DMA1->IFCR = DMA_IFCR_CGIF1;
	DMA1_Channel1->CCR = 0;
	DMA1_Channel1->CPAR = (uint32_t)&SPI1->DR;
	DMA1_Channel1->CMAR = (uint32_t)frames;
	DMA1_Channel1->CNDTR = count;
	DMA1_Channel1->CCR = cfg;

	if ((DMA1_Channel1->CCR & cfg) != cfg) {        /* 内存递增+半字宽 写入未生效 */
		if (lcd_dma_status == 0) lcd_dma_status = 1;
	}

	SET_BIT(SPI1->CR2, SPI_CR2_TXDMAEN);
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);

	start = HAL_GetTick();
	while ((DMA1->ISR & DMA_ISR_TCIF1) == 0U) {    /* 等DMA搬完 */
		if ((HAL_GetTick() - start) > 200U) break;
	}
	done = ((DMA1->ISR & DMA_ISR_TCIF1) != 0U) ? 1 : 0;

	if (done) {
		start = HAL_GetTick();
		while ((SPI1->SR & SPI_SR_BSY) != 0U) {    /* 等最后几帧移出 */
			if ((HAL_GetTick() - start) > 200U) break;
		}
	}

	CLEAR_BIT(SPI1->CR2, SPI_CR2_TXDMAEN);
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);
	DMA1->IFCR = DMA_IFCR_CGIF1;

	if (!done && lcd_dma_status == 0) {
		lcd_dma_status = 2;                    /* 传输超时 */
	}

	if (lcd_dma_status != 0) {
		/* 判定DMA不可用: 复位SPI清FIFO, 用阻塞方式把本段发出去 */
		CLEAR_BIT(SPI1->CR1, SPI_CR1_SPE);
		SET_BIT(SPI1->CR1, SPI_CR1_SPE | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE);
		HAL_SPI_Transmit(&hspi1, (uint8_t *)frames, (uint16_t)count, 1000);
	}
}

/******************************************************************************
      函数说明：发送一段帧(第9位区分命令/数据), CS框住整段传输
      入口数据：frames 9bit帧数组, count 帧数(不超过65535)
      返回值：  无
******************************************************************************/
static void LCD_Send_Frames(const lcd_frame_t *frames, uint32_t count)
{
	LCD_CS_Clr();
	LCD_Send_DMA(frames, count);
	LCD_CS_Set();
}

/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(uint8_t dat) 
{    
	lcd_frame_t frame = LCD_FRAME_DATA(dat);

	LCD_Send_Frames(&frame, 1);
}

/******************************************************************************
      函数说明：连续写入像素数据帧
      入口数据：frames 帧首地址, count 帧数
      返回值：  无
******************************************************************************/
void LCD_WR_DATA_Bulk(const lcd_frame_t *frames, uint32_t count)
{
	while (count > 0) {
		uint32_t chunk = (count > LCD_BULK_CHUNK) ? LCD_BULK_CHUNK : count;

		LCD_Send_Frames(frames, chunk);
		frames += chunk;
		count -= chunk;
	}
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
	lcd_frame_t frames[2];

	frames[0] = LCD_FRAME_DATA(dat >> 8);
	frames[1] = LCD_FRAME_DATA(dat);

	LCD_Send_Frames(frames, 2);
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(uint8_t dat)
{
	lcd_frame_t frame = dat;       // 第9位=0: 命令

	LCD_Send_Frames(&frame, 1);
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_WR_REG(0x2A);
	LCD_WR_DATA(x1 + LCD_X_OFFSET);
	LCD_WR_DATA(x2 + LCD_X_OFFSET);
	LCD_WR_REG(0x2B);
	LCD_WR_DATA(y1 + LCD_Y_OFFSET);
	LCD_WR_DATA(y2 + LCD_Y_OFFSET);
	LCD_WR_REG(0x2C);
}

void LCD_Init(void)
{
	LCD_GPIO_Init();//初始化GPIO

	LCD_DMA_ConfigSpi();//配置SPI1的DMA通路(DMAMUX请求 + 1线发送使能)

	LCD_BLK_Set();//先使能背光/面板电源, 保证初始化时序有效

	LCD_RES_Set();//复位
	delay_ms(200);

	LCD_RES_Clr();
	delay_ms(100);

	LCD_RES_Set();
	delay_ms(100);

	/* BOE 1.47-inch ST7789P3 initialization sequence. */
	LCD_WR_REG(0x11);
	delay_ms(120);

	LCD_WR_REG(0x36);
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB0);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0xE0);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA8(0x02);

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x19);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x13);

	LCD_WR_REG(0xC6);
	LCD_WR_DATA8(0x0F);

	LCD_WR_REG(0xD0);
	LCD_WR_DATA8(0xA7);

	LCD_WR_REG(0xD0);
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xD6);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xF0);
	LCD_WR_DATA8(0x0F);
	LCD_WR_DATA8(0x16);
	LCD_WR_DATA8(0x07);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x25);
	LCD_WR_DATA8(0x36);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4B);
	LCD_WR_DATA8(0x37);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2E);
	LCD_WR_DATA8(0x35);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xF0);
	LCD_WR_DATA8(0x10);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x06);
	LCD_WR_DATA8(0x23);
	LCD_WR_DATA8(0x35);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x4A);
	LCD_WR_DATA8(0x38);
	LCD_WR_DATA8(0x10);
	LCD_WR_DATA8(0x10);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0x21);

	LCD_WR_REG(0x29);
	delay_ms(20);

	LCD_WR_REG(0x2C);
} 








