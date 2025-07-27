#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"
#include <stdlib.h>

extern DMA_HandleTypeDef hdma_spi1_tx;

#define MAX_BUFFER_SIZE 512  // ���ݿ���RAM����
#define MAX_ALLOWED_DISTANCE 50  // ����


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
		LCD_SetCursor(x0,i);      				//���ù��λ�� 
		LCD_WriteRAM_Prepare();     			//��ʼд��GRAM	  
		for(j=0;j<xlen;j++)LCD->LCD_RAM=color;	//��ʾ��ɫ 	    
	}	 
}

/******************************************************************************
      ����˵������ָ��λ�û���
      ������ݣ�x,y ��������
                color �����ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_SetCursor(x,y);		//���ù��λ�� 
	LCD_WriteRAM_Prepare();	//��ʼд��GRAM
	LCD->LCD_RAM=color;  
}


void LCD_Clear(uint32_t color)
{
	uint32_t index=0;      
	uint32_t totalpoint=LCD_W;
	totalpoint*=LCD_H; 			//�õ��ܵ���
	LCD_SetCursor(0x00,0x0000);	//���ù��λ�� 
	LCD_WriteRAM_Prepare();     		//��ʼд��GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD->LCD_RAM=color;	
	}
}


/******************************************************************************
      ����˵��������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   �ߵ���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1;
	uRow=x1;//�����������
	uCol=y1;
	if(delta_x>0)incx=1; //���õ������� 
	else if (delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//ˮƽ�� 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//����
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


// �����������������Լ�������ڹ�һ����
int16_t gcd(int16_t a, int16_t b)
{
    while(b != 0) {
        int16_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

//���ݴ�����ר�в���
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(uint16_t x0,uint16_t y0,uint16_t len,uint16_t color)
{
	if(len==0)return;
	LCD_DrawLine(x0,y0,x0+len-1,y0,color);	
}

void gui_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{                                              
    uint32_t i;
    uint32_t imax = (r * 724) >> 10;  // r * 707/1000 �� r * 724/1024
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
        // �����ڲ���
        gui_draw_hline(x0 - x, y0 + i, 2 * x, color);
        gui_draw_hline(x0 - x, y0 - i, 2 * x, color);
        
        i_squared += (i << 1) + 1;  // ������һ��i��ƽ��
    }
}

/******************************************************************************
      ����˵����������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   �ߵ���ɫ
                size �ߵĿ��(����)
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawThickLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint8_t size)
{
	if(size == 1) {
        LCD_DrawLine(x1, y1, x2, y2, color);
        return;
    }
    
    // ���ٱ߽���
    if(x1 < size || x2 < size || y1 < size || y2 < size) return;
    
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    
    // ���پ����飨���ƣ�
    uint16_t abs_dx = dx > 0 ? dx : -dx;
    uint16_t abs_dy = dy > 0 ? dy : -dy;
    if((abs_dx > MAX_ALLOWED_DISTANCE) || (abs_dy > MAX_ALLOWED_DISTANCE)) {
        return;
    }
//    
//     // ����ֱ�ߵ����
//    if(dx == 0) {
//        // ���ƴ�ֱ�ߵ�����
//        for(uint8_t i = 0; i < thickness; i++) {
//            LCD_DrawLine(x1 + i - thickness/2, y1, x2 + i - thickness/2, y2, color);
//        }
//        // �������˵İ�Բ
//        LCD_DrawCircle(x1, y1, thickness/2, color, 1); // ���Բ
//        LCD_DrawCircle(x2, y2, thickness/2, color, 1); // �յ�Բ
//        return;
//    }
//    
//    // ����ˮƽ�ߵ����
//    if(dy == 0) {
//        // ����ˮƽ�ߵ�����
//        for(uint8_t i = 0; i < thickness; i++) {
//            LCD_DrawLine(x1, y1 + i - thickness/2, x2, y2 + i - thickness/2, color);
//        }
//        // �������˵İ�Բ
//        LCD_DrawCircle(x1, y1, thickness/2, color, 1); // ���Բ
//        LCD_DrawCircle(x2, y2, thickness/2, color, 1); // �յ�Բ
//        return;
//    }
//    
//    // �����ߵĴ�ֱ����
//    float nx = -dy;
//    float ny = dx;
//    
//    // ��һ��
//	int16_t gcd_val = gcd(abs(nx), abs(ny));
//    if(gcd_val != 0) {
//        nx /= gcd_val;
//        ny /= gcd_val;
//    }
//    
//    // ����ƫ����
//    float offset = (thickness - 1) / 2.0f;
//    
//    // ���ƶ���ƽ�����γɿ�������
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
//    // �������˵�Բ�ζ˵�
//    LCD_DrawCircle(x1, y1, thickness/2, color, 1); // ���Բ
//    LCD_DrawCircle(x2, y2, thickness/2, color, 1); // �յ�Բ

	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
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
    // �߽���
    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;
    if(x0 >= SCREEN_WIDTH) x0 = SCREEN_WIDTH - 1;
    if(y0 >= SCREEN_HEIGHT) y0 = SCREEN_HEIGHT - 1;
    if(x1 >= SCREEN_WIDTH) x1 = SCREEN_WIDTH - 1;
    if(y1 >= SCREEN_HEIGHT) y1 = SCREEN_HEIGHT - 1;
    
    // �򻯰���߻���
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    
    for(;;) {
        // ���ֵ�
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
      ����˵����������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   ���ε���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}


/******************************************************************************
      ����˵������Բ
      ������ݣ�x0,y0   Բ������
                r       �뾶
                color   Բ����ɫ
      ����ֵ��  ��
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
		if((a*a+b*b)>(r*r))//�ж�Ҫ���ĵ��Ƿ��Զ
		{
			b--;
		}
	}
}

/******************************************************************************
      ����˵������ʾ���ִ�
      ������ݣ�x,y��ʾ����
                *s Ҫ��ʾ�ĺ��ִ�
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ� ��ѡ 16 24 32
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
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
      ����˵������ʾ����12x12����
      ������ݣ�x,y��ʾ����
                *s Ҫ��ʾ�ĺ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChinese12x12(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont12)/sizeof(typFNT_GB12);  // ������Ŀ
    uint16_t TypefaceNum = (sizey/8 + ((sizey%8)?1:0)) * sizey;  // һ���ַ���ռ�ֽڴ�С
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
                    
                    // ͳһʹ�ôӸ�λ����λ��˳�� (0x80 >> j)
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
            break;  // �ҵ���Ӧ���ֺ��˳�ѭ��
        }
    }
}


/******************************************************************************
      ����˵������ʾ����16x16����
      ������ݣ�x,y��ʾ����
                *s Ҫ��ʾ�ĺ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
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
      ����˵������ʾ����24x24����
      ������ݣ�x,y��ʾ����
                *s Ҫ��ʾ�ĺ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
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
      ����˵������ʾ����32x32����
      ������ݣ�x,y��ʾ����
                *s Ҫ��ʾ�ĺ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
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
      ����˵������ʾ�����ַ�
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ���ַ�
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t temp, sizex, t;
    uint16_t i, TypefaceNum;
    uint16_t col = 0, row = 0;
    
    // �����ַ����
    sizex = sizey / 2;
    // ����һ���ַ�ռ�õ��ֽ���
    TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
    
    // �ַ�ƫ�Ƽ���
    num = num - ' ';
    
    for(i = 0; i < TypefaceNum; i++)
    {
        // ��ȡ��������
        if(sizey == 12) temp = ascii_1206[num][i];
        else if(sizey == 16) temp = ascii_1608[num][i];
        else if(sizey == 24) temp = ascii_2412[num][i];
        else if(sizey == 32) temp = ascii_3216[num][i];
        else return;
        
        // ����ÿ���ֽڵ�8λ
        for(t = 0; t < 8; t++)
        {
            // ���㵱ǰ����λ��
            uint16_t px = x + col;
            uint16_t py = y + row;
            
            if(px >= LCD_W || py >= LCD_H) continue;
            
            if(temp & (0x80  >> (7 - t)))  // ע��λ˳�򣬿�����Ҫ����
            {
                LCD_Fast_DrawPoint(px, py, fc);
            }
            else if(!mode)  // �ǵ���ģʽ���Ʊ���
            {
                LCD_Fast_DrawPoint(px, py, bc);
            }
            
            col++;
            if(col >= sizex)
            {
                col = 0;
                row++;
                break;  // ������һ��
            }
        }
    }
}


/******************************************************************************
      ����˵������ʾ�ַ���
      ������ݣ�x,y��ʾ����
                *p Ҫ��ʾ���ַ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
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
      ����˵������ʾ����
      ������ݣ�m������nָ��
      ����ֵ��  ��
******************************************************************************/
uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      ����˵������ʾ��������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ��������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
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
      ����˵������ʾ��λС������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾС������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
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
      ����˵������ʾͼƬ
      ������ݣ�x,y�������
                length ͼƬ����
                width  ͼƬ���
                pic[]  ͼƬ����    
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[])
{
    uint16_t i, j;
    uint16_t color;
    uint32_t index = 0; // ���ڱ��� pic[] ����

    for (i = 0; i < width; i++)
    {
        LCD_SetCursor(x, y + i);    // ���ù��λ�� 
        LCD_WriteRAM_Prepare();     // ��ʼд��GRAM
        for (j = 0; j < length; j++)
        {
            // �������8λ���ݳ�һ��16λ��ɫֵ����λ��ǰ���λ��ǰȡ����LCD��
//            color = (pic[index + 1] << 8) | pic[index]; // ���� pic ��С��ģʽ�����ֽ���ǰ��
            // ��� pic �Ǵ��ģʽ�����ֽ���ǰ�������Ϊ��
            color = (pic[index] << 8) | pic[index + 1];
            
            LCD->LCD_RAM = color; // д����ɫ����
            index += 2; // ÿ�ζ�ȡ2�ֽ�
        }
    }
}

/* ������ɫ�� */
void DrawColorBars(void) {
    uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA,BLACK,WHITE};
    for (int i=0; i<8; i++) {
        LCD_FillRect_FastStatic(i*40, 0, ((i+1)*40)-1, SCREEN_HEIGHT-1, colors[i]);
    }
}

/* ���ƻҶȽ��� */
void DrawGrayscale(void) {
    #define GRAY_LEVELS 24
    const uint16_t level_width = SCREEN_WIDTH / GRAY_LEVELS;
    const uint16_t remainder = SCREEN_WIDTH % GRAY_LEVELS;

    for (uint8_t n = 0; n < GRAY_LEVELS; n++) {
        // ���㵱ǰ�Ҷȼ�����Χ
        uint16_t start_x = n * level_width;
        uint16_t end_x = start_x + level_width - 1;

        // �����������أ��������һ��
        if (n == GRAY_LEVELS - 1) {
            end_x += remainder;
        }

        // ����24���Ҷ�ֵ (0~23 �� 0~255)
        uint8_t gray = (n * 255) / (GRAY_LEVELS - 1);
        
        // ����RGB565��ɫ����ȷ����ʵ��RGB�꣩
        uint16_t color = RGB(gray, gray, gray);
        
        // ���Ҷȴ�����
        LCD_FillRect_FastStatic(start_x, 0, end_x, SCREEN_HEIGHT-1, color);
    }
}

/* ����������ť */
void DrawClearButton(void) {
    LCD_FillRect_FastStatic(SCREEN_WIDTH-BTN_WIDTH, SCREEN_HEIGHT-BTN_HEIGHT, 
            SCREEN_WIDTH, SCREEN_HEIGHT, GRAY);
    LCD_ShowString(SCREEN_WIDTH-BTN_WIDTH+5, SCREEN_HEIGHT-BTN_HEIGHT+8, 
                  "Clear", BLACK, GRAY, 16, 0);
}


/**
  * @brief  ��ָ��λ�����һ��Բ
  * @param  x0,y0: Բ������
  * @param  r: Բ�İ뾶
  * @param  color: �����ɫ
  * @retval ��
  */
void LCD_FillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color) {
    if (r == 0) return;
    
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 0;
    
    while (x >= y) {
        // ���ˮƽ��
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

