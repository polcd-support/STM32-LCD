#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"
#include "spi.h"
#include "dma.h"
#include <stdlib.h>

extern DMA_HandleTypeDef hdma_spi1_tx;
extern SPI_HandleTypeDef hspi1;

#define MAX_BUFFER_SIZE 192  // 根据可用RAM调整
#define MAX_ALLOWED_DISTANCE 50  // 像素

volatile uint8_t dmaTransferComplete = 1;


// DMA传输完成回调函数
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) {
        dmaTransferComplete = 1;
    }
}

// DMA错误处理回调
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) {
        // 处理错误，例如重试或报错
        dmaTransferComplete = 1;
        LCD_CS_Set();  // 确保CS拉高
        // 可以添加错误处理代码
    }
}


uint32_t RGB565_to_RGB666_32bit(uint16_t color) {
    // 提取 RGB565 的 R、G、B
    uint8_t r = (color >> 8) & 0xF8;  // R: 5-bit → 左移3位对齐到8-bit
    uint8_t g = (color >> 3) & 0xFC;  // G: 6-bit → 左移2位对齐到8-bit
    uint8_t b = (color << 3) & 0xF8;  // B: 5-bit → 左移3位对齐到8-bit

    // 转换为 RGB666 + 2位空（32-bit）
    uint32_t rgb666 = ((uint32_t)r << 24) |  // R: 高8-bit（实际6-bit + 2-bit空）
                      ((uint32_t)g << 16) |  // G: 中8-bit（6-bit + 2-bit空）
                      ((uint32_t)b << 8);    // B: 低8-bit（6-bit + 2-bit空）

    return rgb666;
}

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/

void LCD_FillRect_FastStatic(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	// 等待前一次DMA传输完成
    while(dmaTransferComplete == 0) {}
//    LCD_Address_Set(x0, y0, x1, y1); // 设置显示范围

//    uint32_t pixelCount = (x1 - x0 + 1) * (y1 - y0 + 1);
//    
//    uint8_t r = (color >> 8) & 0xF8;    // 高5位红色
//    uint8_t g = (color >> 3) & 0xFC;    // 中6位绿色
//    uint8_t b = (color << 3);           // 低5位蓝色



//    // 静态缓冲区（分块传输），每个像素现在需要3字节
//    uint8_t buffer[MAX_BUFFER_SIZE * 3];  
//    uint32_t pixelsPerBlock = MAX_BUFFER_SIZE / 3;  // 计算每块能容纳的像素数
//    uint32_t remaining = pixelCount;

//    LCD_CS_Clr();
//    LCD_DC_Set();

//    while (remaining > 0) {
//        uint32_t currentPixels = (remaining > pixelsPerBlock) ? pixelsPerBlock : remaining;
//        // 填充当前块
//		// 填充缓冲区（32-bit/像素）
//        for (uint32_t i = 0; i < currentPixels; i++) {
//            buffer[i * 3] = r;      // 红色分量
//            buffer[i * 3 + 1] = g; // 绿色分量
//            buffer[i * 3 + 2] = b; // 蓝色分量
//        }

//        // 等待前一次DMA传输完成
//        while(dmaTransferComplete == 0) {
//            // 可以添加超时处理
//        }
//        
//        dmaTransferComplete = 0;
//        // 发送当前块（每个像素3字节）
//        HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, buffer, currentPixels*3,1000);
//        if (status != HAL_OK) {
//            // 处理错误
//            dmaTransferComplete = 1;
//            break;
//        }
//        remaining -= currentPixels;
//    }
//    // 等待最后一次DMA传输完成
//    while(dmaTransferComplete == 0) {
//        __nop();
//    }
//    LCD_CS_Set();



	LCD_Address_Set(x0, y0, x1, y1); // 设置显示范围

    uint32_t pixelCount = (x1 - x0 + 1) * (y1 - y0 + 1);
    
    // 预计算RGB分量（RGB565转RGB888格式）
    uint8_t r = (color >> 8) & 0xF8;    // 高5位红色
    uint8_t g = (color >> 3) & 0xFC;    // 中6位绿色
    uint8_t b = (color << 3);           // 低5位蓝色

    // 静态缓冲区（分块传输），每个像素现在需要3字节
    uint8_t buffer[MAX_BUFFER_SIZE * 3];  
    uint32_t pixelsPerBlock = MAX_BUFFER_SIZE / 3;  // 计算每块能容纳的像素数
    uint32_t remaining = pixelCount;

    LCD_CS_Clr();
    LCD_DC_Set();

    while (remaining > 0) {
        uint32_t currentPixels = (remaining > pixelsPerBlock) ? pixelsPerBlock : remaining;
        
        // 填充当前块
        for (uint32_t i = 0; i < currentPixels; i++) {
            buffer[i * 3] = r;      // 红色分量
            buffer[i * 3 + 1] = g; // 绿色分量
            buffer[i * 3 + 2] = b; // 蓝色分量
        }


        // 等待前一次DMA传输完成
        while(dmaTransferComplete == 0) {
            // 可以添加超时处理
        }
		dmaTransferComplete = 0;
        // 发送当前块（每个像素3字节）
        HAL_SPI_Transmit_DMA(&hspi1, buffer, currentPixels * 3);
        remaining -= currentPixels;
    }
    while(dmaTransferComplete == 0) {
        __nop();
    }
    LCD_CS_Set();
}

/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA(color);
} 


/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

//电容触摸屏专有部分
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(uint16_t x0,uint16_t y0,uint16_t len,uint16_t color)
{
	if(len==0)return;
	LCD_DrawLine(x0,y0,x0+len-1,y0,color);	
}

void gui_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{                                              
    uint32_t i;
    uint32_t imax = (r * 724) >> 10;  // r * 707/1000 ≈ r * 724/1024
    uint32_t sqmax = r * r + (r >> 1);
    uint32_t x = r;
    uint32_t i_squared = 1;  // 1^2 = 1
    
    gui_draw_hline(x0 - r, y0, 2 * r, color);
    
    for (i = 1; i < imax + 1; i++) {
        if ((i_squared + x * x) > sqmax) {
            if (x > imax) {
                gui_draw_hline(x0 - i + 1, y0 + x, 2 * (i - 1), color);
                gui_draw_hline(x0 - i + 1, y0 - x, 2 * (i - 1), color);
            }
            x--;
        }
        // 绘制内部线
        gui_draw_hline(x0 - x, y0 + i, 2 * x, color);
        gui_draw_hline(x0 - x, y0 - i, 2 * x, color);
        
        i_squared += (i << 1) + 1;  // 计算下一个i的平方
    }
}

/******************************************************************************
      函数说明：画宽线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
                size 线的宽度(像素)
      返回值：  无
******************************************************************************/
void LCD_DrawThickLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint8_t size)
{
	if(size == 1) {
        LCD_DrawLine(x1, y1, x2, y2, color);
        return;
    }
    
    // 快速边界检查
    if(x1 < size || x2 < size || y1 < size || y2 < size) return;
    
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    
    // 快速距离检查（近似）
    uint16_t abs_dx = dx > 0 ? dx : -dx;
    uint16_t abs_dy = dy > 0 ? dy : -dy;
    if((abs_dx > MAX_ALLOWED_DISTANCE) || (abs_dy > MAX_ALLOWED_DISTANCE)) {
        return;
    }
//    
//     // 处理垂直线的情况
//    if(dx == 0) {
//        // 绘制垂直线的主体
//        for(uint8_t i = 0; i < thickness; i++) {
//            LCD_DrawLine(x1 + i - thickness/2, y1, x2 + i - thickness/2, y2, color);
//        }
//        // 绘制两端的半圆
//        LCD_DrawCircle(x1, y1, thickness/2, color, 1); // 起点圆
//        LCD_DrawCircle(x2, y2, thickness/2, color, 1); // 终点圆
//        return;
//    }
//    
//    // 处理水平线的情况
//    if(dy == 0) {
//        // 绘制水平线的主体
//        for(uint8_t i = 0; i < thickness; i++) {
//            LCD_DrawLine(x1, y1 + i - thickness/2, x2, y2 + i - thickness/2, color);
//        }
//        // 绘制两端的半圆
//        LCD_DrawCircle(x1, y1, thickness/2, color, 1); // 起点圆
//        LCD_DrawCircle(x2, y2, thickness/2, color, 1); // 终点圆
//        return;
//    }
//    
//    // 计算线的垂直方向
//    float nx = -dy;
//    float ny = dx;
//    
//    // 归一化
//	int16_t gcd_val = gcd(abs(nx), abs(ny));
//    if(gcd_val != 0) {
//        nx /= gcd_val;
//        ny /= gcd_val;
//    }
//    
//    // 计算偏移量
//    float offset = (thickness - 1) / 2.0f;
//    
//    // 绘制多条平行线形成宽线主体
//    for(uint8_t i = 0; i < thickness; i++) {
//        float currOffset = i - offset;
//        int16_t x1_offset = x1 + (int16_t)(nx * currOffset);
//        int16_t y1_offset = y1 + (int16_t)(ny * currOffset);
//        int16_t x2_offset = x2 + (int16_t)(nx * currOffset);
//        int16_t y2_offset = y2 + (int16_t)(ny * currOffset);
//        
//        LCD_DrawLine(x1_offset, y1_offset, x2_offset, y2_offset, color);
//    }
//    
//    // 绘制两端的圆形端点
//    LCD_DrawCircle(x1, y1, thickness/2, color, 1); // 起点圆
//    LCD_DrawCircle(x2, y2, thickness/2, color, 1); // 终点圆

	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}

void DrawThickLine(int x0, int y0, int x1, int y1, int thickness, uint16_t color) {
    // 边界检查
    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;
    if(x0 >= SCREEN_WIDTH) x0 = SCREEN_WIDTH - 1;
    if(y0 >= SCREEN_HEIGHT) y0 = SCREEN_HEIGHT - 1;
    if(x1 >= SCREEN_WIDTH) x1 = SCREEN_WIDTH - 1;
    if(y1 >= SCREEN_HEIGHT) y1 = SCREEN_HEIGHT - 1;
    
    // 简化版粗线绘制
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    
    for(;;) {
        // 画粗点
        for(int i = -thickness; i <= thickness; i++) {
            for(int j = -thickness; j <= thickness; j++) {
                if(x0 + i >= 0 && x0 + i < SCREEN_WIDTH && 
                   y0 + j >= 0 && y0 + j < SCREEN_HEIGHT) {
                    LCD_DrawPoint(x0 + i, y0 + j, color);
                }
            }
        }
        
        if(x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if(e2 >= dy) { err += dy; x0 += sx; }
        if(e2 <= dx) { err += dx; y0 += sy; }
    }
}


/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}


/******************************************************************************
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
******************************************************************************/
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}

/******************************************************************************
      函数说明：显示汉字串
      入口数据：x,y显示坐标
                *s 要显示的汉字串
                fc 字的颜色
                bc 字的背景色
                sizey 字号 可选 16 24 32
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	while(*s!=0)
	{
		if(sizey==12) LCD_ShowChinese12x12(x,y,s,fc,bc,sizey,mode);
		else if(sizey==16) LCD_ShowChinese16x16(x,y,s,fc,bc,sizey,mode);
		else if(sizey==24) LCD_ShowChinese24x24(x,y,s,fc,bc,sizey,mode);
		else if(sizey==32) LCD_ShowChinese32x32(x,y,s,fc,bc,sizey,mode);
		else return;
		s+=2;
		x+=sizey;
	}
}

/******************************************************************************
      函数说明：显示单个12x12汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese12x12(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	                         
	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont12[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 


/******************************************************************************
      函数说明：显示单个16x16汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese16x16(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont16[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont16[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 


/******************************************************************************
      函数说明：显示单个24x24汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese24x24(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont24[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont24[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 

/******************************************************************************
      函数说明：显示单个32x32汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese32x32(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont32[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont32[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
}


/******************************************************************************
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t temp,sizex,t,m=0;
	uint16_t i,TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}


/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}


/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowIntNum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	uint8_t sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/******************************************************************************
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowFloatNum1(uint16_t x,uint16_t y,float num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t t,temp,sizex;
	uint16_t num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}

/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[])
{
        // 计算显示区域的结束坐标
    uint16_t x_end = x + length;
    uint16_t y_end = y + width;
    
    // 设置显示范围
    LCD_Address_Set(x, y, x_end, y_end);
    
    // 计算总像素数
    uint32_t pixelCount = length * width;
    
    // 计算图片数据总字节数
    uint32_t dataSize = pixelCount * 3;
    
    // 分块传输参数
    uint32_t remaining = dataSize;
    uint32_t offset = 0;
    
    LCD_CS_Clr();
    LCD_DC_Set();
    
    // 分块传输图片数据
    while (remaining > 0) {
        uint32_t chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
        
        // 发送当前数据块
        HAL_SPI_Transmit(&hspi1, (uint8_t*)(pic + offset), chunkSize, HAL_MAX_DELAY);
        
        offset += chunkSize;
        remaining -= chunkSize;
    }
    
    LCD_CS_Set();
}


/* 绘制颜色条 */
void DrawColorBars(void) {
    uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA,BLACK,WHITE};
    for (int i=0; i<8; i++) {
        LCD_FillRect_FastStatic(i*40, 0, ((i+1)*40)-1, SCREEN_HEIGHT-1, colors[i]);
    }
}

/* 绘制灰度渐变 */
void DrawGrayscale(void) {
    #define GRAY_LEVELS 24
    const uint16_t level_width = SCREEN_WIDTH / GRAY_LEVELS;
    const uint16_t remainder = SCREEN_WIDTH % GRAY_LEVELS;

    for (uint8_t n = 0; n < GRAY_LEVELS; n++) {
        // 计算当前灰度级区域范围
        uint16_t start_x = n * level_width;
        uint16_t end_x = start_x + level_width - 1;

        // 处理余数像素，加在最后一级
        if (n == GRAY_LEVELS - 1) {
            end_x += remainder;
        }

        // 计算24级灰度值 (0~23 → 0~255)
        uint8_t gray = (n * 255) / (GRAY_LEVELS - 1);
        
        // 生成RGB565颜色（需确保已实现RGB宏）
        uint16_t color = RGB(gray, gray, gray);
        
        // 填充灰度带区域
        LCD_FillRect_FastStatic(start_x, 0, end_x, SCREEN_HEIGHT-1, color);
    }
}

/* 绘制清屏按钮 */
void DrawClearButton(void) {
    LCD_FillRect_FastStatic(SCREEN_WIDTH-BTN_WIDTH, SCREEN_HEIGHT-BTN_HEIGHT, 
            SCREEN_WIDTH, SCREEN_HEIGHT, GRAY);
    LCD_ShowString(SCREEN_WIDTH-BTN_WIDTH+5, SCREEN_HEIGHT-BTN_HEIGHT+8, 
                  "Clear", BLACK, GRAY, 16, 0);
}


/**
  * @brief  在指定位置填充一个圆
  * @param  x0,y0: 圆心坐标
  * @param  r: 圆的半径
  * @param  color: 填充颜色
  * @retval 无
  */
void LCD_FillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color) {
    if (r == 0) return;
    
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 0;
    
    while (x >= y) {
        // 填充水平线
        LCD_FillRect_FastStatic(x0 - x, y0 + y, x0 + x, y0 + y, color);
        LCD_FillRect_FastStatic(x0 - y, y0 + x, x0 + y, y0 + x, color);
        LCD_FillRect_FastStatic(x0 - x, y0 - y, x0 + x, y0 - y, color);
        LCD_FillRect_FastStatic(x0 - y, y0 - x, x0 + y, y0 - x, color);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

