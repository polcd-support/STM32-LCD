/**
 ****************************************************************************************************
 * @file        gradienter.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       APP-水平仪 代码
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
 * 修改说明
 * V1.1 20220526
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */
 
#include "gradienter.h"


/**
 * @brief       画实心圆
 * @param       x0,y0:坐标
 * @param       len:线长度
 * @param       color:颜色
 * @retval      无
 */
void grad_draw_hline(short x0,short y0,uint16_t len,uint16_t color)
{  
    short ex;
    ex = x0 + len - 1;
    
    if(y0 < 0) return;
    
    if(y0 > (mlcddev.height - 1)) return;
    
    if(ex < 0) return;
    
    if(ex > (mlcddev.width-1)) ex = lcddev.width - 1;
    
    if(x0 > (mlcddev.width-1)) return;
    
    if(x0 < 0)x0 = 0;
    
    mlcd_fill(x0,y0,ex,y0,color);
}

/**
 * @brief       画实心圆
 * @param       x0,y0:坐标
 * @param       r:半径
 * @param       color:颜色
 * @retval      无
 */
void grad_fill_circle(short x0,short y0,uint16_t r,uint16_t color)
{
    uint32_t i;
    uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
    uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
    uint32_t x = r;
    grad_draw_hline(x0 - r,y0,2 * r,color);
    
    for (i = 1;i <= imax;i ++) 
    {
        if ((i * i + x * x) > sqmax) 
        {
            /* draw lines from outside */
            if (x > imax) 
            {
                grad_draw_hline (x0 - i + 1,y0 + x,2 * (i - 1),color);
                grad_draw_hline (x0 - i + 1,y0 - x,2 * (i - 1),color);
            }
            
            x--;
        }
        
        /* draw lines from inside (center) */
        grad_draw_hline(x0 - x,y0 + i,2 * x,color);
        grad_draw_hline(x0 - x,y0 - i,2 * x,color);
    }
}

/**
 * @brief       水平仪功能
 * @param       无
 * @retval      无
 */
uint8_t grad_play(void)
{ 
    OS_CPU_SR cpu_sr = 0;
    uint8_t res,rval = 0;   
    uint8_t t = 0;
    uint16_t angy;              /* 显示角度的纵坐标起始位置 */
    uint8_t fsize;
    uint8_t *buf;
    uint8_t len;  
    uint8_t stable;
    uint16_t bkcolor = BLACK;   /* 默认背景色为黑色 */
    
    eulerian_angles_t e_angles;
    short acc_data[3];          /* 加速度传感器原始数据 */
    short gyro_data[3];         /* 陀螺仪原始数据 */
    
    short xcenter,ycenter;      /* 中心坐标 */
    short xr1,yr1,wr1;          /* 圆1坐标和边长 */
    short xr2,yr2,wr2;          /* 圆2坐标和边长 */
    short scalefac;             /* 缩放因子 */
    short oldxr2,oldyr2;
    double angle;
    
    /* 提示开始检测ICM20608 的MPL */
    window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)gyro_remind_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
    app_es8388_volset(0);       /* 静音  */

    OS_ENTER_CRITICAL();        /* 进入临界区(无法被中断打断) */
    SCB_CleanInvalidateDCache();  /* 清空&失效cache */
    res = imu_init();               /* 初始化IMU */
    OS_EXIT_CRITICAL();             /* 退出临界区(可以被中断打断) */
    
    app_es8388_volset(es8388set.mvol);  /* 重新恢复音量 */
    
    if (res)
    {
        window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)gyro_remind_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        delay_ms(500);   
        rval = 1;
    }
    else rval = grad_load_font(); /* 加载字体 */
    
    if (lcddev.width == 240)
    {
        fsize = 72;angy = 230;
    }
    else if (lcddev.width == 272)
    {
        fsize = 72;angy = 350;
    }
    else if (lcddev.width == 320)
    {
        fsize = 88;angy = 370;
    }
    else if (lcddev.width == 480)
    {
        fsize = 144;angy = 600;
    }
    else if (lcddev.width == 600)
    {
        fsize = 144;angy = 750;
    }
    else if(lcddev.width == 800)
    {
        fsize = 144;angy = 800;
    }
    
    buf = gui_memin_malloc(32); /* 申请内存 */
    
    if (rval == 0) 
    {
        mlcd_init(lcddev.width,lcddev.height,1);    /* 初始化mlcd */
        mlcd_clear(BLACK); 
        xcenter = lcddev.width / 2;
        ycenter = lcddev.height / 2; 
        wr1 = lcddev.width/6;
        wr2 = wr1 + 2; 
        scalefac = 10;
        xr1 = xcenter - wr1 / 2;
        yr1 = ycenter - wr1 / 2;
        xr2 = xcenter - wr2 / 2;
        yr2 = ycenter - wr2 / 2;
        gui_phy.draw_point = (void(*)(uint16_t,uint16_t,uint32_t))mlcd_draw_point;
        
        system_task_return = 0;
        
        while (1)
        { 
            /* 读取传感器数据 */
            sh3001_get_imu_compdata(acc_data, gyro_data);
            
            /* 数据校准 */
            imu_data_calibration(&gyro_data[0], &gyro_data[1], &gyro_data[2],
                                 &acc_data[0],  &acc_data[1],  &acc_data[2]);

            /* 获取欧拉角 */
            e_angles = imu_get_eulerian_angles(gyro_data[0], gyro_data[1], gyro_data[2],
                                               acc_data[0],  acc_data[1],  acc_data[2]);
            
            
            if (system_task_return)
            {
                delay_ms(10);
                if(tpad_scan(1))break;  /* TPAD返回,再次确认,排除干扰 */
                else system_task_return = 0;
            }
            
            if (e_angles.pitch != NULL && e_angles.roll != NULL && e_angles.yaw != NULL)
            {
                if ((t % 10) == 0)
                {  
                    angle = sqrt(e_angles.pitch * e_angles.pitch + e_angles.roll * e_angles.roll); 
                    
                    if (angle >= 0.1)
                    {
                        if (bkcolor != BLACK)
                        {
                            bkcolor = BLACK; 
                            oldxr2 = 0;oldyr2 = 0;
                        }
                        angle =- angle;
                        stable = 0;
                    }
                    else 
                    { 
                        if (stable < 10) stable ++; /* 等待状态稳定 */
                        else
                        {
                            if (bkcolor != GREEN)
                            {
                                bkcolor = GREEN; 
                                oldxr2 = 0;oldyr2 = 0;
                            } 
                        }
                    }
                    
                    yr1 = ycenter + e_angles.pitch * scalefac;
                    yr2 = ycenter - e_angles.pitch * scalefac;
                    xr1 = xcenter + e_angles.roll * scalefac;
                    xr2 = xcenter - e_angles.roll * scalefac; 
                    
                    if (oldxr2 != xr2 || oldyr2 != yr2)
                    {  
                        mlcd_clear(bkcolor);
                        oldxr2 = xr2;oldyr2 = yr2; 
                        grad_fill_circle(xr2,yr2,wr2,RED); 
                        grad_fill_circle(xr1,yr1,wr1,BLUE); 
                        sprintf((char*)buf,"%.1f",angle);
                        len = strlen((char*)buf);
                        buf[len] = 95+' ';
                        buf[len + 1] = 0; 
                        gui_phy.back_color = bkcolor; 
                        gui_show_strmid(0,angy,lcddev.width,fsize,WHITE,fsize,buf); /* 显示新角度 */
                        mlcd_display_layer(0);
                    }
                    
                    t++;
                    continue; 
                }
            }
            
            t++;
            delay_ms(3);
        }
        
        mlcd_delete();                          /* 删除mlcd */
        gui_phy.draw_point=lcd_draw_point;      /* 恢复为原来的画点函数 */
    }
    grad_delete_font();     /* 删除字体 */
    gui_memin_free(buf);    /* 释放内存 */
    return rval;
}
