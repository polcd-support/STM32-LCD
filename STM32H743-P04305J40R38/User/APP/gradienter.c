/**
 ****************************************************************************************************
 * @file        gradienter.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       APP-ˮƽ�� ����
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
 * �޸�˵��
 * V1.1 20220526
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */
 
#include "gradienter.h"


/**
 * @brief       ��ʵ��Բ
 * @param       x0,y0:����
 * @param       len:�߳���
 * @param       color:��ɫ
 * @retval      ��
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
 * @brief       ��ʵ��Բ
 * @param       x0,y0:����
 * @param       r:�뾶
 * @param       color:��ɫ
 * @retval      ��
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
 * @brief       ˮƽ�ǹ���
 * @param       ��
 * @retval      ��
 */
uint8_t grad_play(void)
{ 
    OS_CPU_SR cpu_sr = 0;
    uint8_t res,rval = 0;   
    uint8_t t = 0;
    uint16_t angy;              /* ��ʾ�Ƕȵ���������ʼλ�� */
    uint8_t fsize;
    uint8_t *buf;
    uint8_t len;  
    uint8_t stable;
    uint16_t bkcolor = BLACK;   /* Ĭ�ϱ���ɫΪ��ɫ */
    
    eulerian_angles_t e_angles;
    short acc_data[3];          /* ���ٶȴ�����ԭʼ���� */
    short gyro_data[3];         /* ������ԭʼ���� */
    
    short xcenter,ycenter;      /* �������� */
    short xr1,yr1,wr1;          /* Բ1����ͱ߳� */
    short xr2,yr2,wr2;          /* Բ2����ͱ߳� */
    short scalefac;             /* �������� */
    short oldxr2,oldyr2;
    double angle;
    
    /* ��ʾ��ʼ���ICM20608 ��MPL */
    window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)gyro_remind_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
    app_es8388_volset(0);       /* ����  */

    OS_ENTER_CRITICAL();        /* �����ٽ���(�޷����жϴ��) */
    SCB_CleanInvalidateDCache();  /* ���&ʧЧcache */
    res = imu_init();               /* ��ʼ��IMU */
    OS_EXIT_CRITICAL();             /* �˳��ٽ���(���Ա��жϴ��) */
    
    app_es8388_volset(es8388set.mvol);  /* ���»ָ����� */
    
    if (res)
    {
        window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)gyro_remind_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        delay_ms(500);   
        rval = 1;
    }
    else rval = grad_load_font(); /* �������� */
    
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
    
    buf = gui_memin_malloc(32); /* �����ڴ� */
    
    if (rval == 0) 
    {
        mlcd_init(lcddev.width,lcddev.height,1);    /* ��ʼ��mlcd */
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
            /* ��ȡ���������� */
            sh3001_get_imu_compdata(acc_data, gyro_data);
            
            /* ����У׼ */
            imu_data_calibration(&gyro_data[0], &gyro_data[1], &gyro_data[2],
                                 &acc_data[0],  &acc_data[1],  &acc_data[2]);

            /* ��ȡŷ���� */
            e_angles = imu_get_eulerian_angles(gyro_data[0], gyro_data[1], gyro_data[2],
                                               acc_data[0],  acc_data[1],  acc_data[2]);
            
            
            if (system_task_return)
            {
                delay_ms(10);
                if(tpad_scan(1))break;  /* TPAD����,�ٴ�ȷ��,�ų����� */
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
                        if (stable < 10) stable ++; /* �ȴ�״̬�ȶ� */
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
                        gui_show_strmid(0,angy,lcddev.width,fsize,WHITE,fsize,buf); /* ��ʾ�½Ƕ� */
                        mlcd_display_layer(0);
                    }
                    
                    t++;
                    continue; 
                }
            }
            
            t++;
            delay_ms(3);
        }
        
        mlcd_delete();                          /* ɾ��mlcd */
        gui_phy.draw_point=lcd_draw_point;      /* �ָ�Ϊԭ���Ļ��㺯�� */
    }
    grad_delete_font();     /* ɾ������ */
    gui_memin_free(buf);    /* �ͷ��ڴ� */
    return rval;
}
