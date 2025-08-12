/**
 ****************************************************************************************************
 * @file        usbh_diskio.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-01-06
 * @brief       usbh diskio ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230106
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __USBH_DISKIO_H
#define __USBH_DISKIO_H

#include "usbh_core.h"
#include "usbh_msc.h"
#include "./FATFS/source/diskio.h"



#define USB_DEFAULT_BLOCK_SIZE      512


DSTATUS USBH_initialize(void);
DSTATUS USBH_status(void);
DRESULT USBH_read(BYTE *buff, DWORD sector, UINT count);
DRESULT USBH_write(const BYTE *buff,DWORD sector,UINT count);
DRESULT USBH_ioctl(BYTE cmd,void *buff);

#endif 
