/**
 ****************************************************************************************************
 * @file        mpu.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       MPU�ڴ汣�� ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230321
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __MPU_H
#define __MPU_H

#include "./SYSTEM/sys/sys.h"


/* MPU����ϸ���ù�ϵ���뿴����STM32H7����ֲ�.pdf��
 * ����ĵ���6.6��,Table 91.
 * MPU��������������Զ��壨������stm32h7xx_hal_cortex.h��
 * ����MPU->RASR�Ĵ���AP[26:24]λ������ֵ
 */ 
#define  MPU_REGION_NO_ACCESS       ((uint8_t)0x00U)        /* �޷��ʣ���Ȩ&�û������ɷ��ʣ� */
#define  MPU_REGION_PRIV_RW         ((uint8_t)0x01U)        /* ��֧����Ȩ��д���� */
#define  MPU_REGION_PRIV_RW_URO     ((uint8_t)0x02U)        /* ��ֹ�û�д���ʣ���Ȩ�ɶ�д���ʣ� */
#define  MPU_REGION_FULL_ACCESS     ((uint8_t)0x03U)        /* ȫ���ʣ���Ȩ&�û����ɷ��ʣ� */
#define  MPU_REGION_PRIV_RO         ((uint8_t)0x05U)        /* ��֧����Ȩ������ */
#define  MPU_REGION_PRIV_RO_URO     ((uint8_t)0x06U)        /* ֻ������Ȩ&�û���������д�� */

static uint8_t mpu_convert_bytes_to_pot(uint32_t nbytes);   /* ��nbytesת��Ϊ2Ϊ�ڵ�ָ�� */

void mpu_disable(void); /* ��ֹMPU */
void mpu_enable(void);  /* ʹ��MPU */
void mpu_memory_protection(void);   /* MPU���� */
uint8_t mpu_set_protection(uint32_t baseaddr, uint32_t size, uint32_t rnum, uint8_t de, uint8_t ap, uint8_t sen, uint8_t cen, uint8_t ben);

#endif

















