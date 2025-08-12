/**
 ****************************************************************************************************
 * @file        netplay.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-12-01
 * @brief       APP-����ͨ�� ����
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
 * V1.1 20221201
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "netplay.h"
#include "audioplay.h"
#include "./T9INPUT/t9input.h"
#include "./BSP/PCF8574/pcf8574.h"


/* netplay��ʾ��Ϣ */
uint8_t*const netplay_remindmsg_tbl[5][GUI_LANGUAGE_NUM]=
{
    {"���������!���ڳ�ʼ������...","Ո����W��!���ڳ�ʼ���W��...","Pls insert cable!Ethernet Initing..",}, 
    {"��ʼ��ʧ��!��������!","��ʼ��ʧ��!Ո�z��W��!","Init failed!Check the cable!",},  
    {"����DHCP��ȡIP...","����DHCP�@ȡIP...","DHCP IP configing...",},  
    {"DHCP��ȡIP�ɹ�!","DHCP�@ȡIP�ɹ�!","DHCP IP config OK!",},  
    {"DHCP��ȡIPʧ��,ʹ��Ĭ��IP!","DHCP�@ȡIPʧ��,ʹ��Ĭ�JIP!","DHCP IP config fail!Use default IP",},  
};

/* netplay IP��Ϣ */
uint8_t*const netplay_ipmsg[5][GUI_LANGUAGE_NUM]=
{
    {"����MAC��ַ:","���CMAC��ַ:","Local MAC Addr:",}, 
    {" Զ��IP��ַ:"," �h��IP��ַ:","Remote IP Addr:",}, 
    {" ����IP��ַ:"," ����IP��ַ:"," Local IP Addr:",}, 
    {"   ��������:","   �ӾW�ڴa:","   Subnet MASK:",},
    {"       ����:","       �W�P:","       Gateway:",},  
};

/* ������ʾ */
uint8_t*const netplay_netspdmsg[GUI_LANGUAGE_NUM]={"   �����ٶ�:","   �W�j�ٶ�:","Ethernet Speed:"};

/* netplay ������ʾ��Ϣ */
uint8_t*const netplay_testmsg_tbl[3][GUI_LANGUAGE_NUM]=
{
    {"�ɼ������״̬.","�əz���B�Ӡ�B.","to check the connection.",}, 
    {"2,�����������:","2,�ڞg�[��ݔ��:","2,Input:",}, 	
    {"�ɵ�¼web���档","�ɵ��web���档","in browser,you can log on to website.",}, 	
};

/* netplay memo��ʾ��Ϣ */
uint8_t*const netplay_memoremind_tb[2][GUI_LANGUAGE_NUM]=
{
    {"������:","���Յ^:","Receive:",},
    {"������:","�l�ͅ^:","Send:",},
};

/* netplay ���԰�ť���� */
uint8_t*const netplay_tbtncaption_tb[GUI_LANGUAGE_NUM]={"��ʼ����","�_ʼ�yԇ","Start Test",};

/* netplay Э����� */
uint8_t*const netplay_protcaption_tb[GUI_LANGUAGE_NUM]={"Э��","�f�h","PROT",};

/* netplay Э������ */
uint8_t*const netplay_protname_tb[3]={"TCP Server","TCP Client","UDP",};

/* netplay �˿ڱ��� */
uint8_t*const netplay_portcaption_tb[GUI_LANGUAGE_NUM]={"�˿�:","�˿�:","Port:",};

/* netplay IP��ַ���� */
uint8_t*const netplay_ipcaption_tb[2][GUI_LANGUAGE_NUM]=
{
    {"Ŀ��IP:","Ŀ��IP:","Target IP:",},
    {"����IP:","���CIP:"," Local IP:",},
};

/* netplay ��ť���� */
uint8_t*const netplay_btncaption_tbl[5][GUI_LANGUAGE_NUM]=
{
    {"Э��ѡ��","�f�h�x��","PROT SEL",},
    {"����","�B��","Conn",},
    {"�Ͽ�","���_","Dis Conn",},
    {"�������","�������","Clear",},
    {"����","�l��","Send",},
};

/* ����ģʽѡ�� */
uint8_t*const netplay_mode_tbl[3]={"TCP Server","TCP Client","UDP"};

/* ����������ʾ��Ϣ */
uint8_t*const netplay_connmsg_tbl[4][GUI_LANGUAGE_NUM]=
{
    {"��������...","�����B��...","Connecting...",},
    {"����ʧ��!","�B��ʧ��!","Connect fail!",},
    {"���ӳɹ�!","�B�ӳɹ�!","Connect OK!",},
    {"LwIP����!","LwIP�e�`!","LwIP Error!",}, 
};

/**
 * @brief       ����������
 * @param       ��
 * @retval      ��
 */
void net_load_ui(void)
{
    uint8_t *buf;
    uint8_t fsize = 0;
    uint16_t length;
    uint8_t temp;
    buf = gui_memin_malloc(100);        /* ����100�ֽ� */
    g_back_color = LGRAY;  
    app_filebrower((uint8_t*)APP_MFUNS_CAPTION_TBL[13][gui_phy.language],0X05); /* ��ʾ���� */
    if (lcddev.width == 240) 
    {
        fsize = 12;
    }
    else
    {
        fsize = 16;
    }
    
    length = strlen((char*)netplay_ipmsg[0][gui_phy.language]); /* �õ��ַ������� */
    gui_fill_rectangle(0,gui_phy.tbheight,lcddev.width,lcddev.height-gui_phy.tbheight,g_back_color);    /* ����ɫ */
    gui_show_string(netplay_ipmsg[0][gui_phy.language],10,gui_phy.tbheight+5,lcddev.width,fsize,fsize,BLUE);
    gui_show_string(netplay_ipmsg[2][gui_phy.language],10,(4+fsize)*1+gui_phy.tbheight+5,lcddev.width,fsize,fsize,BLUE);
    gui_show_string(netplay_ipmsg[3][gui_phy.language],10,(4+fsize)*2+gui_phy.tbheight+5,lcddev.width,fsize,fsize,BLUE);
    gui_show_string(netplay_ipmsg[4][gui_phy.language],10,(4+fsize)*3+gui_phy.tbheight+5,lcddev.width,fsize,fsize,BLUE);
    gui_show_string(netplay_netspdmsg[gui_phy.language],10,(4+fsize)*4+gui_phy.tbheight+5,lcddev.width,fsize,fsize,BLUE);   /* ������ʾ	 */
    sprintf((char*)buf,"%02X-%02X-%02X-%02X-%02X-%02X",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);  /* ��ʾMAC��ַ */
    gui_show_string(buf,10+length*fsize/2,gui_phy.tbheight+5,lcddev.width,fsize,fsize,RED);
    sprintf((char*)buf,"%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);  /* ��ʾIP��ַ */
    gui_show_string(buf,10+length*fsize/2,(4+fsize)*1+gui_phy.tbheight+5,lcddev.width,fsize,fsize,RED);
    sprintf((char*)buf,"%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);  /* ��ʾ�������� */
    gui_show_string(buf,10+length*fsize/2,(4+fsize)*2+gui_phy.tbheight+5,lcddev.width,fsize,fsize,RED);
    sprintf((char*)buf,"%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);  /* ��ʾ���� */
    gui_show_string(buf,10+length*fsize/2,(4+fsize)*3+gui_phy.tbheight+5,lcddev.width,fsize,fsize,RED);
    
    temp = ethernet_chip_get_speed();
    
    if (temp)
    {
        temp = 100; /* 100M���� */
    }
    else
    {
        temp = 10; /* 10M���� */
    }
    
    sprintf((char*)buf,"%dM",temp); /* ���� */
    gui_show_string(buf,10+length*fsize/2,(4+fsize)*4+gui_phy.tbheight+5,lcddev.width,fsize,fsize,RED);	 
    
    sprintf((char*)buf,"1,Ping %d.%d.%d.%d %s",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3],netplay_testmsg_tbl[0][gui_phy.language]);   /* 1,ping xxx.xxx.xxx.xxx �ɼ������״̬. */
    gui_show_string(buf,10,(4+fsize)*6+gui_phy.tbheight+5,lcddev.width-20,2*fsize,fsize,BLUE);	 
    sprintf((char*)buf,"%s%d.%d.%d.%d %s",netplay_testmsg_tbl[1][gui_phy.language],lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3],netplay_testmsg_tbl[2][gui_phy.language]);   /* 1,ping xxx.xxx.xxx.xxx �ɼ������״̬. */
    gui_show_string(buf,10,(4+fsize)*8+gui_phy.tbheight+5,lcddev.width-20,2*fsize,fsize,BLUE);	 
    gui_memin_free(buf);
}

/**
 * @brief       ��ʾ��ʾ��Ϣ
 * @param       y     : Y���꣨X����㶨��0��ʼ��
 * @param       height: ����߶�
 * @param       fsize : �����С
 * @param       tx    : �����ֽ���
 * @param       rx    : �����ֽ���
 * @param       prot  : Э������
 *   @arg       0: TCP Server
 *   @arg       1: TCP Client
 *   @arg       2: UDP
 * @param       flag  : ���±��
 *   @arg       bit0: 0: ������,1: ����tx����
 *   @arg       bit1: 0: ������,1: ����rx����
 *   @arg       bit2: 0: ������,1: ����prot����
 * @retval      ��
 */
void net_msg_show(uint16_t y,uint16_t height,uint8_t fsize,uint32_t tx,uint32_t rx,uint8_t prot,uint8_t flag)
{
    uint16_t xdis;
    uint8_t *pbuf;
    pbuf = gui_memin_malloc(100);
    
    if(pbuf == NULL)
    {
        return;         /* �ڴ�����ʧ�� */
    }
    
    if(prot > 2)
    {
        prot = 2;
    }
    xdis = (lcddev.width - (35 * fsize / 2)) / 3;   /* ��϶ */
    
    g_back_color = NET_MSG_BACK_COLOR;
    
    if(flag & 1 << 0)   /* ����TX���� */
    {
        gui_fill_rectangle(xdis/2,y+(height-fsize)/2,10*fsize/2,fsize,NET_MSG_BACK_COLOR);  /* ���֮ǰ����ʾ */
        sprintf((char*)pbuf,"TX:%d",tx);
        gui_show_string(pbuf,xdis/2,y+(height-fsize)/2,lcddev.width,fsize,fsize,NET_MSG_FONT_COLOR);    /* TX�ֽ�����ʾ */
    }
    if(flag & 1 << 1)   /* ����RX���� */
    { 
        gui_fill_rectangle(xdis/2+10*fsize/2+xdis,y+(height-fsize)/2,10*fsize/2,fsize,NET_MSG_BACK_COLOR);  /* ���֮ǰ����ʾ */
        sprintf((char*)pbuf,"RX:%d",rx);
        gui_show_string(pbuf,xdis/2+10*fsize/2+xdis,y+(height-fsize)/2,lcddev.width,fsize,fsize,NET_MSG_FONT_COLOR);    /* RX�ֽ�����ʾ */
    }
    if(flag & 1 << 2)   /* ����prot���� */
    {
        gui_fill_rectangle(xdis/2+20*fsize/2+xdis*2,y+(height-fsize)/2,15*fsize/2,fsize,NET_MSG_BACK_COLOR);    /* ���֮ǰ����ʾ */
        sprintf((char*)pbuf,"%s:%s",netplay_protcaption_tb[gui_phy.language],netplay_protname_tb[prot]);        /* Э�� */
        gui_show_string(pbuf,xdis/2+20*fsize/2+xdis*2,y+(height-fsize)/2,lcddev.width,fsize,fsize,NET_MSG_FONT_COLOR);  /* ��ʾЭ�� */
    }
    gui_memin_free(pbuf);//�ͷ��ڴ� */
    
}

/**
 * @brief       ���ñ༭����ɫ
 * @param       ipx    : IP�༭��
 * @param       portx  : Port�༭��
 * @param       prot   : Э��
 *   @arg       0: TCP Server
 *   @arg       1: TCP Client
 *   @arg       2: UDP
 * @param       connsta: ����״̬
 *   @arg       0: δ����
 *   @arg       1: ������
 * @retval      ��
 */
void net_edit_colorset(_edit_obj *ipx,_edit_obj *portx,uint8_t prot,uint8_t connsta)
{
    if (connsta == 1)                           /* ���ӳɹ�?û��˵,���ǲ��ɱ༭ */
    {
        ipx->textcolor = WHITE;
        portx->textcolor = WHITE;
    }
    else                                        /* ������״̬ */
    {
        switch (prot)
        {
            case 0:                             /* TCP ServerЭ�� */
                portx->textcolor = GREEN;       /* ��ɫ,��ʾ���Ա༭ */
                ipx->textcolor = WHITE;         /* ��ɫ,�̶����� */
                break;
            case 1:                             /* TCP ClientЭ�� */
            case 2:                             /* UDPЭ�� */
                portx->textcolor = GREEN;       /* ��ɫ,��ʾ���Ա༭ */
                ipx->textcolor = GREEN;         /* ��ɫ,��ʾ���Ա༭ */
                break;
        }
    }
    
    edit_draw(ipx);                             /* ���༭�� */
    edit_draw(portx);                           /* ���༭�� */
} 

/**
 * @brief       ���ַ�����ʽ�Ķ˿ں�ת��Ϊ������ʽ�Ķ˿ں�
 * @param       str: �ַ�����ʽ�Ķ˿ں�
 * @retval      ������ʽ�Ķ˿ں�
 */
uint16_t net_get_port(uint8_t *str)
{
    uint16_t port;
    port = atoi((char*)str);
    return port;
}

/**
 * @brief       ���ַ�����ʽ��IP��ַת��Ϊ������ʽ��IP��ַ
 * @param       str: �ַ�����ʽ��IP��ַ
 * @retval      ת�����
 *    @arg      0: ת���ɹ�
 *    @arg      1: �ַ�����ʽ��IP��ַ����ת��ʧ��
 */
uint32_t net_get_ip(uint8_t *str)
{
    uint8_t *p1, *p2, *ipstr;
    struct ip_addr ipx;
    uint8_t ip[4];
    ipstr = gui_memin_malloc(30);

    if (ipstr == NULL)return 0;

    strcpy((char *)ipstr, (char *)str); /* �����ַ��� */
    p1 = ipstr;
    p2 = (uint8_t *)strstr((const char *)p1, ".");

    if (p2 == NULL)
    {
        gui_memin_free(ipstr);  /* IP���� */
        return 0;
    }

    p2[0] = 0;
    ip[0] = atoi((char *)p1);   /* �õ���һ��ֵ */
    p1 = p2 + 1;
    p2 = (uint8_t *)strstr((const char *)p1, ".");

    if (p2 == NULL)
    {
        gui_memin_free(ipstr);  /* IP���� */
        return 0;
    }

    p2[0] = 0;
    ip[1] = atoi((char *)p1);   /* �õ��ڶ���ֵ */
    p1 = p2 + 1;
    p2 = (uint8_t *)strstr((const char *)p1, ".");

    if (p2 == NULL)
    {
        gui_memin_free(ipstr);  /* IP���� */
        return 0;
    }

    p2[0] = 0;
    ip[2] = atoi((char *)p1);   /* �õ�������ֵ */
    p1 = p2 + 1;
    ip[3] = atoi((char *)p1);   /* �õ����ĸ�ֵ */
    IP4_ADDR(&ipx, ip[0], ip[1], ip[2], ip[3]);
    gui_memin_free(ipstr);
    return ipx.addr;/* ���صõ���IP��ַ */
}

extern void tcp_pcb_purge(struct tcp_pcb *pcb);     /* �� tcp.c���� */
extern struct tcp_pcb *tcp_active_pcbs;             /* �� tcp.c���� */
extern struct tcp_pcb *tcp_tw_pcbs;                 /* �� tcp.c���� */

/**
 * @brief       ǿ��ɾ��TCP Server�����Ͽ�ʱ��time wait
 * @param       ��
 * @retval      ��
 */
void net_tcpserver_remove_timewait(void)
{
    struct tcp_pcb *pcb, *pcb2;

    while (tcp_active_pcbs != NULL)delay_ms(10); /* �ȴ�tcp_active_pcbsΪ�� */

    pcb = tcp_tw_pcbs;

    while (pcb != NULL) /* ����еȴ�״̬��pcbs */
    {
        tcp_pcb_purge(pcb);
        tcp_tw_pcbs = pcb->next;
        pcb2 = pcb;
        pcb = pcb->next;
        memp_free(MEMP_TCP_PCB, pcb2);
    }
}

/**
 * @brief       �Ͽ�����
 * @param       netconn1 : �������ӽṹ��1
 * @param       netconn2 : �������ӽṹ��2
 * @retval      ��
 */
void net_disconnect(struct netconn *netconn1,struct netconn *netconn2)
{
    if (netconn1 != NULL)           /* ���ӽṹ����Ч? */
    {
        if (netconn1->type == NETCONN_TCP)netconn_close(netconn1);              /* �ر�TCP netconn1���� */
        else if (netconn1->type == NETCONN_UDP)netconn_disconnect(netconn1);    /* �ر�UDP netconn1������ */

        netconn_delete(netconn1);   /* ɾ��netconn1���� */
    }

    if (netconn2 != NULL)           /* ���ӽṹ����Ч? */
    {
        if (netconn2->type == NETCONN_TCP)netconn_close(netconn2);              /* �ر�TCP netconn2���� */
        else if (netconn2->type == NETCONN_UDP)netconn_disconnect(netconn2);    /* �ر�UDP netconn2������ */

        netconn_delete(netconn2);   /* ɾ��netconn2���� */
    }
}



/**
 * @brief       �������������
 * @param       ��
 * @retval      ��
 */
uint8_t net_test(void)
{
    struct netconn *netconncom = 0; /* ͨ��TCP/UDP�������ӽṹ��ָ��(TCP Server/TCP Client/UDPͨ��) */
    struct netconn *netconnnew = 0; /* ��TCP/UDP�������ӽṹ��ָ��(ֻ��TCP Server���õ����) */
    _edit_obj *eip = 0;         /* IP�༭�� */
    _edit_obj *eport = 0;       /* �˿ڱ༭�� */
    _btn_obj *protbtn = 0;      /* Э��ѡ��ť */
    _btn_obj *sendbtn = 0;      /* ���Ͱ�ť */
    _btn_obj *connbtn = 0;      /* ���Ӱ�ť */
    _btn_obj *clrbtn = 0;       /* �����ť */
    _memo_obj *rmemo = 0, * smemo = 0;  /* memo�ؼ� */
    _t9_obj *t9 = 0;            /* ���뷨 */

    uint8_t ip_height, ip_fsize;            /* IP/PORT����߶Ⱥ������С */
    uint8_t msg_height;                     /* ��Ϣ����߶Ⱥ������С */
    uint16_t memo_width, btn_width;         /* memo�ؼ����,��ť�Ŀ�� */
    uint16_t rmemo_height, smemo_height;    /* ����memo�ͷ���memo�ĸ߶� */
    uint16_t rbtn_height;                   /* ��������ť�ĸ߶� */
    uint8_t m_offx, sm_offy, rm_offy;       /* memo x�����ƫ��;smemo��rmemo y����ƫ�� */
    uint8_t fsize, sbtnfsize;               /* �����С,�ͷ��Ͱ�ť�����С */
    uint16_t t9height;                      /* ���뷨�ĸ߶� */
    uint16_t tempx, tempy;
    uint8_t *ipcaption = netplay_ipcaption_tb[1][gui_phy.language]; /* Ĭ����TCP Serverģʽ,��ʾ����IP */
    
    uint16_t res;
    uint8_t rval = 0;
    uint8_t editflag = 0;   /* 0,�༭����smemo
                             * 1,�༭����eip
                             * 2,�༭����eport
                             */
    uint8_t *p, *ptemp;
    uint32_t rxcnt = 0;
    uint32_t txcnt = 0;
    uint8_t protocol = 0;   /* Ĭ��TCP ServerЭ�� 
                             * 0,TCP ServerЭ��
                             * 1,TCP ClientЭ��
                             * 2,UDPЭ��
                             */
    uint8_t oldconnstatus = 0; /* �ϵ�״̬ */
    uint8_t connstatus = 0; /* 0,δ����,1,������ */
    uint8_t tcpconn = 0;    /* TCP�����Ƿ���:0,δ����;1,������ */
    uint32_t oldaddr = 0;   /* ���һ���������Ե�ip��ַ */
    uint16_t oldport = 0;   /* ���һ���������Ե�port */
    
    err_t err;              /* �����־ */
    ip_addr_t tipaddr;      /* ��ʱIP��ַ */
    uint16_t tport = 8088;  /* ��ʱ�˿ں�,(Ҫ���ӵĶ˿ں�)Ĭ��Ϊ8088; */
    struct netbuf *recvbuf; /* ���ջ����� */
    struct netbuf *sendbuf; /* ���ͻ����� */
    uint16_t *bkcolor;
    

    lcd_clear(NET_MEMO_BACK_COLOR);/* ���� */

    if (lcddev.width == 240)
    {
        ip_height = 20,ip_fsize = 12;
        msg_height = 16;
        memo_width = 172,btn_width = 56;
        rmemo_height = 72,smemo_height = 36;
        rbtn_height = 20;
        m_offx = 4,sm_offy = 4,rm_offy = 5;
        fsize = 12,sbtnfsize = 16;
        t9height = 134;  
    }
    else if (lcddev.width == 272)
    {
        ip_height = 20,ip_fsize = 12;
        msg_height = 20;
        memo_width = 180,btn_width = 68;
        rmemo_height = 160,smemo_height = 48;
        rbtn_height = 34;
        m_offx = 4,sm_offy = 8,rm_offy = 8;
        fsize = 12,sbtnfsize = 16;
        t9height = 176;
    }
    else if (lcddev.width==320)
    { 
        ip_height = 24,ip_fsize = 16;
        msg_height = 20;
        memo_width = 208,btn_width = 80;
        rmemo_height = 144,smemo_height = 48;
        rbtn_height = 40;
        m_offx = 12,sm_offy = 8,rm_offy = 10;
        fsize = 16,sbtnfsize = 24;
        t9height = 176;
    }
    else if (lcddev.width == 480)
    {
        ip_height = 36,ip_fsize = 24;
        msg_height = 28;
        memo_width = 304,btn_width = 100;
        rmemo_height = 336,smemo_height = 64;
        rbtn_height = 50;
        m_offx = 16,sm_offy = 10,rm_offy = 9;
        fsize = 16,sbtnfsize = 24;
        t9height = 266;
    }
    else if (lcddev.width == 600)
    {
        ip_height = 36,ip_fsize = 24;
        msg_height = 28;
        memo_width = 364,btn_width = 120;
        rmemo_height = 436,smemo_height = 80;
        rbtn_height = 50;
        m_offx = 16,sm_offy = 12,rm_offy = 10;
        fsize = 16,sbtnfsize = 24;
        t9height = 368;
    }
    else if (lcddev.width == 800)
    {
        ip_height = 60,ip_fsize = 32;
        msg_height = 30;
        memo_width = 500,btn_width = 200;
        rmemo_height = 480,smemo_height = 150;
        rbtn_height = 50;
        m_offx = 16,sm_offy = 10,rm_offy = 10;
        fsize = 16,sbtnfsize = 24;
        t9height = 488;
    }
    gui_fill_rectangle(0,0,lcddev.width,ip_height,NET_IP_BACK_COLOR);                                                                           /* ���IP��ַ���򱳾� */
    gui_fill_rectangle(0,ip_height,lcddev.width,msg_height,NET_MSG_BACK_COLOR);                                                                 /* ��Ϣ���򱳾� */
    gui_draw_hline(0,ip_height + msg_height - 1,lcddev.width,NET_COM_RIM_COLOR);                                                                /* �ָ��� */
    tempy = ip_height+msg_height + rmemo_height + fsize + 2 * rm_offy; 
    gui_draw_hline(0,tempy,lcddev.width,NET_COM_RIM_COLOR);                                                                                     /* �ָ��� */
    tempx = (lcddev.width - 35 * ip_fsize / 2) / 3;
    gui_show_string(ipcaption,tempx,(ip_height-ip_fsize)/2,lcddev.width,ip_fsize,ip_fsize,WHITE);                                               /* ����IP/Ŀ��IP */
    tempx = lcddev.width - tempx - 10 * ip_fsize / 2;
    gui_show_string(netplay_portcaption_tb[gui_phy.language],tempx,(ip_height-ip_fsize) / 2,lcddev.width,ip_fsize,ip_fsize,WHITE);              /* �˿�: */
    tempy = ip_height + msg_height + rm_offy + fsize; 
    gui_show_string(netplay_memoremind_tb[0][gui_phy.language],m_offx,tempy-fsize - rm_offy / 3,lcddev.width,fsize,fsize,NET_MSG_FONT_COLOR);   /* ��ʾ������ */
    rmemo = memo_creat(m_offx,tempy,memo_width,rmemo_height,0,0,fsize,NET_RMEMO_MAXLEN);                                                        /* ����memo�ؼ�,���NET_RMEMO_MAXLEN���ַ� */
    tempx = lcddev.width - tempx - 10 * ip_fsize / 2;
    eip = edit_creat(strlen((char*)ipcaption)*ip_fsize/2+tempx,(ip_height-ip_fsize-6)/2,15*ip_fsize/2+6,ip_fsize + 6,0,4,ip_fsize);             /* ����ip�༭�� */
    tempx = (lcddev.width - 35 * ip_fsize / 2) / 3;
    tempx = lcddev.width - tempx - 10 * ip_fsize / 2;
    eport = edit_creat(tempx + 5 * ip_fsize / 2,(ip_height - ip_fsize - 6) / 2,5 * ip_fsize / 2 + 6,ip_fsize + 6,0,4,ip_fsize);                 /* ����eport�༭�� */
    tempy = ip_height + msg_height + rm_offy * 2 + rmemo_height + fsize * 2 + sm_offy; 
    gui_show_string(netplay_memoremind_tb[1][gui_phy.language],m_offx,tempy - fsize - sm_offy/3,lcddev.width,fsize,fsize,NET_MSG_FONT_COLOR);   /* ��ʾ������ */
    smemo = memo_creat(m_offx,tempy,memo_width,smemo_height,0,1,fsize,NET_SMEMO_MAXLEN);                                                        /* ���NET_SMEMO_MAXLEN���ַ� */
    tempy = ip_height + msg_height + rm_offy * 2 + rmemo_height+fsize*2+sm_offy*2+smemo_height; 
    t9 = t9_creat((lcddev.width % 5) / 2,tempy,lcddev.width - (lcddev.width % 5),t9height,0);                                                   /* t9�Ŀ�ȱ�����5�ı��� */
    tempy = ip_height+msg_height+rm_offy+fsize; 
    tempx = (lcddev.width - (m_offx + memo_width + btn_width)) / 2 + m_offx + memo_width;
    protbtn = btn_creat(tempx,tempy,btn_width,rbtn_height,0,0);	
    memo_width = (rmemo_height - 3 * rbtn_height) / 2;                                                                                          /* ����һ��memo_width. */
    
    
    
    if (memo_width > rbtn_height / 2)memo_width = rbtn_height / 2;
    
    connbtn = btn_creat(tempx,tempy + memo_width + rbtn_height,btn_width,rbtn_height,0,0);
    clrbtn = btn_creat(tempx,tempy + memo_width * 2 + rbtn_height*2,btn_width,rbtn_height,0,0);
    tempy = ip_height + msg_height + rm_offy * 2 + rmemo_height + fsize * 2 + sm_offy; 
    sendbtn = btn_creat(tempx,tempy,btn_width,smemo_height,0,2);    /* �����߽ǰ�ť */
    
    p = gui_memin_malloc(1500);                   /* ����1500�ֽ��ڴ� */
    ptemp = gui_memin_malloc(100);                /* ����100�ֽ��ڴ� */
    
    if (!rmemo||!eip||!eport||!smemo||!t9||!protbtn||!connbtn||!clrbtn||!sendbtn||!p||!ptemp)rval = 1;    /* ����ʧ��.  */
    
    if (rval == 0)                              /* �����ɹ� */
    { 
        protbtn->caption = netplay_btncaption_tbl[0][gui_phy.language];
        protbtn->font = fsize;
        connbtn->caption = netplay_btncaption_tbl[1][gui_phy.language];
        connbtn->font = fsize;
        clrbtn->caption = netplay_btncaption_tbl[3][gui_phy.language];
        clrbtn->font = fsize;
        sendbtn->bkctbl[0] = 0X6BF6;        /* �߿���ɫ */
        sendbtn->bkctbl[1] = 0X545E;        /* 0X8C3F.��һ�е���ɫ */
        sendbtn->bkctbl[2] = 0X5C7E;        /* 0X545E,�ϰ벿����ɫ */
        sendbtn->bkctbl[3] = 0X2ADC;        /* �°벿����ɫ */
        sendbtn->bcfucolor = WHITE;         /* �ɿ�ʱΪ��ɫ */
        sendbtn->bcfdcolor = BLACK;         /* ����ʱΪ��ɫ */
        sendbtn->caption = netplay_btncaption_tbl[4][gui_phy.language];
        sendbtn->font = sbtnfsize;
        eip->textbkcolor = NET_IP_BACK_COLOR;
        eip->textcolor = WHITE;
        eport->textbkcolor = NET_IP_BACK_COLOR;
        eport->textcolor = GREEN;           /* GREEN,��ʾ���Ա༭ */
        rmemo->textbkcolor = WHITE;
        rmemo->textcolor = BLACK;
        smemo->textbkcolor = WHITE;
        smemo->textcolor = BLACK; 
        sprintf((char*)ptemp,"%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
        strcpy((char*)eip->text,(const char *)ptemp);   /* ����IP��ַ */
        sprintf((char*)ptemp,"%d",tport);
        strcpy((char*)eport->text,(const char *)ptemp); /* �����˿ں� */
        edit_draw(eip);                                 /* ���༭�� */
        edit_draw(eport);                               /* ���༭�� */
        memo_draw_memo(smemo,0);                        /* ��memo�ؼ� */
        memo_draw_memo(rmemo,0);                        /* ��memo�ؼ� */
        btn_draw(protbtn);
        btn_draw(connbtn);
        btn_draw(clrbtn);
        btn_draw(sendbtn); 
        t9_draw(t9);
        net_msg_show(ip_height,msg_height,fsize,txcnt,rxcnt,protocol,0X07);/* ��ʾ��Ϣ */
    } 
    while (rval == 0)
    {
        tp_dev.scan(0);    
        in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);              /* �õ�������ֵ */
        delay_ms(1000/OS_TICKS_PER_SEC);                    /* ��ʱһ��ʱ�ӽ��� */
        
        if (system_task_return)
        {
            delay_ms(15);
            if (tpad_scan(1))break;                         /* TPAD����,�ٴ�ȷ��,�ų����� */
            else system_task_return=0;
        }
        if (connstatus == 0)                                /* ��������δ������ʱ��,�����л����봰�� */
        {
            if (smemo->top<in_obj.y&&in_obj.y<(smemo->top+smemo->height)&&(smemo->left<in_obj.x)&&in_obj.x<(smemo->left+smemo->width))/* ��smemo�ڲ� */ 
            { 
                editflag = 0;                               /* �༭����smemo */
                edit_show_cursor(eip,0);                    /* �ر�edit�Ĺ�� */
                edit_show_cursor(eport,0);                  /* �ر�eport�Ĺ�� */
                eip->type = 0X04;                           /* eip��겻��˸  */
                eport->type = 0X04;                         /* eport��겻��˸  */
                smemo->type = 0X01;                         /* memo�ɱ༭,��˸��� */
            } 
            if (eip->top<in_obj.y&&in_obj.y<(eip->top+eip->height)&&(eip->left<in_obj.x)&&in_obj.x<(eip->left+eip->width))/* ��eip���ڲ�  */
            {
                if(protocol == 0)continue;                  /* tcp serverЭ���ʱ��,����Ҫ����IP��ַ */
                editflag = 1;                               /* �༭����eip */
                memo_show_cursor(smemo,0);                  /* �ر�smemo�Ĺ�� */
                edit_show_cursor(eport,0);                  /* �ر�eport�Ĺ�� */
                eip->type = 0X06;                           /* eip�����˸  */
                eport->type = 0X04;                         /* eport��겻��˸  */
                smemo->type = 0X00;                         /* smemo���ɱ༭,��겻��˸ */
            }
            if(eport->top<in_obj.y&&in_obj.y<(eport->top+eport->height)&&(eport->left<in_obj.x)&&in_obj.x<(eport->left+eport->width))//��eport���ڲ� 
            {
                editflag = 2;                               /* �༭����eport */
                memo_show_cursor(smemo,0);                  /* �ر�smemo�Ĺ�� */
                edit_show_cursor(eip,0);                    /* �ر�eip�Ĺ�� */
                eport->type = 0X06;                         /* eport�����˸  */
                eip->type = 0X04;                           /* eip��겻��˸  */
                smemo->type = 0X00;                         /* smemo���ɱ༭,��겻��˸ */
            }
        }
        
        edit_check(eip,&in_obj);
        edit_check(eport,&in_obj);
        t9_check(t9,&in_obj);
        memo_check(smemo,&in_obj);
        memo_check(rmemo,&in_obj);          /* ���rmemo */
        
        if(t9->outstr[0] != NULL)           /* ����ַ� */
        {
            if (editflag == 1)               /* eip */
            {
                if((t9->outstr[0]<='9'&&t9->outstr[0]>='0')||t9->outstr[0]=='.'||t9->outstr[0] == 0X08)edit_add_text(eip,t9->outstr);
            }
            else if(editflag == 2)           /* eport */
            {
                if((t9->outstr[0]<='9'&&t9->outstr[0]>='0')||t9->outstr[0]==0X08)edit_add_text(eport,t9->outstr);
            }
            else                           /* smemo */
            {   
                memo_add_text(smemo,t9->outstr);
            }
            
            t9->outstr[0] = NULL;           /* �������ַ� */
        }
        res = btn_check(protbtn,&in_obj);   
        if (res && ((protbtn->sta & ( 1 << 7)) == 0) && (protbtn->sta & (1 << 6)))/* ������,�а����������ɿ�,����TP�ɿ��� */
        {  
            /* ��ѡ��ģʽ */
            tempx = protocol;
            app_items_sel((lcddev.width-180)/2,(lcddev.height-192)/2,180,72+40*3,(uint8_t**)netplay_mode_tbl,3,(uint8_t*)&tempx,0XD0,(uint8_t*)netplay_btncaption_tbl[0][gui_phy.language]);/* 3��ѡ�� */
            
            if(tempx != protocol)
            {
                protocol = tempx;  
                
                if(protocol != 0)ipcaption = netplay_ipcaption_tb[0][gui_phy.language];/* TCP Client/UDPģʽ,��ʾĿ��IP */
                else 
                { 
                    sprintf((char*)ptemp,"%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
                    strcpy((char*)eip->text,(const char *)ptemp);       /* �ָ�Ĭ��IP��ַ  */
                    ipcaption=netplay_ipcaption_tb[1][gui_phy.language];/* Ĭ����TCP Server/UDPģʽ,��ʾ����IP   */
                }
                
                tempx = (lcddev.width-35*ip_fsize/2)/3;
                gui_fill_rectangle(tempx,(ip_height-ip_fsize)/2,ip_fsize*strlen((char*)ipcaption)/2,ip_fsize,NET_IP_BACK_COLOR);/* ���ԭ������ʾ */
                gui_show_string(ipcaption,tempx,(ip_height-ip_fsize)/2,lcddev.width,ip_fsize,ip_fsize,WHITE);/* ����IP/Ŀ��IP */
                net_edit_colorset(eip,eport,protocol,connstatus);/* �ػ�edit��  */
                net_msg_show(ip_height,msg_height,fsize,txcnt,rxcnt,protocol,1<<2);/* ����prot��Ϣ */
            }
        } 
        
        res = btn_check(connbtn,&in_obj);   
        
        if(res && ((connbtn->sta & (1 << 7)) == 0) && (connbtn->sta & (1 << 6)))/* ������,�а����������ɿ�,����TP�ɿ��� */
        {   
            connstatus =! connstatus;
            
            tcpconn = 0;        /* ���TCP����δ���� */
            
            if(connstatus == 1)/* �������� */
            {
                bkcolor = gui_memex_malloc(200 * 80 * 2);/* �����ڴ� */
                
                if(bkcolor == NULL)/* ��ȡ����ɫʧ����,ֱ�Ӽ�������,��ִ�к������� */
                {
                    connstatus = 0;
                    printf("netplay ex outof memory\r\n");
                    continue;
                }
                
                app_read_bkcolor((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,bkcolor);/* ��ȡ����ɫ */
                window_msg_box((lcddev.width - 200) / 2,(lcddev.height-80)/2,200,80,(uint8_t*)netplay_connmsg_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);/* ��ʾ�������� */
                
                switch(protocol)
                {
                    case 0:/* TCP ServerЭ�� */
                        tport = net_get_port(eport->text);          /* �õ�port�� */
                        netconnnew = netconn_new(NETCONN_TCP);      /* ����һ��TCP���� */
                        netconnnew->recv_timeout=10;                /* ��ֹ�����߳� */
                        err = netconn_bind(netconnnew,IP_ADDR_ANY,tport);   /* �󶨶˿� */
                    
                        if(err == ERR_OK)err = netconn_listen(netconnnew);  /* �������ģʽ */
                        else
                        {
                            connstatus = 0; /* ����ʧ�� */
                            net_disconnect(netconnnew,NULL);    /* �ر����� */
                        }
                        break;
                    case 1: /* TCP ClientЭ�� */
                        tipaddr.addr = net_get_ip(eip->text);
                        if(tipaddr.addr!=0)
                        {
                            netconncom = netconn_new(NETCONN_TCP); /* ����һ��TCP���� */
                            netconncom->recv_timeout=10;
                            tport = net_get_port(eport->text); 
                            err = netconn_connect(netconncom,&tipaddr,tport);   /* ���ӷ����� */
                            
                            if(err == ERR_OK)tcpconn=1; /* ���ӳɹ� */
                            else
                            {
                                connstatus=0;   /* ����ʧ�� */
                                net_disconnect(netconncom,NULL);    /* �ر����� */
                            }
                        } 
                        break;
                    case 2:/* UDPЭ�� */
                        tipaddr.addr = net_get_ip(eip->text);
                        if(tipaddr.addr!=0)
                        {
                            netconncom = netconn_new(NETCONN_UDP);  /* ����һ��UDP���� */
                            netconncom->recv_timeout = 10;          /* ���ճ�ʱ���� */
                            tport = net_get_port(eport->text); 
                            err = netconn_bind(netconncom,IP_ADDR_ANY,tport);   /* ��UDP_PORT�˿� */
                            if(err == ERR_OK)err = netconn_connect(netconncom,&tipaddr,tport);/* ���ӵ�Զ�������˿� */
                            if(err != ERR_OK)/* ����ʧ�� */
                            { 
                                connstatus = 0;/* ����ʧ�� */
                                net_disconnect(netconncom,NULL);/* �ر����� */
                            } 
                        }
                        break;
                } 
                
                if(err == ERR_OK)window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)netplay_connmsg_tbl[2][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);/* ��ʾ���ӳɹ� */
                else window_msg_box((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,(uint8_t*)netplay_connmsg_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);/* ��ʾ����ʧ�� */
                
                delay_ms(800);/* ��ʱ�ȴ���ʾ */
                app_recover_bkcolor((lcddev.width - 200) / 2,(lcddev.height - 80) / 2,200,80,bkcolor);/* �ָ�����ɫ */
                gui_memex_free(bkcolor);/* �ͷ��ڴ� */
            }
        } 
        
        res = btn_check(clrbtn,&in_obj);   
        if(res && ((clrbtn->sta & (1 << 7)) == 0) && (clrbtn->sta & (1 << 6)))/* ������,�а����������ɿ�,����TP�ɿ��� */
        {   
            rxcnt = 0;/* ������������ */
            txcnt = 0;/* ������������ */
            rmemo->text[0] = 0;/* ���rmemo,��ͷ��ʼ */
            memo_draw_memo(rmemo,1);/* �ػ�rmemo */
            net_msg_show(ip_height,msg_height,fsize,txcnt,rxcnt,protocol,0X07);/* ����������Ϣ  */
        } 
        res = btn_check(sendbtn,&in_obj);   
        
        if(res && ((sendbtn->sta & ( 1 << 7)) == 0) && (sendbtn->sta & (1 << 6)))/* ������,�а����������ɿ�,����TP�ɿ��� */
        {  
            tempx=strlen((char*)smemo->text);/* ���������ݲŷ��� */
            
            if(connstatus == 1 && tempx)/* ������,������OK */
            {
                if(tcpconn == 1 && protocol!=2)/* TCP Client/TCP Server�������� */
                { 
                    err = netconn_write(netconncom ,smemo->text,tempx,NETCONN_COPY);/* ����smemo->text�е�����  */
                    
                    if(err == ERR_OK)/* ���ͳɹ� */
                    {
                        txcnt += strlen((char*)smemo->text);/* �ܷ��ͳ������� */
                        net_msg_show(ip_height,msg_height,fsize,txcnt,rxcnt,protocol,1<<0);/* ����TX��Ϣ  */
                    }
                }
                else
                {
                    sendbuf = netbuf_new();
                    netbuf_alloc(sendbuf,strlen((char *)smemo->text));
                    strcpy(sendbuf->p->payload,(void*)smemo->text);/* �������ݵ�sendbuf���� */
                    err = netconn_send(netconncom,sendbuf);/* ��netbuf�е����ݷ��ͳ�ȥ */
                    
                    if(err != ERR_OK)printf("netconn_send fail\r\n"); 
                    else 
                    {
                        txcnt += strlen((char*)smemo->text);/* �ܷ��ͳ������� */
                        net_msg_show(ip_height,msg_height,fsize,txcnt,rxcnt,protocol,1 << 0);/* ����TX��Ϣ */
                    }
                    netbuf_delete(sendbuf);  /* ɾ��buf */
                }
            }
        } 
        if (connstatus == 1)/* ����״̬ */
        {
            if (tcpconn == 0 && protocol == 0)/* TCP Serverģʽ��,���ӻ�δ����,���TCP���� */
            {
                err = netconn_accept(netconnnew,&netconncom);/* ������������ */
                if(err == ERR_OK)/* �ɹ���⵽���� */
                { 
                    netconncom->recv_timeout=10; 
                    tcpconn = 1;
                }
            }
            else
            {
                /* ������հ� */
                err = netconn_recv(netconncom,&recvbuf);/* �鿴�Ƿ���յ����� */
                
                if (err == ERR_OK)  /* ���յ����� */
                {
                    netconn_getaddr(netconncom,&tipaddr,&tport,0); /* ��ȡԶ��IP��ַ�Ͷ˿ں� */
                    
                    if(tipaddr.addr != oldaddr || tport != oldport)/* �µ�ַ/�˿ں� */
                    {
                        oldaddr = tipaddr.addr;
                        oldport = tport;
                        sprintf((char*)ptemp,"[From:%d.%d.%d.%d:%d]:\r\n",oldaddr&0XFF,(oldaddr>>8)&0XFF,(oldaddr>>16)&0XFF,(oldaddr>>24)&0XFF,oldport); 
                        tempx=strlen((char*)rmemo->text)+strlen((char*)ptemp);/* �õ��µ��ܳ��� */
                        if(tempx > NET_RMEMO_MAXLEN)rmemo->text[0] = 0;/* ���rmemo,��ͷ��ʼ */
                        
                        strcat(((char*)rmemo->text),(char*)ptemp);/* ����յ������� */
                    }
                    
                    memcpy(p,recvbuf->p->payload,recvbuf->p->tot_len);
                    p[recvbuf->p->tot_len] = 0;/* ĩβ��������� */
                    tempx = strlen((char*)rmemo->text) + strlen((char*)p);/* �õ��µ��ܳ��� */
                    if(tempx > NET_RMEMO_MAXLEN)rmemo->text[0] = 0;/* ���rmemo,��ͷ��ʼ */
                    strcat(((char*)rmemo->text),(char*)p);/* ����յ������� */
                    rxcnt+=strlen((char*)p);/* �ܽ��ճ������� */
                    memo_draw_memo(rmemo,1);/* �ػ�rmemo */
                    net_msg_show(ip_height,msg_height,fsize,txcnt,rxcnt,protocol,1<<1);/* ����RX��Ϣ  */
                    netbuf_delete(recvbuf);
                }
                else if(err == ERR_CLSD)
                {
                    if(protocol == 0)tcpconn = 0;/* �������ӶϿ�״̬ */
                    else connstatus = 0;
                    net_disconnect(netconncom,NULL);/* �Ͽ�netconncom���� */
                } 
            }
        }
        if (oldconnstatus != connstatus)/* ����״̬�ı��� */
        {
            oldconnstatus = connstatus;
            
            if(connstatus == 0)/* ���ӶϿ���?ǿ�ƶϿ�����? */
            {
                net_disconnect(netconnnew,netconncom);/* �Ͽ����� */
                netconncom = NULL;
                netconnnew = NULL; 
                if(protocol == 0)net_tcpserver_remove_timewait();/* TCP Server,ɾ���ȴ�״̬ */
                protbtn->sta = 0;/* Э��ѡ��ť���뼤��״̬ */
                connbtn->caption = netplay_btncaption_tbl[1][gui_phy.language];  			
            }
            else/* ���ӳɹ� */
            {
                protbtn->sta = 2;           /* Э��ѡ��ť����Ǽ���״̬ */
                connbtn->caption = netplay_btncaption_tbl[2][gui_phy.language]; 
                editflag = 0;               /* ֻ����༭smemo */
                edit_show_cursor(eip,0);    /* �ر�edit�Ĺ�� */
                edit_show_cursor(eport,0);  /* �ر�eport�Ĺ�� */
                eip->type = 0X04;           /* eip��겻��˸  */
                eport->type = 0X04;         /* eport��겻��˸  */
                smemo->type = 0X01;         /* memo�ɱ༭,��˸��� */
            }
            btn_draw(protbtn);              /* �ػ���ť */
            btn_draw(connbtn);
            net_edit_colorset(eip,eport,protocol,connstatus);   /* �ػ�edit�� */
        }
    } 
    if (connstatus) /* ����״̬�˳�?�Ͽ�����! */
    {
        net_disconnect(netconnnew,netconncom);      /* �Ͽ����� */
        if (protocol == 0)net_tcpserver_remove_timewait();  /* TCP Server,ɾ���ȴ�״̬ */
    }
    gui_memin_free(ptemp); 
    gui_memin_free(p); 
    edit_delete(eip);
    edit_delete(eport);
    memo_delete(rmemo);
    memo_delete(smemo);
    t9_delete(t9);
    btn_delete(protbtn);
    btn_delete(connbtn);
    btn_delete(clrbtn);
    btn_delete(sendbtn);
    system_task_return = 0;
    return 0;
} 

/**
 * @brief       ����ͨ�Ų���
 * @param       ��
 * @retval      ��
 */
uint8_t net_play(void)
{
    uint8_t res;
    uint16_t yoff = 0;
    _btn_obj* tbtn = 0;                 /* ���԰�ť�ؼ� */
    
    if (lcddev.width == 240) yoff = 170;
    else yoff = 210; 
    
    tbtn = btn_creat((lcddev.width - 180) / 2,yoff+(lcddev.height - yoff - 60) / 2,180,60,0,0);//������׼��ť */
    
    if (tbtn == NULL)return 1;          /* ��������ʧ�� */
    
    if (g_audiodev.status & (1 << 7))   /* ��ǰ�ڷŸ�?? */
    {
        audio_stop_req(&g_audiodev);    /* ֹͣ��Ƶ���� */
        audio_task_delete();            /* ɾ�����ֲ������� */
    } 
    
    window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[0][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
    res = lwip_comm_init();             /* lwip��ʼ�� LwIP_Initһ��Ҫ��OSInit֮�������LWIP�̴߳���֮ǰ��ʼ��!!!!!!!! */
    
    if (res == 0)                        /* ������ʼ���ɹ� */
    {
        lwip_comm_dhcp_creat();         /* ����DHCP���� */
        /* ��ʾ����DHCP��ȡIP */
        window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[2][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        
        while(lwipdev.dhcpstatus == 0 || lwipdev.dhcpstatus == 1)/* �ȴ�DHCP����ɹ� */
        {
            delay_ms(10);/* �ȴ� */
        }
        if (lwipdev.dhcpstatus==2)window_msg_box((lcddev.width-220)/2,(lcddev.height-100)/2,220,100,(uint8_t*)netplay_remindmsg_tbl[3][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);/* DHCP�ɹ� */
        else window_msg_box((lcddev.width-220)/2,(lcddev.height-100)/2,220,100,(uint8_t*)netplay_remindmsg_tbl[4][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);/* DHCPʧ�� */
        
        delay_ms(1000);
        tbtn->caption = netplay_tbtncaption_tb[gui_phy.language];
        tbtn->font = 24;                /* 24������ */
        net_load_ui();                  /* ����������UI */
        btn_draw(tbtn);                 /* ������ť */
        httpd_init();                   /* ��ʼ��http */
        
        while(1)
        {
            tp_dev.scan(0);    
            in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);  /* �õ�������ֵ */
            delay_ms(5);                            /* ��ʱһ��ʱ�ӽ��� */
           
            if (system_task_return)
            {
                delay_ms(15);
                if (tpad_scan(1))break;             /* TPAD����,�ٴ�ȷ��,�ų����� */
                else system_task_return = 0;
            }
            res = btn_check(tbtn,&in_obj);
            
            if(res && ((tbtn->sta & (1 << 7)) == 0) && (tbtn->sta & (1 << 6)))  /* �а����������ɿ�,����TP�ɿ��� */
            {
                net_test();
                net_load_ui();                      /* ����������UI */
                btn_draw(tbtn);                     /* �ػ���ť */
            }
        }
    }
    else                                            /* ��ʾ������ʼ��ʧ��! */
    {
        window_msg_box((lcddev.width - 220) / 2,(lcddev.height - 100) / 2,220,100,(uint8_t*)netplay_remindmsg_tbl[1][gui_phy.language],(uint8_t*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0,0);
        delay_ms(2000);
    } 
    
    system_task_return = 0;
    lwip_comm_destroy(); 
    pcf8574_write_bit(ETH_RESET_IO,1);              /* ���ָ�λLAN8720,���͹��� */
    return 0;
} 
