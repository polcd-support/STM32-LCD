/**
 ****************************************************************************************************
 * @file        facereco.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-05-26
 * @brief       APP-��������ͷ����
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

#include "webcamera.h"
#include "camera.h" 
#include "audioplay.h"
#include "./T9INPUT/t9input.h"
#include "./BSP/OV5640/ov5640.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/PCF8574/pcf8574.h"


/* webcam ��ť���� */
uint8_t*const webcam_btncaption_tbl[4][GUI_LANGUAGE_NUM]=
{ 
    {"����","�_��","ON",},
    {"�ر�","�P�]","OFF",}, 
    {"�ֱ���","�ֱ���","PIXEL",},
    {"�Խ�","����","FOCUS",}, 
};

/* webcam״̬��ʾ��Ϣ */
uint8_t*const webcam_stamsg_tbl[3][GUI_LANGUAGE_NUM]=
{
    {"�ֱ���:","�ֱ���:"," Pixel:",},
    {"��  ��:","��  ��:"," Speed",},
    {"֡  ��:","��  ��:"," Frame:",}, 
};
extern uint8_t*const camera_remind_tbl[4][GUI_LANGUAGE_NUM];

/* WEBCAM JPEG�ߴ�֧���б� */
const uint16_t webcam_jpeg_img_size_tbl[][2]=
{ 
    320,240,    /* QVGA  */
    640,480,    /* VGA */
    800,600,    /* SVGA */
    960,640,
    1024,768,   /* XGA */
    1280,800,   /* WXGA */
};

/* WEB CAMERA SEND���� */
/* �����������ȼ� */
#define WEBCAM_SEND_TASK_PRIO           2 
/* ���������ջ��С */
#define WEBCAM_SEND_STK_SIZE            1000
/* �����ջ�������ڴ����ķ�ʽ�������� */
OS_STK * WEBCAM_SEND_TASK_STK;
/* ������ */
void webcam_send_task(void *pdata); 

volatile uint8_t webcam_sendtask_status = 0;    /* WEB����ͷ��������״̬ */
                                                    /* 0,δ����/�����Ѿ���ɾ��; */
                                                    /* 1,����,������������ */
                                                    /* 2,����,������ͣ���� */
                                                    /* 0xff,����ɾ������ */

extern uint8_t g_ov_frame;                          /* ֡�� */
extern uint8_t*const netplay_remindmsg_tbl[5][GUI_LANGUAGE_NUM];

struct netconn *webcamnet = 0;                      /* WEB CAM TCP�������ӽṹ��ָ�� */
uint32_t *webcam_line_buf0;                         /* �����л���0 */
uint32_t *webcam_line_buf1;                         /* �����л���1 */

uint32_t webcam_sendnum;                            /* ����������(�ֽ�) */
volatile uint8_t webcam_oensec = 0;                   /* ���ӱ�־ */

//WEB CAM FIFO */
volatile uint16_t webcamfifordpos=0;                /* FIFO��λ�� */
volatile uint16_t webcamfifowrpos=0;                /* FIFOдλ�� */
uint32_t *webcamfifobuf[WEBCAM_FIFO_NUM];           /* ����WEBCAM_FIFO_SIZE������FIFO */
 

/**
 * @brief       ��ȡFIFO
 * @param       buf:���ݻ������׵�ַ
 * @param       fsize: �����С
 * @retval      ����ֵ:0,û�����ݿɶ�;1,������1�����ݿ�
 */
uint8_t webcam_fifo_read(uint32_t **buf)
{
    if (webcamfifordpos == webcamfifowrpos)
    {
        return 0;
    }
    
    webcamfifordpos++;          /* ��λ�ü�1 */
    
    if (webcamfifordpos >= WEBCAM_FIFO_NUM)
    {
        webcamfifordpos = 0;    /* ���� */
    }
    
    *buf = webcamfifobuf[webcamfifordpos];
    
    return 1;
}

/**
 * @brief       дһ��FIFO
 * @param       buf:���ݻ������׵�ַ
 * @param       fsize: �����С
 * @retval      ����ֵ:0,д��ɹ�;1,д��ʧ��
 */
uint8_t webcam_fifo_write(uint32_t *buf)
{
    uint16_t i;
    uint16_t temp = webcamfifowrpos;                /* ��¼��ǰдλ�� */
    webcamfifowrpos ++;                             /* дλ�ü�1 */
    
    if (webcamfifowrpos >= WEBCAM_FIFO_NUM)
    {
        webcamfifowrpos = 0;                        /* ���� */
    }
    
    if(webcamfifordpos == webcamfifowrpos)
    {
        webcamfifowrpos = temp;                     /* ��ԭԭ����дλ��,�˴�д��ʧ�� */
        //printf("shit:%d\r\n",temp);
        return 1;
    }
    
    for (i = 0;i < WEBCAM_LINE_SIZE;i++)
    {
        webcamfifobuf[webcamfifowrpos][i] = buf[i]; /* �������� */
    }
    
    return 0;
}   

/**
 * @brief       ����ͷjpeg���ݽ��ջص�����
 * @param       ��
 * @retval      ��
 */
void webcam_dcmi_rx_callback(void)
{    
    if (DMA1_Stream1->CR & (1 << 19))
    {
        webcam_fifo_write(webcam_line_buf0);    /* webcam_line_buf0д��FIFO */
    }
    else
    {
        webcam_fifo_write(webcam_line_buf1);    /* webcam_line_buf1д��FIFO */
    }
}

/**
 * @brief       ����webcam��������
 * @param       ��
 * @retval      ����ֵ:0,�ɹ�������,�������
 */
uint8_t webcam_send_task_creat(void)
{
    OS_CPU_SR cpu_sr = 0;
    uint8_t res;
    WEBCAM_SEND_TASK_STK = gui_memin_malloc(WEBCAM_SEND_STK_SIZE * sizeof(OS_STK));
    
    if (WEBCAM_SEND_TASK_STK == 0)
    {
        return 1;               /* �ڴ�����ʧ�� */
    }
    
    webcam_sendtask_status = 2;
    
    OS_ENTER_CRITICAL();        /* �����ٽ���(�޷����жϴ��) */
    res = OSTaskCreate(webcam_send_task,(void *)0,(OS_STK*)&WEBCAM_SEND_TASK_STK[WEBCAM_SEND_STK_SIZE - 1],WEBCAM_SEND_TASK_PRIO);
    OS_EXIT_CRITICAL();         /* �˳��ٽ���(���Ա��жϴ��) */
    return res;
}

/**
 * @brief       ɾ��webcam��������
 * @param       ��
 * @retval      ��
 */
void webcam_send_task_delete(void)
{ 
    if (webcam_sendtask_status)
    {
        webcam_sendtask_status = 0XFF;          /* ����ɾ������ */
        while(webcam_sendtask_status != 0);     /* �ȴ�����ɾ���ɹ� */
    }
    else
    {
        return ;
    }
    
    gui_memin_free(WEBCAM_SEND_TASK_STK);       /* �ͷ��ڴ� */
}

/**
 * @brief       webcam����ʼ����
 * @param       ��
 * @retval      ��
 */
void webcam_send_task_start(void)
{
    webcam_sendtask_status = 1;
}

/**
 * @brief       webcam����ֹͣ����
 * @param       ��
 * @retval      ��
 */
void webcam_send_task_stop(void)
{
    webcam_sendtask_status = 2;
}

/**
 * @brief       ��������ͷ��������,ͨ���������򴴽�
 * @param       ��
 * @retval      ��
 */
void webcam_send_task(void *pdata)
{ 
    uint8_t res = 0;
    uint32_t *tbuf;
    err_t err;                              /* �����־ */
    
    while (webcam_sendtask_status)
    {
        if (webcam_sendtask_status == 1)    /* ������������ */
        {
            res = webcam_fifo_read(&tbuf);
        }
        if (res)
        {
            err = netconn_write(webcamnet,tbuf,WEBCAM_LINE_SIZE * 4,NETCONN_COPY);  /* �������� */
            
            if (err != ERR_OK)              /* ���ͳɹ� */
            {
                if (err == ERR_ABRT || err == ERR_RST || err == ERR_CLSD)
                {
                    webcam_send_task_stop();
                }
            }
            else
            {
                webcam_sendnum += WEBCAM_LINE_SIZE * 4; /* ���������ۼ� */
            }
            
            res = 0;
        }
        else
        { 
            if (webcam_sendtask_status == 0XFF)
            {
                break;  /* ��Ҫɾ�������� */
            }
            
            delay_ms(1000/OS_TICKS_PER_SEC);    /* ��ʱһ��ʱ�ӽ��� */
        }
    }  	
    webcam_sendtask_status = 0;                 /* ����ɾ����� */
    OSTaskDel(WEBCAM_SEND_TASK_PRIO);           /* ɾ�����ֲ������� */
} 

/**
 * @brief       WEBCAM��ʾ��Ϣ
 * @param       x��X����
 * @param       y��Y����
 * @param       fsize�����ݴ�С
 * @param       msg������
 * @retval      ��
 */
void webcam_msg_show(uint16_t x,uint16_t y,uint16_t fsize,uint8_t* msg)
{
    gui_fill_rectangle(x,y,fsize * 9 / 2,fsize,NET_MEMO_BACK_COLOR);    /* ���ԭ������ʾ,���9���ַ���� */
    gui_show_string(msg,x,y,fsize * 9 / 2,fsize,fsize,WHITE);
}

/**
 * @brief       ������������ͷ����
 * @param       ��
 * @retval      ��
 */
void webcam_run(void)
{
    uint16_t t; 
    uint8_t res; 
    uint8_t rval = 0; 
    uint8_t jpeg_size = 3;
    
    uint16_t h1,h2;             /* ������ */
    uint16_t wh1,wh2,w2;        /* ������ */
    uint16_t t9height = 0;      /* T9���뷨�߶� */
    uint16_t btnw,btnh;         /* ��ť���/�߶� */
    uint8_t pbtn;               /* ��ť���� */
    
    uint8_t cbtnfsize = 0;      /* ��ť�����С */
    uint16_t tempy;
    uint16_t tport = 8088;      /* ��ʱ�˿ں�,(Ҫ���ӵĶ˿ں�)Ĭ��Ϊ8088; */
    
    uint8_t tcpconn = 0;        /* TCP����״̬:0,û������;1,������(��Client����); */
    uint8_t tcpstatus = 0;      /* TCP Server״̬,0,δ����;1,���� */
    
    _edit_obj* eport = 0;       /* �˿ڱ༭�� */
    _btn_obj* pixbtn = 0;       /* �л��ֱ��ʰ�ť */
    _btn_obj* focbtn = 0;       /* �Խ���ť */
    _btn_obj* conbtn = 0;       /* ����/�رհ�ť */
    _t9_obj * t9 = 0;           /* ���뷨 */
    uint8_t *buf;
    
    struct netconn *netconnnew = 0; /* ��TCP�������ӽṹ��ָ�� */
    err_t err;                      /* �����־ */
    
     
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
    app_filebrower((uint8_t*)APP_MFUNS_CAPTION_TBL[17][gui_phy.language],0X05); /* ��ʾ���� */
    g_point_color = WHITE;
    g_back_color = NET_MEMO_BACK_COLOR;

    gui_draw_rectangle(wh1,gui_phy.tbheight + h1 + gui_phy.tbfsize / 2,lcddev.width - 2 * wh1,3 * gui_phy.tbfsize / 2 + 2 * h2,WHITE);
    lcd_show_string(wh1 + wh2,gui_phy.tbheight + h1,200,32,gui_phy.tbfsize,(uint8_t*)"TCP Server",g_back_color);
    gui_draw_rectangle(wh1,gui_phy.tbheight + h1 * 2 + h2 * 2 + 5*gui_phy.tbfsize / 2,2 * wh2 + 8 * gui_phy.tbfsize,7 * gui_phy.tbfsize / 2 + 2 * h2,WHITE);
    lcd_show_string(wh1 + wh2,gui_phy.tbheight + h1 * 2 + h2 * 2 + 2 * gui_phy.tbfsize,200,32,gui_phy.tbfsize,(uint8_t*)"Status",g_back_color);

    sprintf((char*)buf,"IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);/* ��ʾIP��ַ */
    gui_show_string(buf,wh1 + wh2,gui_phy.tbheight + h1 + gui_phy.tbfsize+h2,lcddev.width,gui_phy.tbfsize,gui_phy.tbfsize,WHITE);
    gui_show_string((uint8_t*)"PORT:",lcddev.width - wh1 - wh2 - 9 * gui_phy.tbfsize / 2 - 2,gui_phy.tbheight + h1 + gui_phy.tbfsize+h2,200,32,gui_phy.tbfsize,WHITE);

    tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 3 * gui_phy.tbfsize - h2 / 2;
    gui_show_string((uint8_t*)webcam_stamsg_tbl[0][gui_phy.language],wh1 + wh2,tempy,200,32,gui_phy.tbfsize,WHITE);
    gui_show_string((uint8_t*)webcam_stamsg_tbl[1][gui_phy.language],wh1 + wh2,tempy + gui_phy.tbfsize + h2 / 2,200,32,gui_phy.tbfsize,WHITE);
    gui_show_string((uint8_t*)webcam_stamsg_tbl[2][gui_phy.language],wh1 + wh2,tempy + gui_phy.tbfsize * 2 + h2,200,32,gui_phy.tbfsize,WHITE);
    
    sprintf((char*)buf,"%dX%d",webcam_jpeg_img_size_tbl[jpeg_size][0],webcam_jpeg_img_size_tbl[jpeg_size][1]);
    tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 3 * gui_phy.tbfsize - h2 / 2;
    webcam_msg_show(wh1 + wh2 + gui_phy.tbfsize * 7 / 2,tempy,gui_phy.tbfsize,buf);
    
    eport = edit_creat(lcddev.width - wh1 - wh2 - 4 * gui_phy.tbfsize / 2 - 2 - 3,gui_phy.tbheight + h1 + gui_phy.tbfsize + h2 - 3,4 * gui_phy.tbfsize / 2 + 6,gui_phy.tbfsize + 6,0,4,gui_phy.tbfsize);/* ����eport�༭�� */
    t9 = t9_creat((lcddev.width % 5) / 2,lcddev.height - t9height,lcddev.width - (lcddev.width % 5 ),t9height,0); 
    tempy = gui_phy.tbheight + h1 * 2 + h2 * 2 + 2 * gui_phy.tbfsize + gui_phy.tbfsize / 2;
    
    pixbtn=btn_creat(lcddev.width - btnw - w2,tempy,btnw,btnh,0,0);
    focbtn=btn_creat(lcddev.width - btnw - w2,tempy + pbtn + btnh,btnw,btnh,0,0);
    conbtn=btn_creat(lcddev.width - btnw - w2,tempy + pbtn * 2 + btnh * 2,btnw,btnh,0,0);

    if (!webcamfifobuf[WEBCAM_FIFO_NUM-1] || !webcam_line_buf1 || !t9 || !conbtn || !eport)
    {
        rval = 1;/* �ڴ�����ʧ�� */
    }
    if (rval == 0)
    {
        eport->textbkcolor = NET_MEMO_BACK_COLOR;
        eport->textcolor = BLUE;                /* BLUE,��ʾ���Ա༭  */
        eport->type = 0X06;                     /* eport�����˸  */
        eport->cursorpos = 4;                   /* ���λ�������(8088,�ĸ��ַ�) */
        pixbtn->caption = webcam_btncaption_tbl[2][gui_phy.language];
        pixbtn->font = cbtnfsize;
        focbtn->caption = webcam_btncaption_tbl[3][gui_phy.language];
        focbtn->font = cbtnfsize; 
        conbtn->caption = webcam_btncaption_tbl[0][gui_phy.language];
        conbtn->font = cbtnfsize;

        sprintf((char*)buf,"%d",tport);
        strcpy((char*)eport->text,(const char *)buf);   /* �����˿ں� */
        t9_draw(t9);                                    /* ��T9���뷨 */
        btn_draw(pixbtn);                               /* ����ť */
        btn_draw(focbtn);                               /* ����ť */
        btn_draw(conbtn);                               /* ����ť */
        edit_draw(eport);                               /* ���༭�� */
        
        dcmi_init();                                    /* DCMI���� */
        dcmi_rx_callback = webcam_dcmi_rx_callback;     /* �������ݻص����� */
        dcmi_dma_init((uint32_t)webcam_line_buf0,(uint32_t)webcam_line_buf1,WEBCAM_LINE_SIZE,2,1);  /* DCMI DMA���� */
        ov5640_jpeg_mode();                             /* JPEGģʽ */
        //��Ϊ15֡ */
        ov5640_write_reg(0X3035,0X11);
        ov5640_write_reg(0x3824,0X1F);
        ov5640_image_window_set(0,0,1280,800);
        ov5640_outsize_set(16,4,webcam_jpeg_img_size_tbl[jpeg_size][0],webcam_jpeg_img_size_tbl[jpeg_size][1]); /* ��������ߴ�(640*480) */
        dcmi_start();                                   /* �������� */
        tim6_int_init(10000-1,10800-1);                  /* 10Khz����Ƶ��,1�����ж�,����֡�ʺͷ����ٶ�ͳ�� */
        delay_ms(100);
        rval = webcam_send_task_creat();                /* ������������ */
        while (rval == 0)
        {   
            tp_dev.scan(0);    
            in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);      /* �õ�������ֵ */
            edit_check(eport,&in_obj);
            t9_check(t9,&in_obj);
            if(t9->outstr[0] != NULL && tcpstatus == 0)       /* TCPδ�����������,��������ַ� */
            { 
                if((t9->outstr[0] <= '9' && t9->outstr[0] >= '0') || t9->outstr[0] == 0X08)
                {
                    edit_add_text(eport,t9->outstr);
                }
                
                t9->outstr[0] = NULL;   /* �������ַ� */
            }
            
            res = btn_check(focbtn,&in_obj);
            
            if (res && ((focbtn->sta & (1 << 7)) == 0) && (focbtn->sta & (1 << 6))) /* ������,�а����������ɿ�,����TP�ɿ��� */
            {   
                ov5640_focus_single();  /* �Զ��Խ� */
            }
            
            res = btn_check(pixbtn,&in_obj);
            
            if (res && ((pixbtn->sta & (1 << 7)) == 0) && (pixbtn->sta & (1 << 6))) /* ������,�а����������ɿ�,����TP�ɿ��� */
            {   
                jpeg_size++;
                
                if(jpeg_size > 5)jpeg_size = 0;
                if(jpeg_size > 3)
                {
                    /* ��Ϊ7.5֡ */
                    ov5640_write_reg(0X3035,0X21);
                    ov5640_write_reg(0x3824,0X0F); 
                }
                else
                {
                    /* ��Ϊ15֡ */
                    ov5640_write_reg(0X3035,0X11);
                    ov5640_write_reg(0x3824,0X1F); 	
                }
                
                ov5640_outsize_set(16,4,webcam_jpeg_img_size_tbl[jpeg_size][0],webcam_jpeg_img_size_tbl[jpeg_size][1]); /* ��������ߴ� */
                sprintf((char*)buf,"%dX%d",webcam_jpeg_img_size_tbl[jpeg_size][0],webcam_jpeg_img_size_tbl[jpeg_size][1]);
                tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 3 * gui_phy.tbfsize - h2 / 2;
                webcam_msg_show(wh1 + wh2 + gui_phy.tbfsize * 7 / 2,tempy,gui_phy.tbfsize,buf);
            }
            res = btn_check(conbtn,&in_obj);
            
            if (res && ((conbtn->sta & (1 << 7)) == 0) && (conbtn->sta & (1 << 6))) /* ������,�а����������ɿ�,����TP�ɿ��� */
            {   
                tcpstatus = !tcpstatus;
                tcpconn = 0;                    /* ���TCP����δ����(û��Client����) */
                
                if (tcpstatus == 1)            /* �������� */
                { 
                    tport = net_get_port(eport->text);                  /* �õ�port�� */
                    netconnnew = netconn_new(NETCONN_TCP);              /* ����һ��TCP���� */
                    netconnnew->recv_timeout=10;                        /* ��ֹ�����߳� */
                    err = netconn_bind(netconnnew,IP_ADDR_ANY,tport);   /* �󶨶˿� */
                    
                    if (err == ERR_OK)
                    {
                        err = netconn_listen(netconnnew);               /* �������ģʽ */
                        if (err != ERR_OK) tcpstatus = 0;               /* ����ʧ��  */
                    }
                    else
                    {
                        tcpstatus = 0;                                  /* ����ʧ��  */
                    }
                }
                if (tcpstatus == 0)/* TCP����δ����/�ر� */
                {
                    webcam_send_task_stop();                            /* webcam��������ֹͣ���� */
                    eport->type = 0X06;                                 /* eport�����˸   */
                    eport->textcolor = BLUE;                            /* ��ɫ,��ʾ�ɱ༭ */
                    edit_draw(eport);                                   /* ���༭��  */
                    net_disconnect(netconnnew,webcamnet);               /* �Ͽ����� */
                    netconnnew = NULL; 
                    webcamnet = NULL; 
                    net_tcpserver_remove_timewait();                    /* TCP Server,ɾ���ȴ�״̬ */
                }
                else
                {
                    edit_show_cursor(eport,0);                          /* �ر�eport�Ĺ�� */
                    eport->type = 0X04;                                 /* eip��겻��˸  */
                    eport->textcolor = WHITE;                           /* ��ɫ,��ʾ���ɱ༭ */
                }
                edit_draw(eport);                                       /* �ػ��༭��  */
                conbtn->caption = webcam_btncaption_tbl[tcpstatus][gui_phy.language]; 
                btn_draw(conbtn);                                       /* �ػ���ť */
            }
            
            if (tcpconn == 0 && tcpstatus == 1)                         /* ������TCP����,��TCP Clientδ����. */
            {
                err = netconn_accept(netconnnew,&webcamnet);            /* ������������ */
                
                if (err == ERR_OK)                                      /* �ɹ���⵽���� */
                { 
                    webcamnet->recv_timeout = 10; 
                    tcpconn = 1;                                        /* TCP Client����OK */
                    webcam_send_task_start();                           /* webcam��������ʼ���� */
                }
            }
            else if (webcam_sendtask_status == 2)                       /* �������ӷ������ݵ�ʱ��,ͻȻ�Ͽ���,��˵��client�����Ͽ���,������������ */
            {
                net_disconnect(webcamnet,NULL);                         /* �Ͽ����� */
                tcpconn = 0;
            }
            if (system_task_return)
            {
                delay_ms(10);
                if(tpad_scan(1))break;                                  /* TPAD����,�ٴ�ȷ��,�ų����� */
                else system_task_return = 0;
            }
            if (webcam_oensec)                                          /* 1��ʱ�䵽 */
            {
                webcam_oensec = 0;
                sprintf((char*)buf,"%dKB/S",webcam_sendnum/1024);
                webcam_sendnum = 0;
                tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 4 * gui_phy.tbfsize;
                webcam_msg_show(wh1 + wh2 + gui_phy.tbfsize * 7 / 2,tempy,gui_phy.tbfsize,buf); /* ��ʾ���紫������ */
                sprintf((char*)buf,"%dFPS",g_ov_frame);
                g_ov_frame = 0;
                tempy = gui_phy.tbheight + h1 * 2 + h2 * 3 + 5 * gui_phy.tbfsize + h2 / 2;
                webcam_msg_show(wh1 + wh2 + gui_phy.tbfsize * 7 / 2,tempy,gui_phy.tbfsize,buf); /* ��ʾ֡�� */
            }
            else delay_ms(5);
        }
        
        webcam_send_task_stop();                /* webcam��������ֹͣ���� */
        TIM6->CR1 &= ~(1 << 0);                 /* �رն�ʱ��6  */
        dcmi_stop();                            /* ֹͣ����ͷ���� */
        webcam_send_task_delete();              /* ɾ��webcam�������� */
        net_disconnect(netconnnew,webcamnet);   /* �Ͽ�����   */
        net_tcpserver_remove_timewait();        /* TCP Server,ɾ���ȴ�״̬  */
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
 * @brief       ���繦�ܲ���
 * @param       ��
 * @retval      ��
 */
uint8_t webcam_play(void)
{
    uint8_t res = 0;
    
    if (g_audiodev.status & (1 <<7 ))   /* ��ǰ�ڷŸ�?? */
    {
        audio_stop_req(&g_audiodev);    /* ֹͣ��Ƶ���� */
        audio_task_delete();            /* ɾ�����ֲ�������. */
    } 
    
    window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)camera_remind_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
    
    if (ov5640_init())                  /* ��ʼ��OV5640 */
    {
        window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)camera_remind_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        delay_ms(1500);  
        res = 1;
    }
    
    if (res == 0)
    {
        ov5640_rgb565_mode();                           /* RGB565ģʽ */
        ov5640_focus_init(); 
        ov5640_light_mode(0);                           /* �Զ�ģʽ */
        ov5640_color_saturation(3);                     /* ɫ�ʱ��Ͷ�0 */
        ov5640_brightness(4);                           /* ����0 */
        ov5640_contrast(3);                             /* �Աȶ�0 */
        ov5640_sharpness(33);                           /* �Զ���� */
        ov5640_focus_constant();                        /* ���������Խ� */
        
        window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        res = lwip_comm_init();                         /* lwip��ʼ�� LwIP_Initһ��Ҫ��OSInit֮�������LWIP�̴߳���֮ǰ��ʼ��!!!!!!!! */
        if (res == 0)                                   /* ������ʼ���ɹ� */
        {
            lwip_comm_dhcp_creat();                     /* ����DHCP���� */
            /* ��ʾ����DHCP��ȡIP */
            window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[2][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
            
            while(lwipdev.dhcpstatus == 0 || lwipdev.dhcpstatus == 1)   /* �ȴ�DHCP����ɹ� */
            {
                delay_ms(100);                           /* �ȴ� */
            }
            
            if(lwipdev.dhcpstatus == 2) window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[3][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);   /* DHCP�ɹ� */
            else window_msg_box((lcddev.width-220)/2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[4][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);   /* DHCPʧ�� */
            
            if (lwipdev.dhcpstatus == 2 || lwipdev.dhcpstatus == 0XFF)
            {
                webcam_run();                               /* ��ʼ��������ͷ */
            }
        }
        else    /* ��ʾ������ʼ��ʧ��! */
        {
            window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
            delay_ms(2000);
        } 
    }
    
    system_task_return = 0;
    lwip_comm_destroy(); 
    pcf8574_write_bit(ETH_RESET_IO,1);              /* ���ָ�λLAN8720,���͹��� */
    sw_sdcard_mode();                               /* �л�ΪSD��ģʽ */
    return 0;
} 
