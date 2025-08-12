/**
 ****************************************************************************************************
 * @file        memlcd.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.2
 * @date        2022-05-26
 * @brief       GUI代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "memlcd.h"
#include "./BSP/LCD/ltdc.h"


_mlcd_dev mlcddev;  /* mem lcd 结构体 */


/**
 * @brief       初始化mem lcd设备
 * @param       width,height:mem lcd的宽度和高度
 * @param       lx:要申请多少个层,1~MEMLCD_MAX_LAYER
 * @retval      返回值:0,成功;1,lx值非法;2,内存错误
 */
uint8_t mlcd_init(uint16_t width,uint16_t height,uint8_t lx)
{
    uint8_t i;
    uint32_t memsize = 0;
    mlcddev.width = width;
    mlcddev.height = height;
    
    memsize = (uint32_t)width * height * 2;
    if (lx > MEMLCD_MAX_LAYER || lx == 0) return 1;     /* lx非法 */
    
    for (i = 0;i < lx;i ++)
    {
        mlcddev.grambuf[i] = mymalloc(SRAMEX,memsize);  /* 申请内存 */
        
        if (mlcddev.grambuf[i] == NULL) break;
    }
    
    if(i != MEMLCD_MAX_LAYER)                           /* 申请失败了 */
    {
        mlcd_delete();
        return 2;
    }
    return 0;
}

/**
 * @brief       删除memlcd设备
 * @param       无
 * @retval      无
 */
void mlcd_delete(void)
{
    uint8_t i;
    
    for (i = 0;i < MEMLCD_MAX_LAYER;i ++)
    {
        if (mlcddev.grambuf[i])  myfree(SRAMEX,mlcddev.grambuf[i]); /* 释放内存 */
        
        mlcddev.grambuf[i] = 0;
    }
    
    mlcddev.width = 0;
    mlcddev.height = 0;
    mlcddev.layer = 0; 
} 

/**
 * @brief       设置操作层
 * @param       lx:层编号
 * @retval      无
 */
void mlcd_set_layer(uint8_t lx)
{
    if (lx < MEMLCD_MAX_LAYER) mlcddev.layer = lx;  /* 设置操作层 */
}

/**
 * @brief       在指定位置画点
 * @param       x,y:坐标
 * @param       color:颜色
 * @retval      无
 */
void mlcd_draw_point(uint16_t x,uint16_t y,uint16_t color)
{
    if (lcdltdc.pwidth&&mlcddev.dir == 0)   /* RGB竖屏 */
    {
        x = mlcddev.width - x - 1;          /* 坐标变换 */
        mlcddev.grambuf[mlcddev.layer][y + x * mlcddev.height] = color;
    }
    else mlcddev.grambuf[mlcddev.layer][x + y * mlcddev.width] = color; 
}

/**
 * @brief       读取指定位置点的颜色值
 * @param       x,y:坐标
 * @retval      返回值:颜色
 */
uint16_t mlcd_read_point(uint16_t x,uint16_t y)
{ 
    if (lcdltdc.pwidth&&mlcddev.dir == 0)   /* RGB竖屏  */
    {
        x = mlcddev.width - x - 1;          /* 坐标变换 */
        
        return mlcddev.grambuf[mlcddev.layer][y + x * mlcddev.height];
    }
    else return mlcddev.grambuf[mlcddev.layer][x + y * mlcddev.width];
} 

/**
 * @brief       清屏函数
 * @param       color:颜色值
 * @retval      无
 */
void mlcd_clear(uint16_t color)
{
    uint32_t i,len = 0;
    uint32_t *pbuf = (uint32_t*)mlcddev.grambuf[mlcddev.layer]; /* 改为32位赋值,提高速度 */
    uint32_t color32 = ((uint32_t)color << 16) | color;
    len = (uint32_t)mlcddev.width * mlcddev.height / 2;
    
    for (i = 0;i < len;i ++)
    {
        pbuf[i] = color32;
    } 
}

/**
 * @brief       在指定区域内填充单个颜色
 * @param       (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)  
 * @param       color:要填充的颜色
 * @retval      无
 */
void mlcd_fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{          
    uint16_t i,j; 
    uint16_t xend=0;  
    xend = ex + 1;
    
    if (lcdltdc.pwidth && mlcddev.dir == 0)  /* RGB竖屏 */
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
 * @brief       显示层
 * @param       lx:层编号
 * @retval      无
 */
void mlcd_display_layer(uint8_t lx)
{
    lcd_color_fill(0,0,lcddev.width - 1,lcddev.height - 1,mlcddev.grambuf[lx]);
}
