#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"
#include "spi.h"
#include "dma.h"
#include <stdlib.h>

extern DMA_HandleTypeDef hdma_spi1_tx;
extern SPI_HandleTypeDef hspi1;

#define MAX_BUFFER_SIZE 192  // ���ݿ���RAM����
#define MAX_ALLOWED_DISTANCE 50  // ����

volatile uint8_t dmaTransferComplete = 1;


// DMA������ɻص�����
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) {
        dmaTransferComplete = 1;
    }
}

// DMA������ص�
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) {
        // ��������������Ի򱨴�
        dmaTransferComplete = 1;
        LCD_CS_Set();  // ȷ��CS����
        // ������Ӵ��������
    }
}


uint32_t RGB565_to_RGB666_32bit(uint16_t color) {
    // ��ȡ RGB565 �� R��G��B
    uint8_t r = (color >> 8) & 0xF8;  // R: 5-bit �� ����3λ���뵽8-bit
    uint8_t g = (color >> 3) & 0xFC;  // G: 6-bit �� ����2λ���뵽8-bit
    uint8_t b = (color << 3) & 0xF8;  // B: 5-bit �� ����3λ���뵽8-bit

    // ת��Ϊ RGB666 + 2λ�գ�32-bit��
    uint32_t rgb666 = ((uint32_t)r << 24) |  // R: ��8-bit��ʵ��6-bit + 2-bit�գ�
                      ((uint32_t)g << 16) |  // G: ��8-bit��6-bit + 2-bit�գ�
                      ((uint32_t)b << 8);    // B: ��8-bit��6-bit + 2-bit�գ�

    return rgb666;
}

/******************************************************************************
      ����˵������ָ�����������ɫ
      ������ݣ�xsta,ysta   ��ʼ����
                xend,yend   ��ֹ����
								color       Ҫ������ɫ
      ����ֵ��  ��
******************************************************************************/

void LCD_FillRect_FastStatic(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	// �ȴ�ǰһ��DMA�������
    while(dmaTransferComplete == 0) {}
//    LCD_Address_Set(x0, y0, x1, y1); // ������ʾ��Χ

//    uint32_t pixelCount = (x1 - x0 + 1) * (y1 - y0 + 1);
//    
//    uint8_t r = (color >> 8) & 0xF8;    // ��5λ��ɫ
//    uint8_t g = (color >> 3) & 0xFC;    // ��6λ��ɫ
//    uint8_t b = (color << 3);           // ��5λ��ɫ



//    // ��̬���������ֿ鴫�䣩��ÿ������������Ҫ3�ֽ�
//    uint8_t buffer[MAX_BUFFER_SIZE * 3];  
//    uint32_t pixelsPerBlock = MAX_BUFFER_SIZE / 3;  // ����ÿ�������ɵ�������
//    uint32_t remaining = pixelCount;

//    LCD_CS_Clr();
//    LCD_DC_Set();

//    while (remaining > 0) {
//        uint32_t currentPixels = (remaining > pixelsPerBlock) ? pixelsPerBlock : remaining;
//        // ��䵱ǰ��
//		// ��仺������32-bit/���أ�
//        for (uint32_t i = 0; i < currentPixels; i++) {
//            buffer[i * 3] = r;      // ��ɫ����
//            buffer[i * 3 + 1] = g; // ��ɫ����
//            buffer[i * 3 + 2] = b; // ��ɫ����
//        }

//        // �ȴ�ǰһ��DMA�������
//        while(dmaTransferComplete == 0) {
//            // ������ӳ�ʱ����
//        }
//        
//        dmaTransferComplete = 0;
//        // ���͵�ǰ�飨ÿ������3�ֽڣ�
//        HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, buffer, currentPixels*3,1000);
//        if (status != HAL_OK) {
//            // �������
//            dmaTransferComplete = 1;
//            break;
//        }
//        remaining -= currentPixels;
//    }
//    // �ȴ����һ��DMA�������
//    while(dmaTransferComplete == 0) {
//        __nop();
//    }
//    LCD_CS_Set();



	LCD_Address_Set(x0, y0, x1, y1); // ������ʾ��Χ

    uint32_t pixelCount = (x1 - x0 + 1) * (y1 - y0 + 1);
    
    // Ԥ����RGB������RGB565תRGB888��ʽ��
    uint8_t r = (color >> 8) & 0xF8;    // ��5λ��ɫ
    uint8_t g = (color >> 3) & 0xFC;    // ��6λ��ɫ
    uint8_t b = (color << 3);           // ��5λ��ɫ

    // ��̬���������ֿ鴫�䣩��ÿ������������Ҫ3�ֽ�
    uint8_t buffer[MAX_BUFFER_SIZE * 3];  
    uint32_t pixelsPerBlock = MAX_BUFFER_SIZE / 3;  // ����ÿ�������ɵ�������
    uint32_t remaining = pixelCount;

    LCD_CS_Clr();
    LCD_DC_Set();

    while (remaining > 0) {
        uint32_t currentPixels = (remaining > pixelsPerBlock) ? pixelsPerBlock : remaining;
        
        // ��䵱ǰ��
        for (uint32_t i = 0; i < currentPixels; i++) {
            buffer[i * 3] = r;      // ��ɫ����
            buffer[i * 3 + 1] = g; // ��ɫ����
            buffer[i * 3 + 2] = b; // ��ɫ����
        }


        // �ȴ�ǰһ��DMA�������
        while(dmaTransferComplete == 0) {
            // ������ӳ�ʱ����
        }
		dmaTransferComplete = 0;
        // ���͵�ǰ�飨ÿ������3�ֽڣ�
        HAL_SPI_Transmit_DMA(&hspi1, buffer, currentPixels * 3);
        remaining -= currentPixels;
    }
    while(dmaTransferComplete == 0) {
        __nop();
    }
    LCD_CS_Set();
}

/******************************************************************************
      ����˵������ָ��λ�û���
      ������ݣ�x,y ��������
                color �����ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_Address_Set(x,y,x,y);//���ù��λ�� 
	LCD_WR_DATA(color);
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
void LCD_ShowChinese12x12(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//������Ŀ
	uint16_t TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	                         
	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//ͳ�ƺ�����Ŀ
	for(k=0;k<HZnum;k++) 
	{
		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//�ǵ��ӷ�ʽ
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
					else//���ӷ�ʽ
					{
						if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
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
void LCD_ShowChinese16x16(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//������Ŀ
	uint16_t TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	uint16_t x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//ͳ�ƺ�����Ŀ
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//�ǵ��ӷ�ʽ
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
					else//���ӷ�ʽ
					{
						if(tfont16[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
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
void LCD_ShowChinese24x24(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//������Ŀ
	uint16_t TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//ͳ�ƺ�����Ŀ
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//�ǵ��ӷ�ʽ
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
					else//���ӷ�ʽ
					{
						if(tfont24[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
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
void LCD_ShowChinese32x32(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//������Ŀ
	uint16_t TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//ͳ�ƺ�����Ŀ
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//�ǵ��ӷ�ʽ
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
					else//���ӷ�ʽ
					{
						if(tfont32[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
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
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t temp,sizex,t,m=0;
	uint16_t i,TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	uint16_t x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //�õ�ƫ�ƺ��ֵ
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //���ù��λ�� 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //����6x12����
		else if(sizey==16)temp=ascii_1608[num][i];		 //����8x16����
		else if(sizey==24)temp=ascii_2412[num][i];		 //����12x24����
		else if(sizey==32)temp=ascii_3216[num][i];		 //����16x32����
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//�ǵ���ģʽ
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
			else//����ģʽ
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//��һ����
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
        // ������ʾ����Ľ�������
    uint16_t x_end = x + length;
    uint16_t y_end = y + width;
    
    // ������ʾ��Χ
    LCD_Address_Set(x, y, x_end, y_end);
    
    // ������������
    uint32_t pixelCount = length * width;
    
    // ����ͼƬ�������ֽ���
    uint32_t dataSize = pixelCount * 3;
    
    // �ֿ鴫�����
    uint32_t remaining = dataSize;
    uint32_t offset = 0;
    
    LCD_CS_Clr();
    LCD_DC_Set();
    
    // �ֿ鴫��ͼƬ����
    while (remaining > 0) {
        uint32_t chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
        
        // ���͵�ǰ���ݿ�
        HAL_SPI_Transmit(&hspi1, (uint8_t*)(pic + offset), chunkSize, HAL_MAX_DELAY);
        
        offset += chunkSize;
        remaining -= chunkSize;
    }
    
    LCD_CS_Set();
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

