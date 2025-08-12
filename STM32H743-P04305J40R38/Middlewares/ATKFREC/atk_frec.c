/**
 ****************************************************************************************************
 * @file        atk_frec.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-06-02
 * @brief       ����ʶ��� ����
 *              ������ʶ�������ALIENTEK�ṩ,�����ṩ2��LIB(��:ATKFREC_DF.lib��ATKFREC_SF.lib),�����ʹ��
 *              ATKFREC_DF.lib:�ʺϾ���˫����Ӳ��������㵥Ԫ��STM32F7ϵ�У�����STM32F767/769��
 *              ATKFREC_SF.lib:�ʺϾ��е�����Ӳ��������㵥Ԫ��STM32F4/F7ϵ�У�����STM32F407/429/746��
 *              ��ѡ����ʵ�lib���в��ԣ�
 *  
 *              ����:��������ͷ,ʵ������ʶ��.
 *              ˵��:��ʶ���,��Ҫ�õ��ڴ����,�ڴ���ռ������560KB����(20������).ÿ����һ������,�ڴ�ռ����10KB����.
 *              ����:���ڱ�ʶ�����M3/M4/M7ΪĿ�괦����,�ڴ�����,�㷨�Ͻ����˴����˸�,����,�ܶ๦�ܲ�̫����,Ч��Ҳ��
 *                   �Ǻܺ�.��û����ʶ����Ч���(������������,Ҳ���н�����).����,�δ���,������Ҳο���.
 *              
 *              ��������:
 *              1,����ͷģ��һ��.
 *              2,SD��һ��
 *              
 *              ʹ�÷���:
 *              ��һ��:����atk_frec_initialization����,��ʼ������ʶ���
 *              �ڶ���:����atk_frec_add_a_face����,�������ģ��(����Ѿ�����,���Ժ��Դβ�)
 *              ������:����atk_frec_load_data_model����,��������ģ�嵽�ڴ�����(���������ģ�����Ҫ,��û�������ģ��,��ɺ��Դ˲�)
 *              ���Ĳ�:����atk_frec_recognition_face����,��ȡʶ����.
 *              ���岽:����atk_frec_delete_data����,����ɾ��һ������ģ��
 *              ������:�����������ʶ���,�����atk_frec_destroy����,�ͷ������ڴ�,��������ʶ��.
 *
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
 * V1.1 20220602
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "atk_frec.h"
#include "stdio.h"
#include "./MALLOC/malloc.h"
#include "./FATFS/source/ff.h"


/**
 * @brief       �ڴ����ú���
 * @param       p               : �ڴ��׵�ַ
 * @param       c               : Ҫ���õ�ֵ
 * @param       len             : ��Ҫ���õ��ڴ��С(�ֽ�Ϊ��λ)
 * @retval      ��
 */
void atk_frec_memset(char *p, char c, unsigned long len)
{
    my_mem_set((uint8_t *)p, (uint8_t)c, (uint32_t)len);
}

/**
 * @brief       �ڴ����뺯��
 * @param       size            : ������ڴ��С
 * @retval      0,ʧ��; ����, �ڴ��׵�ַ;
 */
void *atk_frec_malloc(unsigned int size)
{
    return mymalloc(SRAMEX, size);;
}

/**
 * @brief       �ڴ��ͷź���
 * @param       ptr             : �ڴ��׵�ַ
 * @retval      ��
 */
void atk_frec_free(void *ptr)
{
    myfree(SRAMEX, ptr);
}

/**
 * @brief       �ڴ�������뺯��
 * @param       size            : ������ڴ��С
 * @retval      0,ʧ��; ����, �ڴ��׵�ַ;
 */
void *atk_frec_fastmalloc(unsigned int size)
{
    return mymalloc(SRAMDTCM, size);
}

/**
 * @brief       �ڴ�����ͷź���
 * @param       ptr             : �ڴ��׵�ַ
 * @retval      ��
 */
void atk_frec_fastfree(void *ptr)
{
    myfree(SRAMDTCM, ptr);
}

/**
 * @brief       ��������ʶ�����������
 * @param       index           : Ҫ���������λ��(һ����ռһ��λ��),��Χ:0~MAX_LEBEL_NUM-1
 * @param       buf             : Ҫ��������ݻ������׵�ַ
 * @param       size            : Ҫ��������ݴ�С
 * @retval      0, �ɹ�; ����, ȡ������;
 */
uint8_t atk_frec_save_data(uint8_t index, uint8_t *buf, uint32_t size)
{
    uint8_t *path;
    FIL *fp;
    DIR fdir;
    uint32_t fw;
    uint8_t res;
    path = atk_frec_fastmalloc(30);         /* �����ڴ� */
    fp = atk_frec_fastmalloc(sizeof(FIL));  /* �����ڴ� */

    if (!fp)
    {
        atk_frec_fastfree(path);
        return ATK_FREC_MEMORY_ERR;
    }

    sprintf((char *)path, ATK_FREC_DATA_PNAME, index);
    res = f_opendir(&fdir, (const TCHAR *)ATK_FREC_DATA_PDIR); /* ���Դ�ATK_FREC_DATA_PDIRĿ¼ */

    if (res)    /* ��ʧ�� */
    {
        f_mkdir(ATK_FREC_DATA_PDIR);    /* �����ļ��� */
    }
    else
    {
        f_closedir(&fdir);  /* �ر�dir */
    }

    res = f_open(fp, (char *)path, FA_WRITE | FA_CREATE_NEW);

    if (res == FR_OK)
    {
        res = f_write(fp, buf, size, &fw);  /* д���ļ� */
    }

    f_close(fp);

    if (res)res = ATK_FREC_READ_WRITE_ERR;

    atk_frec_fastfree(path);
    atk_frec_fastfree(fp);
    return res;
}

/**
 * @brief       ��ȡ����ʶ�����������
 * @param       index           : Ҫ��ȡ������λ��(һ����ռһ��λ��),��Χ:0~MAX_LEBEL_NUM-1
 * @param       buf             : Ҫ��ȡ�����ݻ������׵�ַ
 * @param       size            : Ҫ��ȡ�����ݴ�С
 * @retval      0, �ɹ�; ����, ȡ������;
 */
uint8_t atk_frec_read_data(uint8_t index, uint8_t *buf, uint32_t size)
{
    uint8_t *path;
    FIL *fp;
    uint32_t fr;
    uint8_t res;
    path = atk_frec_fastmalloc(30);         /* �����ڴ� */
    fp = atk_frec_fastmalloc(sizeof(FIL));  /* �����ڴ� */

    if (!fp)
    {
        atk_frec_fastfree(path);
        return ATK_FREC_MEMORY_ERR;
    }

    sprintf((char *)path, ATK_FREC_DATA_PNAME, index);
    res = f_open(fp, (char *)path, FA_READ);

    if (res == FR_OK && size)
    {
        res = f_read(fp, buf, size, &fr);   /* ��ȡ�ļ� */

        if (fr == size)res = 0;
        else res = ATK_FREC_READ_WRITE_ERR;
    }

    f_close(fp);

    if (res)res = ATK_FREC_READ_WRITE_ERR;

    atk_frec_fastfree(path);
    atk_frec_fastfree(fp);
    return res;
}

/**
 * @brief       ɾ��һ����������
 * @param       index           : Ҫɾ��������λ��(һ����ռһ��λ��),��Χ:0~MAX_LEBEL_NUM-1
 * @retval      0, �ɹ�; ����, ȡ������;
 */
uint8_t atk_frec_delete_data(uint8_t index)
{
    uint8_t *path;
    uint8_t res;
    path = atk_frec_fastmalloc(30); /* �����ڴ� */

    if (!path)
    {
        return ATK_FREC_MEMORY_ERR;
    }

    sprintf((char *)path, ATK_FREC_DATA_PNAME, index);
    res = f_unlink((char *)path);
    atk_frec_fastfree(path);
    return res;
}





