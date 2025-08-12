/**
 ****************************************************************************************************
 * @file        fs.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-05
 * @brief       lwip fs��������(����lwip��fs.c�޸�,��֧�ִ�SD����ȡ��ҳԴ��͸�����Դ)
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

#include "lwip/opt.h"
#include "lwip/def.h"
#include "fs.h"
#include "spb.h"
#include <string.h>


/* ����Web Server��Դ����·��,�����Լ���ʵ������޸� */
#define HTTP_SRC_PATH  "1:SYSTEM/LWIP/WebServer"

/**
 * �����ļ�,���ߴ�����
 * �������ļ������óߴ��,���ö�ζ�ȡ�ķ�ʽ,������ȡʣ������.
 * ��������ļ��������óߴ�,�����һ�ζ���.
 */
#define HTTP_MAX_FILE_SIZE		100*1024

/* ��������ܴ򿪵��ļ���Ŀ */
#define LWIP_MAX_OPEN_FILES     10

/* �����ļ�ϵͳ�ڴ����ṹ�� */
struct fs_table
{
    struct fs_file file;
    u8_t inuse;  /* 0��ʾδʹ��,1��ʾʹ�� */
};
/* ����һ��fs_tabble����,���а���LWIP_MAX_OPEN_FILES��Ԫ�� */
struct fs_table fs_memory[LWIP_MAX_OPEN_FILES];

/**
 * @brief       �ļ��ڴ����뺯�� ��LWIP_MAX_OPEN_FILES���ļ�ͬʱ�����ڴ�
 * @param       ��
 * @retval      �ڴ��ַ, NULL = ʧ��
 */
static struct fs_file *fs_malloc(void)
{
    int i;

    for (i = 0; i < LWIP_MAX_OPEN_FILES; i++)
    {
        if (fs_memory[i].inuse == 0)
        {
            fs_memory[i].inuse = 1;

            fs_memory[i].file.flwip = mymalloc(SRAMIN, sizeof(FIL)); /* Ϊ�ļ�ָ�������ڴ� */

            if (fs_memory[i].file.flwip)
            {
                return &fs_memory[i].file;
            }
            else return NULL; /* ����ʧ�� */
        }
    }

    return NULL;
}

/**
 * @brief       �ͷ��ڴ�,һ���ͷŵ�LWIP_MAX_OPEN_FILES���������ڴ�
 * @param       file            : fs_fileָ��
 * @retval      ��
 */
static void fs_free(struct fs_file *file)
{
    int i;

    for (i = 0; i < LWIP_MAX_OPEN_FILES; i++)
    {
        if (&fs_memory[i].file == file)
        {
            fs_memory[i].inuse = 0;
            myfree(SRAMIN, file->flwip);    /* �ͷ��ڴ� */
            myfree(SRAMEX, file->data);     /* �ͷ��ڴ� */
            break;
        }
    }

    return;
}

/**
 * @brief       ��һ���ļ�
 * @param       name            : Ҫ�򿪵��ļ���
 * @retval      ��
 */
struct fs_file *fs_open(const char *name)
{
    struct fs_file *file;
    uint8_t *pname = 0;
    uint8_t res;
    uint32_t br;
    uint32_t lenth = 0;
    /* printf("fopen:%s\r\n",name); */
    file = fs_malloc();      /* �����ڴ� */

    if (file == NULL)return NULL;

    pname = mymalloc(SRAMIN, 100);

    if (pname)                                  /* �ڴ�����ɹ� */
    {
        strcpy((char *)pname, HTTP_SRC_PATH);   /* ������WebServerĿ¼·�� */
        strcat((char *)pname, name);            /* �����Ŀ¼ */
        /* printf("open:%s\r\n",pname); */
        res = f_open(file->flwip, (const TCHAR *)pname, FA_READ); /* ���ļ� */

        if (res == FR_OK)
        {
            if (file->flwip->obj.objsize < HTTP_MAX_FILE_SIZE)lenth = file->flwip->obj.objsize; /* lenthΪ�ļ���С */
            else lenth = HTTP_MAX_FILE_SIZE;            /* lenthΪHTTP_MAX_FILE_SIZE��С */

            file->data = mymalloc(SRAMEX, lenth);       /* ����lenth��С�ڴ� */

            if (!file->data)
            {
                spb_delete();                           /* �ͷ�SPBռ�õ��ڴ� */
                file->data = mymalloc(SRAMEX, lenth);   /* ��������lenth��С�ڴ� */
            }

            if (file->data)                             /* ����OK */
            {
                res = f_read(file->flwip, file->data, lenth, &br);

                if (res == FR_OK)
                {
                    file->len = br;                      /* ��ȡ�����ֽ� */
                    file->fleft = file->flwip->obj.objsize - file->flwip->fptr; /* �ļ�ʣ���ֽ��� */
                    file->dataleft = 0;                  /* ��������Ϊ0 */
                    file->dataptr = 0;                   /* ��������ָ������Ϊ0 */
                    file->pextension = NULL;
                    file->http_header_included = 1;      /* ������httpͷ */
                    myfree(SRAMIN, pname);
                    return file;
                }

            }
        }
    }

    if (file->flwip)f_close(file->flwip);   /* �ر��ļ� */

    myfree(SRAMIN, pname);
    fs_free(file);                          /* �ͷ�file�ڴ� */
    return NULL;
}

/**
 * @brief       �ر��ļ�
 * @param       file            : Ҫ�رյ��ļ���
 * @retval      ��
 */
void fs_close(struct fs_file *file)
{
    fs_free(file);  /* �ͷ�file���ڴ� */
}

/**
 * @brief       ��ȡ�ļ�
 * @param       file            : Ҫ��ȡ���ļ�
 * @param       buffer          : ��ȡ�����ŵĻ�����
 * @param       count           : Ҫ��ȡ�ĸ���
 * @retval      ��ȡ����, -1 = ʧ��
 */
int fs_read(struct fs_file *file, char *buffer, int count)
{
    uint8_t res;
    uint32_t br;

    if (file->fleft && (file->dataleft == 0))                           /* ��������,����Ҫ���� */
    {
        res = f_read(file->flwip, file->data, HTTP_MAX_FILE_SIZE, &br); /* ��ȡ20K�ֽ� */

        if (res == FR_OK)          /* ��ȡ�ɹ� */
        {
            file->dataptr = 0;
            file->dataleft = br;   /* ���ݻ�����ʣ���ֽ��� */
        }
        else return -1;            /* ��ȡ���� */
    }

    if (file->dataleft)            /* ����������ʣ������ */
    {
        if (file->dataleft <= count)
        {
            count = file->dataleft;
        }

        memcpy(buffer, file->data + file->dataptr, count);
        file->dataptr += count;
        file->dataleft -= count;
        file->fleft -= count;
        return count;
    }

    return -1;
}

/**
 * @brief       ��ǰ�ļ�ʣ���ֽ���
 * @param       file            : Ҫ��ȡ���ļ� 
 * @retval      ʣ�೤��(�ֽ���)
 */
int fs_bytes_left(struct fs_file *file)
{
    return file->fleft;
}
























