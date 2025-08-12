/**
 ****************************************************************************************************
 * @file        mpu.c
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

#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/MPU/mpu.h"


/**
 * @brief       ��ֹMPU����
 * @param       ��
 * @retval      ��
 */
void mpu_disable(void)
{
    SCB->SHCSR &= ~(1 << 16);   /* ��ֹMemManage */
    MPU->CTRL &= ~(1 << 0);     /* ��ֹMPU */
}

/**
 * @brief       ����MPU����
 * @param       ��
 * @retval      ��
 */
void mpu_enable(void)
{
    MPU->CTRL = (1 << 2) | (1 << 0);    /* ʹ��PRIVDEFENA,ʹ��MPU */
    SCB->SHCSR |= 1 << 16;              /* ʹ��MemManage */
}

/**
 * @brief       ��ֹMPU����
 * @param       nbytes:Ҫת��������,�ֽ���
 * @retval      nbytes��2Ϊ�׵�ָ��ֵ
 */
static uint8_t mpu_convert_bytes_to_pot(uint32_t nbytes)
{
    uint8_t count = 0;

    while (nbytes != 1)
    {
        nbytes >>= 1;
        count++;
    }

    return count;
}

/**
 * @brief       ����ĳ�������MPU����
 * @param       baseaddr: MPU��������Ļ�ַ(�׵�ַ)
 * @param       size:MPU��������Ĵ�С(������32�ı���,��λΪ�ֽ�)
 * @param       rnum:MPU���������,��Χ:0~7,���֧��8����������
 * @param       de:��ָֹ�����;0,����ָ�����;1,��ָֹ�����
 * @param       ap:����Ȩ��,���ʹ�ϵ����:
 *   @arg       0,�޷��ʣ���Ȩ&�û������ɷ��ʣ�
 *   @arg       1,��֧����Ȩ��д����
 *   @arg       2,��ֹ�û�д���ʣ���Ȩ�ɶ�д���ʣ�
 *   @arg       3,ȫ���ʣ���Ȩ&�û����ɷ��ʣ�
 *   @arg       4,�޷�Ԥ��(��ֹ����Ϊ4!!!)
 *   @arg       5,��֧����Ȩ������
 *   @arg       6,ֻ������Ȩ&�û���������д��
 * @note        ���:STM32F7����ֲ�.pdf,4.6��,Table 89.
 * @param       sen:�Ƿ�������;0,������;1,����
 * @param       cen:�Ƿ�����cache;0,������;1,����
 * @param       ben:�Ƿ�������;0,������;1,����
 * @retval      0, �ɹ�; 1, ����;
 */
uint8_t mpu_set_protection(uint32_t baseaddr, uint32_t size, uint32_t rnum, uint8_t de, uint8_t ap, uint8_t sen, uint8_t cen, uint8_t ben)
{
    uint32_t tempreg = 0;
    uint8_t rnr = 0;

    if ((size % 32) || size == 0)return 1;      /* ��С����32�ı���,����sizeΪ0,˵���������� */

    rnr = mpu_convert_bytes_to_pot(size) - 1;   /* ת��Ϊ2Ϊ�׵�ָ��ֵ */
    mpu_disable();                      /* ����֮ǰ,�Ƚ�ֹMPU���� */
    MPU->RNR = rnum;                    /* ���ñ������� */
    MPU->RBAR = baseaddr;               /* ���û�ַ */
    tempreg |= ((uint32_t)de) << 28;    /* ��ֹ/����ָ�����(��ֹ/�����ȡָ��) */
    tempreg |= ((uint32_t)ap) << 24;    /* ���÷���Ȩ��, */
    tempreg |= 0 << 19;                 /* ����������չ��Ϊlevel0 */
    tempreg |= ((uint32_t)sen) << 18;   /* �Ƿ������� */
    tempreg |= ((uint32_t)cen) << 17;   /* �Ƿ�����cache */
    tempreg |= ((uint32_t)ben) << 16;   /* �Ƿ������� */
    tempreg |= 0 << 8;                  /* ��ֹ������ */
    tempreg |= rnr << 1;                /* ���ñ��������С */
    tempreg |= 1 << 0;                  /* ʹ�ܸñ������� */
    MPU->RASR = tempreg;                /* ����RASR�Ĵ��� */
    mpu_enable();                       /* �������,ʹ��MPU���� */
    return 0;
}
 
/**
 * @brief       ������Ҫ�����Ĵ洢��
 * @note   
 *              ����Բ��ִ洢�������MPU����,������ܵ��³��������쳣
 *              ����MCU������ʾ,����ͷ�ɼ����ݳ���ȵ�����...
 *
 * @param       ��
 * @retval      nbytes��2Ϊ�׵�ָ��ֵ
 */
void mpu_memory_protection(void)
{
    /* ��������DTCM,��128K�ֽ�,����ָ�����,��ֹ����,����cache,������ */
    mpu_set_protection(0x20000000, 128 * 1024, 1, 0, MPU_REGION_FULL_ACCESS, 0, 1, 1);

    /* ��������AXI SRAM,��512K�ֽ�,����ָ�����,��ֹ����,����cache,������ */
    mpu_set_protection(0x24000000, 512 * 1024, 2, 0, MPU_REGION_FULL_ACCESS, 0, 1, 1);

    /* ��������SRAM1~SRAM3,��512K�ֽ�,����ָ�����,��ֹ����,����cache,������ */
    mpu_set_protection(0x30000000, 512 * 1024, 3, 0, MPU_REGION_FULL_ACCESS, 0, 1, 1);

    /* ��������SRAM4,��64K�ֽ�,����ָ�����,��ֹ����,����cache,������ */
    mpu_set_protection(0x38000000, 64 * 1024, 4, 0, MPU_REGION_FULL_ACCESS, 0, 1, 1);

    /* ����MCU LCD�����ڵ�FMC����,��64M�ֽ�,����ָ�����,��ֹ����,��ֹcache,��ֹ���� */
    mpu_set_protection(0x60000000, 64 * 1024 * 1024, 5, 0, MPU_REGION_FULL_ACCESS, 0, 0, 0);
    
    /* ����SDRAM����,��32M�ֽ�,����ָ�����,��ֹ����,����cache,������ */
    mpu_set_protection(0XC0000000, 32 * 1024 * 1024, 6, 0, MPU_REGION_FULL_ACCESS, 0, 0, 0);
    
    /* ��������NAND FLASH����,��256M�ֽ�,��ָֹ�����,��ֹ����,��ֹcache,��ֹ���� */
    mpu_set_protection(0X80000000, 256 * 1024 * 1024, 7, 1, MPU_REGION_FULL_ACCESS, 0, 0, 0);
}

/**
 * @brief       MemManage�������ж�
 * @note        ������ж��Ժ�,���޷��ָ���������!!
 *
 * @param       ��
 * @retval      nbytes��2Ϊ�׵�ָ��ֵ
 */
void MemManage_Handler(void)
{
    LED1(0);                            /* ����LED1(GREEN LED) */
    printf("Mem Access Error!!\r\n");   /* ���������Ϣ */
    delay_ms(1000);
    printf("Soft Reseting...\r\n");     /* ��ʾ������� */
    delay_ms(1000);
    sys_soft_reset();                   /* ��λ */
}














