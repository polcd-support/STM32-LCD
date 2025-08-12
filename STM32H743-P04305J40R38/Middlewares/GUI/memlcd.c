/**
 ****************************************************************************************************
 * @file        memlcd.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.2
 * @date        2022-05-26
 * @brief       GUI����
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "memlcd.h"
#include "./BSP/LCD/ltdc.h"


_mlcd_dev mlcddev;  /* mem lcd �ṹ�� */


/**
 * @brief       ��ʼ��mem lcd�豸
 * @param       width,height:mem lcd�Ŀ�Ⱥ͸߶�
 * @param       lx:Ҫ������ٸ���,1~MEMLCD_MAX_LAYER
 * @retval      ����ֵ:0,�ɹ�;1,lxֵ�Ƿ�;2,�ڴ����
 */
uint8_t mlcd_init(uint16_t width,uint16_t height,uint8_t lx)
{
    uint8_t i;
    uint32_t memsize = 0;
    mlcddev.width = width;
    mlcddev.height = height;
    
    memsize = (uint32_t)width * height * 2;
    if (lx > MEMLCD_MAX_LAYER || lx == 0) return 1;     /* lx�Ƿ� */
    
    for (i = 0;i < lx;i ++)
    {
        mlcddev.grambuf[i] = mymalloc(SRAMEX,memsize);  /* �����ڴ� */
        
        if (mlcddev.grambuf[i] == NULL) break;
    }
    
    if(i != MEMLCD_MAX_LAYER)                           /* ����ʧ���� */
    {
        mlcd_delete();
        return 2;
    }
    return 0;
}

/**
 * @brief       ɾ��memlcd�豸
 * @param       ��
 * @retval      ��
 */
void mlcd_delete(void)
{
    uint8_t i;
    
    for (i = 0;i < MEMLCD_MAX_LAYER;i ++)
    {
        if (mlcddev.grambuf[i])  myfree(SRAMEX,mlcddev.grambuf[i]); /* �ͷ��ڴ� */
        
        mlcddev.grambuf[i] = 0;
    }
    
    mlcddev.width = 0;
    mlcddev.height = 0;
    mlcddev.layer = 0; 
} 

/**
 * @brief       ���ò�����
 * @param       lx:����
 * @retval      ��
 */
void mlcd_set_layer(uint8_t lx)
{
    if (lx < MEMLCD_MAX_LAYER) mlcddev.layer = lx;  /* ���ò����� */
}

/**
 * @brief       ��ָ��λ�û���
 * @param       x,y:����
 * @param       color:��ɫ
 * @retval      ��
 */
void mlcd_draw_point(uint16_t x,uint16_t y,uint16_t color)
{
    if (lcdltdc.pwidth&&mlcddev.dir == 0)   /* RGB���� */
    {
        x = mlcddev.width - x - 1;          /* ����任 */
        mlcddev.grambuf[mlcddev.layer][y + x * mlcddev.height] = color;
    }
    else mlcddev.grambuf[mlcddev.layer][x + y * mlcddev.width] = color; 
}

/**
 * @brief       ��ȡָ��λ�õ����ɫֵ
 * @param       x,y:����
 * @retval      ����ֵ:��ɫ
 */
uint16_t mlcd_read_point(uint16_t x,uint16_t y)
{ 
    if (lcdltdc.pwidth&&mlcddev.dir == 0)   /* RGB����  */
    {
        x = mlcddev.width - x - 1;          /* ����任 */
        
        return mlcddev.grambuf[mlcddev.layer][y + x * mlcddev.height];
    }
    else return mlcddev.grambuf[mlcddev.layer][x + y * mlcddev.width];
} 

/**
 * @brief       ��������
 * @param       color:��ɫֵ
 * @retval      ��
 */
void mlcd_clear(uint16_t color)
{
    uint32_t i,len = 0;
    uint32_t *pbuf = (uint32_t*)mlcddev.grambuf[mlcddev.layer]; /* ��Ϊ32λ��ֵ,����ٶ� */
    uint32_t color32 = ((uint32_t)color << 16) | color;
    len = (uint32_t)mlcddev.width * mlcddev.height / 2;
    
    for (i = 0;i < len;i ++)
    {
        pbuf[i] = color32;
    } 
}

/**
 * @brief       ��ָ����������䵥����ɫ
 * @param       (sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)  
 * @param       color:Ҫ������ɫ
 * @retval      ��
 */
void mlcd_fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{          
    uint16_t i,j; 
    uint16_t xend=0;  
    xend = ex + 1;
    
    if (lcdltdc.pwidth && mlcddev.dir == 0)  /* RGB���� */
    {
        for (i = sy;i <= ey;i++)
        {  
            for (j = sx;j < xend;j ++)
            { 
                mlcddev.grambuf[mlcddev.layer][i + (mlcddev.width - j - 1) * mlcddev.height] = color;
            }
        }
    }
    else 
    {
        for (i = sy;i <= ey;i ++)
        {  
            for (j = sx;j < xend;j ++)
            { 
                mlcddev.grambuf[mlcddev.layer][j + i * mlcddev.width] = color;
            }
        }
    }
}  

/**
 * @brief       ��ʾ��
 * @param       lx:����
 * @retval      ��
 */
void mlcd_display_layer(uint8_t lx)
{
    lcd_color_fill(0,0,lcddev.width - 1,lcddev.height - 1,mlcddev.grambuf[lx]);
}
