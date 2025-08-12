/**
 ****************************************************************************************************
 * @file        exeplay.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-10-31
 * @brief       APP-������ ����
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
 * V1.1 20221031
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#ifndef __EXEPLAY_H
#define __EXEPLAY_H

#include "common.h"


/** 
    APP�����Ϣ����
    ˼·:��BIN�ļ��ȴ�ŵ��ⲿSRAM,Ȼ�����ñ�־λ,����һ����λ.��λ֮��,ϵͳ�жϱ�־λ,
    �����Ҫ����app,���Ȱѱ�־λ���,Ȼ�����ⲿSDRAM��APP���뵽�ڲ�sram,�����ת��app����
    ʼ��ַ,��ʼ����app����.
    ע��:
    1,Ĭ������APP�ĳߴ����ΪEXEPLAY_APP_SIZE�ֽ�.
    2,APP_SIZE����С��EXEPLAY_APP_SIZE��
 */

#define EXEPLAY_APP_SIZE    450 * 1024  /* app��������ߴ�.����Ϊ146K�ֽ� */
#define EXEPLAY_APP_BASE    0x24001000  /* appִ�д����Ŀ�ĵ�ַ,Ҳ���ǽ�Ҫ���еĴ����ŵĵ�ַ */
#define	EXEPLAY_SRC_BASE    0XC01F4000  /* appִ�д����Դ��ַ,Ҳ������λ֮ǰ,app�����ŵĵ�ַ */


typedef  void (*dummyfun)(void);    /* ����һ���������� */
extern dummyfun jump2app;           /* �ٺ���,��PCָ���ܵ��µ�main����ȥ */
void exeplay_write_appmask(uint16_t val);
void exeplay_app_check(void);
uint8_t exe_play(void);

#endif
































