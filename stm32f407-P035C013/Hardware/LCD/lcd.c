#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"
#include <stdlib.h>

extern DMA_HandleTypeDef hdma_spi1_tx;

#define MAX_BUFFER_SIZE 512  // 根据可用RAM调整
#define MAX_ALLOWED_DISTANCE 50  // 像素


void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{
	LCD_FillRect_FastStatic(xsta,ysta,xend,yend,color);
}

void LCD_FillRect_FastStatic(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	uint16_t i,j;
	uint16_t xlen=0;
	uint16_t temp;
	xlen=x1-x0+1;	 
	for(i=y0;i<=y1;i++)
	{
		LCD_SetCursor(x0,i);      				//设置光标位置 
		LCD_WriteRAM_Prepare();     			//开始写入GRAM	  
		for(j=0;j<xlen;j++)LCD->LCD_RAM=color;	//显示颜色 	    
	}	 
}

/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_SetCursor(x,y);		//设置光标位置 
	LCD_WriteRAM_Prepare();	//开始写入GRAM
	LCD->LCD_RAM=color;  
}


void LCD_Clear(uint32_t color)
{
	uint32_t index=0;      
	uint32_t totalpoint=LCD_W;
	totalpoint*=LCD_H; 			//得到总点数
	LCD_SetCursor(0x00,0x0000);	//设置光标位置 
	LCD_WriteRAM_Prepare();     		//开始写入GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD->LCD_RAM=color;	
	}
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


// 辅助函数：计算最大公约数（用于归一化）
int16_t gcd(int16_t a, int16_t b)
{
    while(b != 0) {
        int16_t t = b;
        b = a % b;
        a = t;
    }
    return a;
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
void LCD_ShowChinese12x12(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont12)/sizeof(typFNT_GB12);  // 汉字数目
    uint16_t TypefaceNum = (sizey/8 + ((sizey%8)?1:0)) * sizey;  // 一个字符所占字节大小
    uint16_t col = 0, row = 0;
    
    for(k=0; k<HZnum; k++) 
    {
        if((tfont12[k].Index[0]==*(s)) && (tfont12[k].Index[1]==*(s+1)))
        {    
            for(i=0; i<TypefaceNum; i++)
            {
                uint8_t temp = tfont12[k].Msk[i];
                
                for(j=0; j<8; j++)
                {
                    uint16_t px = x + col;
                    uint16_t py = y + row;
                    
                    if(px >= LCD_W || py >= LCD_H) continue;
                    
                    // 统一使用从高位到低位的顺序 (0x80 >> j)
                    if(temp & (0x80 >> (7 - j))) {
                        LCD_Fast_DrawPoint(px, py, fc);
                    }
                    else if(!mode) {
                        LCD_Fast_DrawPoint(px, py, bc);
                    }
                    
                    col++;
                    if(col >= sizey) {
                        col = 0;
                        row++;
                        break;
                    }
                }
            }
            break;  // 找到对应汉字后退出循环
        }
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
void LCD_ShowChinese16x16(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont16)/sizeof(typFNT_GB16);
    uint16_t TypefaceNum = (sizey/8 + ((sizey%8)?1:0)) * sizey;
    uint16_t col = 0, row = 0;
    
    for(k=0; k<HZnum; k++) 
    {
        if((tfont16[k].Index[0]==*(s)) && (tfont16[k].Index[1]==*(s+1)))
        {
            for(i=0; i<TypefaceNum; i++)
            {
                uint8_t temp = tfont16[k].Msk[i];
                
                for(j=0; j<8; j++)
                {
                    uint16_t px = x + col;
                    uint16_t py = y + row;
                    
                    if(px >= LCD_W || py >= LCD_H) continue;
                    
                    if(temp & (0x80 >> (7 - j))) {
                        LCD_Fast_DrawPoint(px, py, fc);
                    }
                    else if(!mode) {
                        LCD_Fast_DrawPoint(px, py, bc);
                    }
                    
                    col++;
                    if(col >= sizey) {
                        col = 0;
                        row++;
                        break;
                    }
                }
            }
            break;
        }
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
void LCD_ShowChinese24x24(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont24)/sizeof(typFNT_GB24);
    uint16_t TypefaceNum = (sizey/8 + ((sizey%8)?1:0)) * sizey;
    uint16_t col = 0, row = 0;
    
    for(k=0; k<HZnum; k++) 
    {
        if((tfont24[k].Index[0]==*(s)) && (tfont24[k].Index[1]==*(s+1)))
        {
            for(i=0; i<TypefaceNum; i++)
            {
                uint8_t temp = tfont24[k].Msk[i];
                
                for(j=0; j<8; j++)
                {
                    uint16_t px = x + col;
                    uint16_t py = y + row;
                    
                    if(px >= LCD_W || py >= LCD_H) continue;
                    
                    if(temp & (0x80 >> (7 - j))) {
                        LCD_Fast_DrawPoint(px, py, fc);
                    }
                    else if(!mode) {
                        LCD_Fast_DrawPoint(px, py, bc);
                    }
                    
                    col++;
                    if(col >= sizey) {
                        col = 0;
                        row++;
                        break;
                    }
                }
            }
            break;
        }
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
void LCD_ShowChinese32x32(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont32)/sizeof(typFNT_GB32);
    uint16_t TypefaceNum = (sizey/8 + ((sizey%8)?1:0)) * sizey;
    uint16_t col = 0, row = 0;
    
    for(k=0; k<HZnum; k++) 
    {
        if((tfont32[k].Index[0]==*(s)) && (tfont32[k].Index[1]==*(s+1)))
        {
            for(i=0; i<TypefaceNum; i++)
            {
                uint8_t temp = tfont32[k].Msk[i];
                
                for(j=0; j<8; j++)
                {
                    uint16_t px = x + col;
                    uint16_t py = y + row;
                    
                    if(px >= LCD_W || py >= LCD_H) continue;
                    
                    if(temp & (0x80 >> (7 - j))) {
                        LCD_Fast_DrawPoint(px, py, fc);
                    }
                    else if(!mode) {
                        LCD_Fast_DrawPoint(px, py, bc);
                    }
                    
                    col++;
                    if(col >= sizey) {
                        col = 0;
                        row++;
                        break;
                    }
                }
            }
            break;
        }
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
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t temp, sizex, t;
    uint16_t i, TypefaceNum;
    uint16_t col = 0, row = 0;
    
    // 计算字符宽度
    sizex = sizey / 2;
    // 计算一个字符占用的字节数
    TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
    
    // 字符偏移计算
    num = num - ' ';
    
    for(i = 0; i < TypefaceNum; i++)
    {
        // 获取字体数据
        if(sizey == 12) temp = ascii_1206[num][i];
        else if(sizey == 16) temp = ascii_1608[num][i];
        else if(sizey == 24) temp = ascii_2412[num][i];
        else if(sizey == 32) temp = ascii_3216[num][i];
        else return;
        
        // 处理每个字节的8位
        for(t = 0; t < 8; t++)
        {
            // 计算当前像素位置
            uint16_t px = x + col;
            uint16_t py = y + row;
            
            if(px >= LCD_W || py >= LCD_H) continue;
            
            if(temp & (0x80  >> (7 - t)))  // 注意位顺序，可能需要调整
            {
                LCD_Fast_DrawPoint(px, py, fc);
            }
            else if(!mode)  // 非叠加模式绘制背景
            {
                LCD_Fast_DrawPoint(px, py, bc);
            }
            
            col++;
            if(col >= sizex)
            {
                col = 0;
                row++;
                break;  // 跳到下一行
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
    uint16_t i, j;
    uint16_t color;
    uint32_t index = 0; // 用于遍历 pic[] 数组

    for (i = 0; i < width; i++)
    {
        LCD_SetCursor(x, y + i);    // 设置光标位置 
        LCD_WriteRAM_Prepare();     // 开始写入GRAM
        for (j = 0; j < length; j++)
        {
            // 组合两个8位数据成一个16位颜色值（高位在前或低位在前取决于LCD）
//            color = (pic[index + 1] << 8) | pic[index]; // 假设 pic 是小端模式（低字节在前）
            // 如果 pic 是大端模式（高字节在前），则改为：
            color = (pic[index] << 8) | pic[index + 1];
            
            LCD->LCD_RAM = color; // 写入颜色数据
            index += 2; // 每次读取2字节
        }
    }
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
        LCD_Fill(x0 - x, y0 + y, x0 + x, y0 + y, color);
        LCD_Fill(x0 - y, y0 + x, x0 + y, y0 + x, color);
        LCD_Fill(x0 - x, y0 - y, x0 + x, y0 - y, color);
        LCD_Fill(x0 - y, y0 - x, x0 + y, y0 - x, color);
        
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

