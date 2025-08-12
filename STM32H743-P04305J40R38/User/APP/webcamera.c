/**
 ****************************************************************************************************
 * @file        facereco.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       APP-网络摄像头代码
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

#include "webcamera.h"
#include "camera.h" 
#include "audioplay.h"
#include "./T9INPUT/t9input.h"
#include "./BSP/OV5640/ov5640.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/PCF8574/pcf8574.h"


/* webcam 按钮标题 */
uint8_t*const webcam_btncaption_tbl[4][GUI_LANGUAGE_NUM]=
{ 
    {"开启","開啟","ON",},
    {"关闭","關閉","OFF",}, 
    {"分辨率","分辨率","PIXEL",},
    {"对焦","對焦","FOCUS",}, 
};

/* webcam状态提示信息 */
uint8_t*const webcam_stamsg_tbl[3][GUI_LANGUAGE_NUM]=
{
    {"分辨率:","分辨率:"," Pixel:",},
    {"速  度:","速  度:"," Speed",},
    {"帧  率:","幀  率:"," Frame:",}, 
};
extern uint8_t*const camera_remind_tbl[4][GUI_LANGUAGE_NUM];

/* WEBCAM JPEG尺寸支持列表 */
const uint16_t webcam_jpeg_img_size_tbl[][2]=
{ 
    320,240,    /* QVGA  */
    640,480,    /* VGA */
    800,600,    /* SVGA */
    960,640,
    1024,768,   /* XGA */
    1280,800,   /* WXGA */
};

/* WEB CAMERA SEND任务 */
/* 设置任务优先级 */
#define WEBCAM_SEND_TASK_PRIO           2 
/* 设置任务堆栈大小 */
#define WEBCAM_SEND_STK_SIZE            1000
/* 任务堆栈，采用内存管理的方式控制申请 */
OS_STK * WEBCAM_SEND_TASK_STK;
/* 任务函数 */
void webcam_send_task(void *pdata); 

volatile uint8_t webcam_sendtask_status = 0;    /* WEB摄像头发送任务状态 */
                                                    /* 0,未运行/任务已经被删除; */
                                                    /* 1,运行,数据正常发送 */
                                                    /* 2,运行,数据暂停发送 */
                                                    /* 0xff,请求删除任务 */

extern uint8_t g_ov_frame;                          /* 帧率 */
extern uint8_t*const netplay_remindmsg_tbl[5][GUI_LANGUAGE_NUM];

struct netconn *webcamnet = 0;                      /* WEB CAM TCP网络连接结构体指针 */
uint32_t *webcam_line_buf0;                         /* 定义行缓存0 */
uint32_t *webcam_line_buf1;                         /* 定义行缓存1 */

uint32_t webcam_sendnum;                            /* 发送数据量(字节) */
volatile uint8_t webcam_oensec = 0;                   /* 秒钟标志 */

//WEB CAM FIFO */
volatile uint16_t webcamfifordpos=0;                /* FIFO读位置 */
volatile uint16_t webcamfifowrpos=0;                /* FIFO写位置 */
uint32_t *webcamfifobuf[WEBCAM_FIFO_NUM];           /* 定义WEBCAM_FIFO_SIZE个接收FIFO */
 

/**
 * @brief       读取FIFO
 * @param       buf:数据缓存区首地址
 * @param       fsize: 字体大小
 * @retval      返回值:0,没有数据可读;1,读到了1个数据块
 */
uint8_t webcam_fifo_read(uint32_t **buf)
{
    if (webcamfifordpos == webcamfifowrpos)
    {
        return 0;
    }
    
    webcamfifordpos++;          /* 读位置加1 */
    
    if (webcamfifordpos >= WEBCAM_FIFO_NUM)
    {
        webcamfifordpos = 0;    /* 归零 */
    }
    
    *buf = webcamfifobuf[webcamfifordpos];
    
    return 1;
}

/**
 * @brief       写一个FIFO
 * @param       buf:数据缓存区首地址
 * @param       fsize: 字体大小
 * @retval      返回值:0,写入成功;1,写入失败
 */
uint8_t webcam_fifo_write(uint32_t *buf)
{
    uint16_t i;
    uint16_t temp = webcamfifowrpos;                /* 记录当前写位置 */
    webcamfifowrpos ++;                             /* 写位置加1 */
    
    if (webcamfifowrpos >= WEBCAM_FIFO_NUM)
    {
        webcamfifowrpos = 0;                        /* 归零 */
    }
    
    if(webcamfifordpos == webcamfifowrpos)
    {
        webcamfifowrpos = temp;                     /* 还原原来的写位置,此次写入失败 */
        //printf("shit:%d\r\n",temp);
        return 1;
    }
    
    for (i = 0;i < WEBCAM_LINE_SIZE;i++)
    {
        webcamfifobuf[webcamfifowrpos][i] = buf[i]; /* 拷贝数据 */
    }
    
    return 0;
}   

/**
 * @brief       摄像头jpeg数据接收回调函数
 * @param       无
 * @retval      无
 */
void webcam_dcmi_rx_callback(void)
{    
    if (DMA1_Stream1->CR & (1 << 19))
    {
        webcam_fifo_write(webcam_line_buf0);    /* webcam_line_buf0写入FIFO */
    }
    else
    {
        webcam_fifo_write(webcam_line_buf1);    /* webcam_line_buf1写入FIFO */
    }
}

/**
 * @brief       创建webcam发送任务
 * @param       无
 * @retval      返回值:0,成功；其他,错误代码
 */
uint8_t webcam_send_task_creat(void)
{
    OS_CPU_SR cpu_sr = 0;
    uint8_t res;
    WEBCAM_SEND_TASK_STK = gui_memin_malloc(WEBCAM_SEND_STK_SIZE * sizeof(OS_STK));
    
    if (WEBCAM_SEND_TASK_STK == 0)
    {
        return 1;               /* 内存申请失败 */
    }
    
    webcam_sendtask_status = 2;
    
    OS_ENTER_CRITICAL();        /* 进入临界区(无法被中断打断) */
    res = OSTaskCreate(webcam_send_task,(void *)0,(OS_STK*)&WEBCAM_SEND_TASK_STK[WEBCAM_SEND_STK_SIZE - 1],WEBCAM_SEND_TASK_PRIO);
    OS_EXIT_CRITICAL();         /* 退出临界区(可以被中断打断) */
    return res;
}

/**
 * @brief       删除webcam发送任务
 * @param       无
 * @retval      无
 */
void webcam_send_task_delete(void)
{ 
    if (webcam_sendtask_status)
    {
        webcam_sendtask_status = 0XFF;          /* 请求删除任务 */
        while(webcam_sendtask_status != 0);     /* 等待任务删除成功 */
    }
    else
    {
        return ;
    }
    
    gui_memin_free(WEBCAM_SEND_TASK_STK);       /* 释放内存 */
}

/**
 * @brief       webcam任务开始发送
 * @param       无
 * @retval      无
 */
void webcam_send_task_start(void)
{
    webcam_sendtask_status = 1;
}

/**
 * @brief       webcam任务停止发送
 * @param       无
 * @retval      无
 */
void webcam_send_task_stop(void)
{
    webcam_sendtask_status = 2;
}

/**
 * @brief       网络摄像头发送任务,通过其他程序创建
 * @param       无
 * @retval      无
 */
void webcam_send_task(void *pdata)
{ 
    uint8_t res = 0;
    uint32_t *tbuf;
    err_t err;                              /* 错误标志 */
    
    while (webcam_sendtask_status)
    {
        if (webcam_sendtask_status == 1)    /* 继续发送数据 */
        {
            res = webcam_fifo_read(&tbuf);
        }
        if (res)
        {
            err = netconn_write(webcamnet,tbuf,WEBCAM_LINE_SIZE * 4,NETCONN_COPY);  /* 发送数据 */
            
            if (err != ERR_OK)              /* 发送成功 */
            {
                if (err == ERR_ABRT || err == ERR_RST || err == ERR_CLSD)
                {
                    webcam_send_task_stop();
                }
            }
            else
            {
                webcam_sendnum += WEBCAM_LINE_SIZE * 4; /* 发送数据累加 */
            }
            
            res = 0;
        }
        else
        { 
            if (webcam_sendtask_status == 0XFF)
            {
                break;  /* 需要删除任务了 */
            }
            
            delay_ms(1000/OS_TICKS_PER_SEC);    /* 延时一个时钟节拍 */
        }
    }  	
    webcam_sendtask_status = 0;                 /* 任务删除完成 */
    OSTaskDel(WEBCAM_SEND_TASK_PRIO);           /* 删除音乐播放任务 */
} 

/**
 * @brief       WEBCAM显示信息
 * @param       x：X坐标
 * @param       y：Y坐标
 * @param       fsize：数据大小
 * @param       msg：数据
 * @retval      无
 */
void webcam_msg_show(uint16_t x,uint16_t y,uint16_t fsize,uint8_t* msg)
{
    gui_fill_rectangle(x,y,fsize * 9 / 2,fsize,NET_MEMO_BACK_COLOR);    /* 清除原来的显示,最多9个字符宽度 */
    gui_show_string(msg,x,y,fsize * 9 / 2,fsize,fsize,WHITE);
}

/**
 * @brief       运行网络摄像头功能
 * @param       无
 * @retval      无
 */
void webcam_run(void)
{
    uint16_t t; 
    uint8_t res; 
    uint8_t rval = 0; 
    uint8_t jpeg_size = 3;
    
    uint16_t h1,h2;             /* 纵向间隔 */
    uint16_t wh1,wh2,w2;        /* 横向间隔 */
    uint16_t t9height = 0;      /* T9输入法高度 */
    uint16_t btnw,btnh;         /* 按钮宽度/高度 */
    uint8_t pbtn;               /* 按钮间间距 */
    
    uint8_t cbtnfsize = 0;      /* 按钮字体大小 */
    uint16_t tempy;
    uint16_t tport = 8088;      /* 临时端口号,(要连接的端口号)默认为8088; */
    
    uint8_t tcpconn = 0;        /* TCP连接状态:0,没有连接;1,有连接(有Client连接); */
    uint8_t tcpstatus = 0;      /* TCP Server状态,0,未开启;1,开启 */
    
    _edit_obj* eport = 0;       /* 端口编辑框 */
    _btn_obj* pixbtn = 0;       /* 切换分辨率按钮 */
    _btn_obj* focbtn = 0;       /* 对焦按钮 */
    _btn_obj* conbtn = 0;       /* 开启/关闭按钮 */
    _t9_obj * t9 = 0;           /* 输入法 */
    uint8_t *buf;
    
    struct netconn *netconnnew = 0; /* 新TCP网络连接结构体指针 */
    err_t err;                      /* 错误标志 */
    
     
    if (lcddev.width == 240)
    {
        h1 = 10;h2 = 10;
        wh1 = 4;wh2 = 4;w2 = 20;
        btnw = 60;pbtn = 5;
        cbtnfsize = 16;t9height = 134;  
    }
    else if (lcddev.width == 272)
    { 
        h1 = 20;h2 = 10;
        wh1 = 8;wh2 = 4;w2 = 25;
        btnw = 70;pbtn = 5;
        cbtnfsize = 16;t9height = 176;
    }
    else if (lcddev.width == 320)
    { 
        h1 = 20;h2 = 10;
        wh1 = 16;wh2 = 8;w2 = 35;
        btnw = 80;pbtn = 5;	 
        cbtnfsize = 16;t9height = 176;
    }
    else if (lcddev.width == 480)
    {
        h1 = 40;h2 = 20;
        wh1 = 20;wh2 = 15;w2 = 60;
        btnw = 120;pbtn = 8;
        cbtnfsize = 24;t9height = 266;
    } 
    else if (lcddev.width == 600)
    {
        h1 = 60;h2 = 30;
        wh1 = 30;wh2 = 20;w2 = 80;
        btnw = 150;pbtn = 10;
        cbtnfsize = 24;t9height = 368;
    }
    else if(lcddev.width == 800)
    {
        h1 = 80;h2 = 40;
        wh1 = 40;wh2 = 30;w2 = 100;
        btnw = 200;pbtn = 15;
        cbtnfsize = 24;t9height = 488;
    }
    
    btnh = (7 * gui_phy.tbfsize / 2 + 2 * h2 - 2 * pbtn) / 3;
    
    webcam_line_buf0 = gui_memin_malloc(WEBCAM_LINE_SIZE * 4);
    webcam_line_buf1 = gui_memin_malloc(WEBCAM_LINE_SIZE * 4);
    
    for(t = 0;t < WEBCAM_FIFO_NUM;t ++)
    {
        webcamfifobuf[t] = gui_memex_malloc(WEBCAM_LINE_SIZE * 4);
    }
    
    buf = gui_memin_malloc(100);
    lcd_clear(NET_MEMO_BACK_COLOR);
    app_filebrower((uint8_t*)APP_MFUNS_CAPTION_TBL[17][gui_phy.language],0X05); /* 显示标题 */
    g_point_color = WHITE;
    g_back_color = NET_MEMO_BACK_COLOR;

    gui_draw_rectangle(wh1,gui_phy.tbheight + h1 + gui_phy.tbfsize / 2,lcddev.width - 2 * wh1,3 * gui_phy.tbfsize / 2 + 2 * h2,WHITE);
    lcd_show_string(wh1 + wh2,gui_phy.tbheight + h1,200,32,gui_phy.tbfsize,(uint8_t*)"TCP Server",g_back_color);
    gui_draw_rectangle(wh1,gui_phy.tbheight + h1 * 2 + h2 * 2 + 5*gui_phy.tbfsize / 2,2 * wh2 + 8 * gui_phy.tbfsize,7 * gui_phy.tbfsize / 2 + 2 * h2,WHITE);
    lcd_show_string(wh1 + wh2,gui_phy.tbheight + h1 * 2 + h2 * 2 + 2 * gui_phy.tbfsize,200,32,gui_phy.tbfsize,(uint8_t*)"Status",g_back_color);

    sprintf((char*)buf,"IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);/* 显示IP地址 */
    gui_show_string(buf,wh1 + wh2,gui_phy.tbheight + h1 + gui_phy.tbfsize+h2,lcddev.width,gui_phy.tbfsize,gui_phy.tbfsize,WHITE);
    gui_show_string((uint8_t*)"PORT:",lcddev.width - wh1 - wh2 - 9 * gui_phy.tbfsize / 2 - 2,gui_phy.tbheight + h1 + gui_phy.tbfsize+h2,200,32,gui_phy.tbfsize,WHITE);

    tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 3 * gui_phy.tbfsize - h2 / 2;
    gui_show_string((uint8_t*)webcam_stamsg_tbl[0][gui_phy.language],wh1 + wh2,tempy,200,32,gui_phy.tbfsize,WHITE);
    gui_show_string((uint8_t*)webcam_stamsg_tbl[1][gui_phy.language],wh1 + wh2,tempy + gui_phy.tbfsize + h2 / 2,200,32,gui_phy.tbfsize,WHITE);
    gui_show_string((uint8_t*)webcam_stamsg_tbl[2][gui_phy.language],wh1 + wh2,tempy + gui_phy.tbfsize * 2 + h2,200,32,gui_phy.tbfsize,WHITE);
    
    sprintf((char*)buf,"%dX%d",webcam_jpeg_img_size_tbl[jpeg_size][0],webcam_jpeg_img_size_tbl[jpeg_size][1]);
    tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 3 * gui_phy.tbfsize - h2 / 2;
    webcam_msg_show(wh1 + wh2 + gui_phy.tbfsize * 7 / 2,tempy,gui_phy.tbfsize,buf);
    
    eport = edit_creat(lcddev.width - wh1 - wh2 - 4 * gui_phy.tbfsize / 2 - 2 - 3,gui_phy.tbheight + h1 + gui_phy.tbfsize + h2 - 3,4 * gui_phy.tbfsize / 2 + 6,gui_phy.tbfsize + 6,0,4,gui_phy.tbfsize);/* 创建eport编辑框 */
    t9 = t9_creat((lcddev.width % 5) / 2,lcddev.height - t9height,lcddev.width - (lcddev.width % 5 ),t9height,0); 
    tempy = gui_phy.tbheight + h1 * 2 + h2 * 2 + 2 * gui_phy.tbfsize + gui_phy.tbfsize / 2;
    
    pixbtn=btn_creat(lcddev.width - btnw - w2,tempy,btnw,btnh,0,0);
    focbtn=btn_creat(lcddev.width - btnw - w2,tempy + pbtn + btnh,btnw,btnh,0,0);
    conbtn=btn_creat(lcddev.width - btnw - w2,tempy + pbtn * 2 + btnh * 2,btnw,btnh,0,0);

    if (!webcamfifobuf[WEBCAM_FIFO_NUM-1] || !webcam_line_buf1 || !t9 || !conbtn || !eport)
    {
        rval = 1;/* 内存申请失败 */
    }
    if (rval == 0)
    {
        eport->textbkcolor = NET_MEMO_BACK_COLOR;
        eport->textcolor = BLUE;                /* BLUE,表示可以编辑  */
        eport->type = 0X06;                     /* eport光标闪烁  */
        eport->cursorpos = 4;                   /* 光标位置在最后(8088,四个字符) */
        pixbtn->caption = webcam_btncaption_tbl[2][gui_phy.language];
        pixbtn->font = cbtnfsize;
        focbtn->caption = webcam_btncaption_tbl[3][gui_phy.language];
        focbtn->font = cbtnfsize; 
        conbtn->caption = webcam_btncaption_tbl[0][gui_phy.language];
        conbtn->font = cbtnfsize;

        sprintf((char*)buf,"%d",tport);
        strcpy((char*)eport->text,(const char *)buf);   /* 拷贝端口号 */
        t9_draw(t9);                                    /* 画T9输入法 */
        btn_draw(pixbtn);                               /* 画按钮 */
        btn_draw(focbtn);                               /* 画按钮 */
        btn_draw(conbtn);                               /* 画按钮 */
        edit_draw(eport);                               /* 画编辑框 */
        
        dcmi_init();                                    /* DCMI配置 */
        dcmi_rx_callback = webcam_dcmi_rx_callback;     /* 接收数据回调函数 */
        dcmi_dma_init((uint32_t)webcam_line_buf0,(uint32_t)webcam_line_buf1,WEBCAM_LINE_SIZE,2,1);  /* DCMI DMA配置 */
        ov5640_jpeg_mode();                             /* JPEG模式 */
        //改为15帧 */
        ov5640_write_reg(0X3035,0X11);
        ov5640_write_reg(0x3824,0X1F);
        ov5640_image_window_set(0,0,1280,800);
        ov5640_outsize_set(16,4,webcam_jpeg_img_size_tbl[jpeg_size][0],webcam_jpeg_img_size_tbl[jpeg_size][1]); /* 设置输出尺寸(640*480) */
        dcmi_start();                                   /* 启动传输 */
        tim6_int_init(10000-1,10800-1);                  /* 10Khz计数频率,1秒钟中断,用于帧率和发送速度统计 */
        delay_ms(100);
        rval = webcam_send_task_creat();                /* 创建发送任务 */
        while (rval == 0)
        {   
            tp_dev.scan(0);    
            in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);      /* 得到按键键值 */
            edit_check(eport,&in_obj);
            t9_check(t9,&in_obj);
            if(t9->outstr[0] != NULL && tcpstatus == 0)       /* TCP未开启的情况下,可以添加字符 */
            { 
                if((t9->outstr[0] <= '9' && t9->outstr[0] >= '0') || t9->outstr[0] == 0X08)
                {
                    edit_add_text(eport,t9->outstr);
                }
                
                t9->outstr[0] = NULL;   /* 清空输出字符 */
            }
            
            res = btn_check(focbtn,&in_obj);
            
            if (res && ((focbtn->sta & (1 << 7)) == 0) && (focbtn->sta & (1 << 6))) /* 有输入,有按键按下且松开,并且TP松开了 */
            {   
                ov5640_focus_single();  /* 自动对焦 */
            }
            
            res = btn_check(pixbtn,&in_obj);
            
            if (res && ((pixbtn->sta & (1 << 7)) == 0) && (pixbtn->sta & (1 << 6))) /* 有输入,有按键按下且松开,并且TP松开了 */
            {   
                jpeg_size++;
                
                if(jpeg_size > 5)jpeg_size = 0;
                if(jpeg_size > 3)
                {
                    /* 改为7.5帧 */
                    ov5640_write_reg(0X3035,0X21);
                    ov5640_write_reg(0x3824,0X0F); 
                }
                else
                {
                    /* 改为15帧 */
                    ov5640_write_reg(0X3035,0X11);
                    ov5640_write_reg(0x3824,0X1F); 	
                }
                
                ov5640_outsize_set(16,4,webcam_jpeg_img_size_tbl[jpeg_size][0],webcam_jpeg_img_size_tbl[jpeg_size][1]); /* 设置输出尺寸 */
                sprintf((char*)buf,"%dX%d",webcam_jpeg_img_size_tbl[jpeg_size][0],webcam_jpeg_img_size_tbl[jpeg_size][1]);
                tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 3 * gui_phy.tbfsize - h2 / 2;
                webcam_msg_show(wh1 + wh2 + gui_phy.tbfsize * 7 / 2,tempy,gui_phy.tbfsize,buf);
            }
            res = btn_check(conbtn,&in_obj);
            
            if (res && ((conbtn->sta & (1 << 7)) == 0) && (conbtn->sta & (1 << 6))) /* 有输入,有按键按下且松开,并且TP松开了 */
            {   
                tcpstatus = !tcpstatus;
                tcpconn = 0;                    /* 标记TCP连接未建立(没有Client连上) */
                
                if (tcpstatus == 1)            /* 建立连接 */
                { 
                    tport = net_get_port(eport->text);                  /* 得到port号 */
                    netconnnew = netconn_new(NETCONN_TCP);              /* 创建一个TCP链接 */
                    netconnnew->recv_timeout=10;                        /* 禁止阻塞线程 */
                    err = netconn_bind(netconnnew,IP_ADDR_ANY,tport);   /* 绑定端口 */
                    
                    if (err == ERR_OK)
                    {
                        err = netconn_listen(netconnnew);               /* 进入监听模式 */
                        if (err != ERR_OK) tcpstatus = 0;               /* 连接失败  */
                    }
                    else
                    {
                        tcpstatus = 0;                                  /* 连接失败  */
                    }
                }
                if (tcpstatus == 0)/* TCP服务未开启/关闭 */
                {
                    webcam_send_task_stop();                            /* webcam发送任务停止发送 */
                    eport->type = 0X06;                                 /* eport光标闪烁   */
                    eport->textcolor = BLUE;                            /* 绿色,表示可编辑 */
                    edit_draw(eport);                                   /* 画编辑框  */
                    net_disconnect(netconnnew,webcamnet);               /* 断开连接 */
                    netconnnew = NULL; 
                    webcamnet = NULL; 
                    net_tcpserver_remove_timewait();                    /* TCP Server,删除等待状态 */
                }
                else
                {
                    edit_show_cursor(eport,0);                          /* 关闭eport的光标 */
                    eport->type = 0X04;                                 /* eip光标不闪烁  */
                    eport->textcolor = WHITE;                           /* 白色,表示不可编辑 */
                }
                edit_draw(eport);                                       /* 重画编辑框  */
                conbtn->caption = webcam_btncaption_tbl[tcpstatus][gui_phy.language]; 
                btn_draw(conbtn);                                       /* 重画按钮 */
            }
            
            if (tcpconn == 0 && tcpstatus == 1)                         /* 开启了TCP服务,且TCP Client未连接. */
            {
                err = netconn_accept(netconnnew,&webcamnet);            /* 接收连接请求 */
                
                if (err == ERR_OK)                                      /* 成功监测到连接 */
                { 
                    webcamnet->recv_timeout = 10; 
                    tcpconn = 1;                                        /* TCP Client连接OK */
                    webcam_send_task_start();                           /* webcam发送任务开始发送 */
                }
            }
            else if (webcam_sendtask_status == 2)                       /* 正常连接发送数据的时候,突然断开了,则说明client主动断开了,尝试重新连接 */
            {
                net_disconnect(webcamnet,NULL);                         /* 断开连接 */
                tcpconn = 0;
            }
            if (system_task_return)
            {
                delay_ms(10);
                if(tpad_scan(1))break;                                  /* TPAD返回,再次确认,排除干扰 */
                else system_task_return = 0;
            }
            if (webcam_oensec)                                          /* 1秒时间到 */
            {
                webcam_oensec = 0;
                sprintf((char*)buf,"%dKB/S",webcam_sendnum/1024);
                webcam_sendnum = 0;
                tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 4 * gui_phy.tbfsize;
                webcam_msg_show(wh1 + wh2 + gui_phy.tbfsize * 7 / 2,tempy,gui_phy.tbfsize,buf); /* 显示网络传输速率 */
                sprintf((char*)buf,"%dFPS",g_ov_frame);
                g_ov_frame = 0;
                tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 5 * gui_phy.tbfsize + h2 / 2;
                webcam_msg_show(wh1 + wh2 + gui_phy.tbfsize * 7 / 2,tempy,gui_phy.tbfsize,buf); /* 显示帧率 */
            }
            else delay_ms(5);
        }
        
        webcam_send_task_stop();                /* webcam发送任务停止发送 */
        TIM6->CR1 &= ~(1 << 0);                 /* 关闭定时器6  */
        dcmi_stop();                            /* 停止摄像头工作 */
        webcam_send_task_delete();              /* 删除webcam发送任务 */
        net_disconnect(netconnnew,webcamnet);   /* 断开连接   */
        net_tcpserver_remove_timewait();        /* TCP Server,删除等待状态  */
        netconnnew = NULL; 
        webcamnet = NULL; 
    }
    for(t = 0;t < WEBCAM_FIFO_NUM;t ++)
    {
        gui_memex_free(webcamfifobuf[t]);
    }
    
    gui_memin_free(webcam_line_buf0);
    gui_memin_free(webcam_line_buf1);
    gui_memin_free(buf);
    edit_delete(eport);	 
    t9_delete(t9);
    btn_delete(pixbtn);
    btn_delete(focbtn);
    btn_delete(conbtn); 
}  

/**
 * @brief       网络功能测试
 * @param       无
 * @retval      无
 */
uint8_t webcam_play(void)
{
    uint8_t res = 0;
    
    if (g_audiodev.status & (1 <<7 ))   /* 当前在放歌?? */
    {
        audio_stop_req(&g_audiodev);    /* 停止音频播放 */
        audio_task_delete();            /* 删除音乐播放任务. */
    } 
    
    window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)camera_remind_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
    
    if (ov5640_init())                  /* 初始化OV5640 */
    {
        window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)camera_remind_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        delay_ms(1500);  
        res = 1;
    }
    
    if (res == 0)
    {
        ov5640_rgb565_mode();                           /* RGB565模式 */
        ov5640_focus_init(); 
        ov5640_light_mode(0);                           /* 自动模式 */
        ov5640_color_saturation(3);                     /* 色彩饱和度0 */
        ov5640_brightness(4);                           /* 亮度0 */
        ov5640_contrast(3);                             /* 对比度0 */
        ov5640_sharpness(33);                           /* 自动锐度 */
        ov5640_focus_constant();                        /* 启动持续对焦 */
        
        window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        res = lwip_comm_init();                         /* lwip初始化 LwIP_Init一定要在OSInit之后和其他LWIP线程创建之前初始化!!!!!!!! */
        if (res == 0)                                   /* 网卡初始化成功 */
        {
            lwip_comm_dhcp_creat();                     /* 创建DHCP任务 */
            /* 提示正在DHCP获取IP */
            window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[2][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
            
            while(lwipdev.dhcpstatus == 0 || lwipdev.dhcpstatus == 1)   /* 等待DHCP分配成功 */
            {
                delay_ms(100);                           /* 等待 */
            }
            
            if(lwipdev.dhcpstatus == 2) window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[3][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);   /* DHCP成功 */
            else window_msg_box((lcddev.width-220)/2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[4][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);   /* DHCP失败 */
            
            if (lwipdev.dhcpstatus == 2 || lwipdev.dhcpstatus == 0XFF)
            {
                webcam_run();                               /* 开始网络摄像头 */
            }
        }
        else    /* 提示网卡初始化失败! */
        {
            window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
            delay_ms(2000);
        } 
    }
    
    system_task_return = 0;
    lwip_comm_destroy(); 
    pcf8574_write_bit(ETH_RESET_IO,1);              /* 保持复位LAN8720,降低功耗 */
    sw_sdcard_mode();                               /* 切换为SD卡模式 */
    return 0;
} 
