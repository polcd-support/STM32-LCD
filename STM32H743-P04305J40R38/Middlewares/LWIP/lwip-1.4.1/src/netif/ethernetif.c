/**
 ****************************************************************************************************
 * @file        ethernetif.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-05
 * @brief       APPͨ�� ����
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
 * V1.0 20221205
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "netif/ethernetif.h" 
#include "./BSP/ETHERNET/ethernet_chip.h"
#include "lwip_comm.h" 
#include "netif/etharp.h"  
#include "string.h"  



ETH_TxPacketConfig TxConfig; 
uint32_t current_pbuf_idx =0;
struct pbuf_custom rx_pbuf[ETH_RX_DESC_CNT];
void pbuf_free_custom(struct pbuf *p);

extern eth_chip_object_t ETHCHIP;


/**
 * @brief       ��ethernetif_init()�������ڳ�ʼ��Ӳ��
 * @param       netif           : �����ṹ��ָ��
 * @retval      ERR_OK,����; ����,ʧ��;
 */
static void low_level_init(struct netif *netif)
{ 
    uint32_t idx = 0;
    ETH_MACConfigTypeDef MACConf;
    uint32_t phy_link_state;
    uint32_t speed=0,duplex=0; 
    
    netif->hwaddr_len=ETHARP_HWADDR_LEN;  /* ����MAC��ַ����,Ϊ6���ֽ� */
    /* ��ʼ��MAC��ַ,����ʲô��ַ���û��Լ�����,���ǲ����������������豸MAC��ַ�ظ� */
    netif->hwaddr[0] = lwipdev.mac[0]; 
    netif->hwaddr[1] = lwipdev.mac[1]; 
    netif->hwaddr[2] = lwipdev.mac[2];
    netif->hwaddr[3] = lwipdev.mac[3];   
    netif->hwaddr[4] = lwipdev.mac[4];
    netif->hwaddr[5] = lwipdev.mac[5];
    netif->mtu=ETH_MAX_PAYLOAD;/* ��������䵥Ԫ,����������㲥��ARP���� */
  
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    
    for (idx = 0;idx < ETH_RX_DESC_CNT;idx ++)
    {
        HAL_ETH_DescAssignMemory(&g_eth_handler,idx,ETH_Rx_Buff[idx],NULL);
        rx_pbuf[idx].custom_free_function = pbuf_free_custom;
    }
  
    memset(&TxConfig,0,sizeof(ETH_TxPacketConfig));
    TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM|ETH_TX_PACKETS_FEATURES_CRCPAD;
    TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
    
    phy_link_state = eth_chip_get_link_state(&ETHCHIP);
    
    switch (phy_link_state)
    {
        case ETH_CHIP_STATUS_100MBITS_FULLDUPLEX:    /* 100Mȫ˫�� */
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            break;
        case ETH_CHIP_STATUS_100MBITS_HALFDUPLEX:    /* 100M��˫�� */
            duplex = ETH_HALFDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            break;
        case ETH_CHIP_STATUS_10MBITS_FULLDUPLEX:     /* 10Mȫ˫�� */
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_10M;
            break;
        case ETH_CHIP_STATUS_10MBITS_HALFDUPLEX:     /* 10M��˫�� */
            duplex = ETH_HALFDUPLEX_MODE;
            speed = ETH_SPEED_10M;
            break;
        default:
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            break;     
    }
    
    HAL_ETH_GetMACConfig(&g_eth_handler,&MACConf); 
    MACConf.DuplexMode = duplex;
    MACConf.Speed = speed;
    HAL_ETH_SetMACConfig(&g_eth_handler,&MACConf);  /* ����MAC */
    HAL_ETH_Start_IT(&g_eth_handler);
    /* ������������ */
    netif_set_up(netif);
    netif_set_link_up(netif);
    
    while (!ethernet_read_phy(ETH_CHIP_PHYSCSR))  /* ���MCU��PHYоƬ�Ƿ�ͨ�ųɹ� */
    {
        printf("MCU��PHYоƬͨ��ʧ�ܣ������·����Դ�룡������\r\n");
    }
}

/**
 * @brief       ���ڷ������ݰ�����ײ㺯��(lwipͨ��netif->linkoutputָ��ú���)
 * @param       netif           : �����ṹ��ָ��
 * @param       p               : pbuf���ݽṹ��ָ��
 * @retval      ERR_OK,��������; ERR_MEM,����ʧ��;
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    uint32_t i=0, framelen = 0;
    struct pbuf *q;
    err_t errval = ERR_OK;
    ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT];
    
    for( q = p;q != NULL;q = q->next)
    {
        if (i >= ETH_TX_DESC_CNT) return ERR_IF;
        
        Txbuffer[i].buffer = q->payload;
        Txbuffer[i].len = q->len;
        framelen += q->len;
        
        if (i > 0) Txbuffer[i - 1].next = &Txbuffer[i];
        
        if (q->next == NULL) Txbuffer[i].next = NULL;
        
        i++;
    }
    
    TxConfig.Length = framelen;
    TxConfig.TxBuffer = Txbuffer; 
    SCB_CleanInvalidateDCache();    /* ��Ч�������Dcache */
    errval = HAL_ETH_Transmit(&g_eth_handler,&TxConfig,0);
    
    return errval;
}

/**
 * @brief       ���ڽ������ݰ�����ײ㺯��
 * @param       netif           : �����ṹ��ָ��
 * @retval      pbuf���ݽṹ��ָ��
 */
static struct pbuf * low_level_input(struct netif *netif)
{
    struct pbuf *p = NULL;
    ETH_BufferTypeDef RxBuff;
    uint32_t framelength = 0;
    
    if (HAL_ETH_IsRxDataAvailable(&g_eth_handler))
    {
        SCB_CleanInvalidateDCache();/* ��Ч�������Dcache */
        HAL_ETH_GetRxDataBuffer(&g_eth_handler,&RxBuff);
        HAL_ETH_GetRxDataLength(&g_eth_handler,&framelength);
        p = pbuf_alloced_custom(PBUF_RAW,framelength,PBUF_POOL,&rx_pbuf[current_pbuf_idx],RxBuff.buffer, framelength); 
        
        if (current_pbuf_idx < (ETH_RX_DESC_CNT-1)) current_pbuf_idx ++;
        else current_pbuf_idx = 0; 
        
        return p;
    }
    else return NULL; 
}

/**
 * @brief       ������������(lwipֱ�ӵ���)
 * @param       netif           : �����ṹ��ָ��
 * @retval      ERR_OK,��������; ERR_MEM,����ʧ��;
 */
err_t ethernetif_input(struct netif *netif)
{
    err_t err;
    struct pbuf *p; 
    p = low_level_input(netif);       /* ����low_level_input������������ */
    
    if (p == NULL) return ERR_MEM;
    
    err = netif->input(p, netif);     /* ����netif�ṹ���е�input�ֶ�(һ������)���������ݰ� */
    
    if (err != ERR_OK)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
        pbuf_free(p);
        p = NULL;
    }
    
    HAL_ETH_BuildRxDescriptors(&g_eth_handler);
    return err;
}

/**
 * @brief       ʹ��low_level_init()��������ʼ������
 * @param       netif           : �����ṹ��ָ��
 * @retval      ERR_OK,��������; ERR_MEM,����ʧ��;
 */
err_t ethernetif_init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));
  
#if LWIP_NETIF_HOSTNAME             /* LWIP_NETIF_HOSTNAME */
    netif->hostname="lwip";         /* ��ʼ������ */
#endif

    netif->name[0] = IFNAME0;       /* ��ʼ������netif��name�ֶ� */
    netif->name[1] = IFNAME1;       /* ���ļ��ⶨ�����ﲻ�ù��ľ���ֵ */
    netif->output = etharp_output;  /* IP�㷢�����ݰ����� */
    netif->linkoutput = low_level_output;   /* ARPģ�鷢�����ݰ����� */
    low_level_init(netif);          /* �ײ�Ӳ����ʼ������ */
    
    return ERR_OK;
}

/**
 * @brief       �û��Զ���Ľ���pbuf�ͷź���
 * @param       p           : Ҫ�ͷŵ�pbuf
 * @retval      ��
 */
void pbuf_free_custom(struct pbuf *p)
{
    if(p != NULL)
    {
        p->flags = 0;
        p->next = NULL;
        p->len = p->tot_len = 0;
        p->ref = 0;
        p->payload = NULL;
    }
}
 