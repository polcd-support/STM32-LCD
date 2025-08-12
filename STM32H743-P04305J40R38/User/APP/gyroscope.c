/**
 ****************************************************************************************************
 * @file        gyroscope.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       APP-陀螺仪测试 代码
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

#include "gyroscope.h"
#include "./BSP/SH3001/sh3001.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/TPAD/tpad.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/SH3001/imu.h"
#include "math.h"
 
 
/* MPL提示 */
uint8_t*const gyro_remind_tbl[2][GUI_LANGUAGE_NUM]=
{
    {"初始化IMU,请稍侯...","初始化IMU,請稍後...","IMU Init,Please wait...",},
    {"初始化错误,请检查...","初始化錯誤,請檢查...","Init Error,Please check...",},
};

/* 姿态解算信息 */
uint8_t*const gyro_msg_tbl[3][GUI_LANGUAGE_NUM]=
{
    {"俯仰角","俯仰角","PITCH",},
    {"横滚角","橫滾角","ROLL",},
    {"航向角","航向角","YAW",},
};

/**
 * @brief       画圆形指针表盘
 * @param       x,y:坐标中心点
 * @param       size:表盘大小(直径)
 * @param       d:表盘分割,秒钟的高度 
 * @retval      无
 */
void gyro_circle_panel(uint16_t x,uint16_t y,uint16_t size,uint16_t d)
{
    uint16_t r = size / 2;              /* 得到半径 */
    uint16_t sx = x - r;
    uint16_t sy = y - r;
    uint16_t px0,px1;
    uint16_t py0,py1; 
    uint16_t i; 
    gui_fill_circle(x,y,r,WHITE);       /* 画外圈 */
    gui_fill_circle(x,y,r - 4,BLACK);   /* 画内圈 */
    
    for (i = 0;i < 60;i ++)             /* 画6°格 */
    { 
        px0 = sx + r + (r - 4) * sin((app_pi / 30) * i); 
        py0 = sy + r - (r - 4) * cos((app_pi / 30) * i); 
        px1 = sx + r + (r - d) * sin((app_pi / 30) * i); 
        py1 = sy + r - (r - d) * cos((app_pi / 30) * i);  
        gui_draw_bline1(px0,py0,px1,py1,0,WHITE);
    }
    
    for (i = 0;i < 12;i ++)             /* 画30°格 */
    { 
        px0 = sx + r + (r - 5) * sin((app_pi / 6) * i); 
        py0 = sy + r - (r - 5) * cos((app_pi / 6) * i); 
        px1 = sx + r + (r - d) * sin((app_pi / 6) * i); 
        py1 = sy + r - (r - d) * cos((app_pi / 6) * i);  
        gui_draw_bline1(px0,py0,px1,py1,2,YELLOW);
    }
    
    for (i = 0;i < 4;i ++)              /* 画90°格 */
    { 
        px0 = sx + r + (r - 5) * sin((app_pi / 2) * i); 
        py0 = sy + r - (r - 5) * cos((app_pi / 2) * i); 
        px1 = sx + r + (r - d - 3) * sin((app_pi / 2) * i); 
        py1 = sy + r - (r - d - 3) * cos((app_pi / 2) * i);  
        gui_draw_bline1(px0,py0,px1,py1,2,YELLOW);
    }
} 

/**
 * @brief       显示指针
 * @param       x,y:坐标中心点
 * @param       size:表盘大小(直径)
 * @param       d:表盘分割,秒钟的高度 
 * @param       arg:角度 -1800~1800,单位:0.1°
 * @param       color:颜色值
 * @retval      无
 */
void gyro_circle_show(uint16_t x,uint16_t y,uint16_t size,uint16_t d,s16 arg,uint16_t color)
{
    uint16_t r = size / 2;  /* 得到半径 */
    uint16_t px1,py1;   
    px1 = x + (r - 2 * d - 5)*sin((app_pi / 1800) * arg); 
    py1 = y - (r - 2 * d - 5)*cos((app_pi / 1800) * arg); 
    gui_draw_bline1(x,y,px1,py1,1,color); 
    gui_fill_circle(x + 1,y + 1,d,color);   /* 画中心圈 */
}

/**
 * @brief       显示俯仰角
 * @param       x,y:坐标中心点
 * @param       size:表盘大小(直径)
 * @param       d:表盘分割,秒钟的高度 
 * @param       parg:俯仰角 -900~900,单位:0.1°
 * @retval      无
 */
void gyro_circle_pitch(uint16_t x,uint16_t y,uint16_t size,uint16_t d,s16 parg)
{
    static s16 oldpitch = 0; 
    uint8_t *buf;
    float temp;
    
    if (oldpitch != parg)
    {
        buf = gui_memin_malloc(100);
        gyro_circle_show(x,y,size,d,oldpitch,BLACK);                        /* 先清除原来的显示 */
        temp = (float)parg / 10;
        sprintf((char*)buf,"%.1f°",temp);                                    /* 百分比 */
        gui_fill_rectangle(x - 21 + 6,y + (size / 4) - 6,42,12,BLACK);      /* 填充底色 */
        gui_show_strmid(x - 21 + 6,y + (size / 4) - 6,42,12,GREEN,12,buf);  /* 显示角度 */
        gyro_circle_show(x,y,size,d,parg,GREEN);                            /* 指向新的值 */
        oldpitch = parg;
        gui_memin_free(buf);
    }
}

/**
 * @brief       显示横滚角
 * @param       x,y:坐标中心点
 * @param       size:表盘大小(直径)
 * @param       d:表盘分割,秒钟的高度 
 * @param       parg:横滚角 -1800~1800,单位:0.1°
 * @retval      无
 */
void gyro_circle_roll(uint16_t x,uint16_t y,uint16_t size,uint16_t d,s16 rarg)
{
    static s16 oldroll = 0; 
    uint8_t *buf;
    float temp;
    
    if (oldroll != rarg)
    {
        buf = gui_memin_malloc(100);
        gyro_circle_show(x,y,size,d,oldroll,BLACK);                         /* 先清除原来的显示 */
        temp = (float)rarg / 10;
        sprintf((char*)buf,"%.1f°",temp);                                   /* 百分比 */
        gui_fill_rectangle(x - 24 + 6,y + (size / 4) - 6,48,12,BLACK);      /* 填充底色 */
        gui_show_strmid(x - 24 + 6,y + (size / 4) - 6,48,12,RED,12,buf);    /* 显示角度 */
        gyro_circle_show(x,y,size,d,rarg,RED);                              /* 指向新的值 */
        oldroll = rarg;
        gui_memin_free(buf);
    }
}

/**
 * @brief       6轴陀螺仪MPU9250演示
 * @param       x,y:坐标中心点
 * @param       size:表盘大小(直径)
 * @param       d:表盘分割,秒钟的高度 
 * @param       parg:航向角 -1800~1800,单位:0.1°
 * @retval      无
 */
void gyro_circle_yaw(uint16_t x,uint16_t y,uint16_t size,uint16_t d,s16 yarg)
{
    static s16 oldyaw = 0; 
    uint8_t *buf;
    float temp;
    
    if (oldyaw != yarg)
    {
        buf = gui_memin_malloc(100);
        gyro_circle_show(x,y,size,d,oldyaw,BLACK);                              /* 先清除原来的显示 */
        temp = (float)yarg / 10;
        sprintf((char*)buf,"%.1f°",temp);                                       /* 百分比 */
        gui_fill_rectangle(x - 24 + 6,y + (size/4) - 6,48,12,BLACK);            /* 填充底色 */
        gui_show_strmid(x - 24 + 6,y + (size / 4) - 6,48,12,YELLOW,12,buf);     /* 显示角度 */
        gyro_circle_show(x,y,size,d,yarg,YELLOW);                               /* 指向新的值 */
        oldyaw = yarg;
        gui_memin_free(buf);
    }
}

/**
 * @brief       6轴陀螺仪MPU9250演示
 * @param       无
 * @retval      无
 */
uint8_t gyro_play(void)
{    
    OS_CPU_SR cpu_sr = 0;
    eulerian_angles_t e_angles;
    short acc_data[3];                          /* 加速度传感器原始数据 */
    short gyro_data[3];                         /* 陀螺仪原始数据 */
    float pitch,roll,yaw; 
    short temp;
    uint8_t rval=0;
    uint16_t rpr,ry;
    uint8_t dpr,dy;
    uint16_t xp;
    uint16_t ypr;
    uint8_t fsize; 
    uint8_t res = 0;
    
    /* 提示开始检测ICM20608 的IMU */
    window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)gyro_remind_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
    app_es8388_volset(0);               /* 静音  */
    OS_ENTER_CRITICAL();                /* 进入临界区(无法被中断打断)  */
    SCB_CleanInvalidateDCache();        /* 清空&失效cache */
    res = imu_init();                   /* 初始化IMU */
    OS_EXIT_CRITICAL();                 /* 退出临界区(可以被中断打断) */
    
    app_es8388_volset(es8388set.mvol);  /* 重新恢复音量 */
    
    if (res)
    {
        window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)gyro_remind_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        delay_ms(500);  
        rval = 1;
    }
    else   /* 初始化OK */
    {
        lcd_clear(BLACK);
        app_gui_tcbar(0,0,lcddev.width,gui_phy.tbheight,0x02);  /* 下分界线 */
        gui_show_strmid(0,0,lcddev.width,gui_phy.tbheight,WHITE,gui_phy.tbfsize,(uint8_t*)APP_MFUNS_CAPTION_TBL[19][gui_phy.language]); /* 显示标题 */
        
        if (lcddev.width == 240)
        {
            rpr = 59;ry = 74;
            dpr = 7;dy = 8; 
            fsize = 12;
        }
        else if (lcddev.width == 272)
        {
            rpr = 66;ry = 100;
            dpr = 7;dy = 9; 
            fsize = 12;
        }
        else if (lcddev.width == 320)
        { 
            rpr = 79;ry = 140;
            dpr = 8;dy = 10; 
            fsize = 12;
        }
        else if (lcddev.width == 480)
        {
            rpr = 110;ry = 200;
            dpr = 10;dy = 12; 
            fsize = 16;
        }
        else if (lcddev.width == 600)
        {
            rpr = 135;ry = 250;
            dpr = 12;dy = 12; 
            fsize = 24;
        }
        else if(lcddev.width == 800)
        {
            rpr = 135;ry = 350;
            dpr = 15;dy = 15; 
            fsize = 32;
        }
        
        xp = lcddev.width / 4;
        ypr = xp + gui_phy.tbheight; 
        gyro_circle_panel(xp,ypr,2 * rpr,dpr);                              /* 画俯仰角表盘 */
        gui_show_strmid(xp - (3 * fsize) / 2,ypr + rpr+fsize / 2,3 * fsize,fsize,GREEN,fsize,(uint8_t*)gyro_msg_tbl[0][gui_phy.language]);/* 显示名字 */
        gyro_circle_panel(xp+lcddev.width/2,ypr,2*rpr,dpr);                 /* 画横滚角表盘 */
        gui_show_strmid(lcddev.width / 2 + xp - (3 * fsize) / 2,ypr + rpr + fsize / 2,3*fsize,fsize,RED,fsize,(uint8_t*)gyro_msg_tbl[1][gui_phy.language]);/* 显示名字 */
        gyro_circle_panel(lcddev.width/2,ypr+rpr+fsize+ry,2*ry,dy);         /* 画航向角表盘 */
        gui_show_strmid(lcddev.width / 2 - (3 * fsize) / 2,ypr + rpr + fsize + ry * 2 + fsize / 2,3 * fsize,fsize,YELLOW,fsize,(uint8_t*)gyro_msg_tbl[2][gui_phy.language]);/* 显示名字 */
        gyro_circle_pitch(xp,ypr,2 * rpr,dpr,360);                          /* 让指针显示出来 */
        gyro_circle_roll(xp+lcddev.width / 2,ypr,2 * rpr,dpr,360);          /* 让指针显示出来 */
        gyro_circle_yaw(lcddev.width / 2,ypr + rpr + fsize + ry,2 * ry,dy,360);   /* 让指针显示出来 */
        
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
                if(tpad_scan(1)) break;  /* TPAD返回,再次确认,排除干扰 */
                else system_task_return = 0;
            } 
            
            if (e_angles.pitch != NULL && e_angles.roll != NULL && e_angles.yaw != NULL)       /* 读取IMU数据 */
            {  
                temp = e_angles.pitch * 10;
                gyro_circle_pitch(xp,ypr,2 * rpr,dpr,temp);
                temp = e_angles.roll * 10;
                gyro_circle_roll(xp + lcddev.width / 2,ypr,2 * rpr,dpr,temp);
                temp = e_angles.yaw * 10;
                gyro_circle_yaw(lcddev.width / 2,ypr + rpr + fsize + ry,2 * ry,dy,temp);
            }
            
            delay_ms(1);  
        }
    }
    
    return rval;
}
