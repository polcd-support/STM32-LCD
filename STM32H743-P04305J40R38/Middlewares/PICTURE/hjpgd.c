/**
 ****************************************************************************************************
 * @file        hjpgd.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-05-23
 * @brief       ��������-jpegӲ�����벿�� ����
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20220523
 * ��һ�η���
 *
 ****************************************************************************************************
 */
 
#include "./PICTURE/hjpgd.h"
#include "./PICTURE/piclib.h"
#include "./SYSTEM/usart/usart.h"


jpeg_codec_typedef hjpgd;       /* JPEGӲ������ṹ�� */


/**
 * @brief       JPEG�����������ص�����
 *   @note      ���ڻ�ȡJPEG�ļ�ԭʼ����, ÿ��JPEG DMA IN BUFΪ�յ�ʱ��,���øú���
 * @param       ��
 * @retval      ��
 */
void jpeg_dma_in_callback(void)
{
    hjpgd.inbuf[hjpgd.inbuf_read_ptr].sta = 0;  /* ��buf�Ѿ��������� */
    hjpgd.inbuf[hjpgd.inbuf_read_ptr].size = 0; /* ��buf�Ѿ��������� */
    hjpgd.inbuf_read_ptr++;                     /* ָ����һ��buf */

    if (hjpgd.inbuf_read_ptr >= JPEG_DMA_INBUF_NB)hjpgd.inbuf_read_ptr = 0; /* ���� */

    if (hjpgd.inbuf[hjpgd.inbuf_read_ptr].sta == 0)   /* ����Чbuf */
    {
        hjpgd.indma_pause = 1;  /* �����ͣ */
    }
    else    /* ��Ч��buf */
    {
        /* ������һ��DMA���� */
        jpeg_in_dma_resume((uint32_t)hjpgd.inbuf[hjpgd.inbuf_read_ptr].buf, hjpgd.inbuf[hjpgd.inbuf_read_ptr].size);
    }
}

/**
 * @brief       JPEG���������(YCBCR)�ص�����
 *   @note      �������YCbCr������(YUV)
 * @param       ��
 * @retval      ��
 */
void jpeg_dma_out_callback(void)
{
    uint32_t *pdata = 0;
    hjpgd.outbuf[hjpgd.outbuf_write_ptr].sta = 1;   /* ��buf���� */
    hjpgd.outbuf[hjpgd.outbuf_write_ptr].size = hjpgd.yuvblk_size - (MDMA_Channel6->CBNDTR & 0X1FFFF);  /* ��buf�������ݵĳ��� */

    if (hjpgd.state == JPEG_STATE_FINISHED) /* ����ļ��Ѿ��������,��Ҫ��ȡDOR��������(<=32�ֽ�) */
    {
        pdata = (uint32_t *)(hjpgd.outbuf[hjpgd.outbuf_write_ptr].buf + hjpgd.outbuf[hjpgd.outbuf_write_ptr].size);

        while (JPEG->SR & (1 << 4))
        {
            *pdata = JPEG->DOR;
            pdata++;
            hjpgd.outbuf[hjpgd.outbuf_write_ptr].size += 4;
        }
    }

    hjpgd.outbuf_write_ptr++;   /* ָ����һ��buf */

    if (hjpgd.outbuf_write_ptr >= JPEG_DMA_OUTBUF_NB)hjpgd.outbuf_write_ptr = 0;    /* ���� */

    if (hjpgd.outbuf[hjpgd.outbuf_write_ptr].sta == 1)  /* ����Чbuf */
    {
        hjpgd.outdma_pause = 1; /* �����ͣ */
    }
    else    /* ��Ч��buf */
    {
        /* ������һ��DMA���� */
        jpeg_out_dma_resume((uint32_t)hjpgd.outbuf[hjpgd.outbuf_write_ptr].buf, hjpgd.yuvblk_size);

    }
}

/**
 * @brief       JPEG�����ļ�������ɻص�����
 * @param       ��
 * @retval      ��
 */
void jpeg_endofcovert_callback(void)
{
    hjpgd.state = JPEG_STATE_FINISHED;  /* ���JPEG������� */
}

/**
 * @brief       JPEG header�����ɹ��ص�����
 * @param       ��
 * @retval      ��
 */
void jpeg_hdrover_callback(void)
{
    uint8_t i = 0;
    hjpgd.state = JPEG_STATE_HEADEROK;  /* HEADER��ȡ�ɹ� */
    jpeg_get_info(&hjpgd);              /* ��ȡJPEG�����Ϣ,������С,ɫ�ʿռ�,������ */
    picinfo.ImgWidth = hjpgd.Conf.ImageWidth;
    picinfo.ImgHeight = hjpgd.Conf.ImageHeight;

    /* ��Ҫ��ȡJPEG������Ϣ�Ժ�,���ܸ���jpeg�����С�Ͳ�����ʽ,��������������С,���������MDMA */
    switch (hjpgd.Conf.ChromaSubsampling)
    {
        case JPEG_420_SUBSAMPLING:
            hjpgd.yuvblk_size = 24 * hjpgd.Conf.ImageWidth; /* YUV420,ÿ��YUV����ռ1.5���ֽ�.ÿ�����16��.16*1.5=24 */
            hjpgd.yuvblk_height = 16;   /* ÿ�����16�� */
            break;

        case JPEG_422_SUBSAMPLING:
            hjpgd.yuvblk_size = 16 * hjpgd.Conf.ImageWidth; /* YUV422,ÿ��YUV����ռ2���ֽ�.ÿ�����8��.8*2=16 */
            hjpgd.yuvblk_height = 8;    /* ÿ�����8�� */
            break;

        case JPEG_444_SUBSAMPLING:
            hjpgd.yuvblk_size = 24 * hjpgd.Conf.ImageWidth; /* YUV444,ÿ��YUV����ռ3���ֽ�.ÿ�����8��.8*3=24 */
            hjpgd.yuvblk_height = 8;    /* ÿ�����8�� */
            break;
    }

    hjpgd.yuvblk_curheight = 0; /* ��ǰ�м��������� */

    for (i = 0; i < JPEG_DMA_OUTBUF_NB; i++)
    {
        hjpgd.outbuf[i].buf = mymalloc(SRAMIN, hjpgd.yuvblk_size + 32); /* �п��ܻ����Ҫ32�ֽ��ڴ� */

        if (hjpgd.outbuf[i].buf == NULL)
        {
            hjpgd.state = JPEG_STATE_ERROR; /* HEADER��ȡʧ�� */
        }
    }

    if (hjpgd.outbuf[JPEG_DMA_OUTBUF_NB - 1].buf != NULL)   /* ����buf������OK */
    {
        jpeg_out_dma_init((uint32_t)hjpgd.outbuf[0].buf, hjpgd.yuvblk_size);/* �������DMA */
        jpeg_out_dma_start();   /* ����DMA OUT����,��ʼ����JPEG���������� */
    }

    piclib_ai_draw_init();
}

/**
 * @brief       JPEGӲ������ͼƬ
 *   @note      ע��, �뱣֤:
 *              1, ������ͼƬ�ķֱ���,����С�ڵ�����Ļ�ķֱ���!
 *              2, �뱣֤ͼƬ�Ŀ����16�ı���,����������!
 *
 * @param       filename : ����·�����ļ���(.jpeg/jpg)
 * @retval      �������
 *   @arg       0   , �ɹ�
 *   @arg       ����, ������
 */
uint8_t hjpgd_decode(char *filename)
{
    FIL *ftemp;
    uint16_t *rgb565buf = 0;
    volatile  uint32_t timecnt = 0;
    uint32_t br = 0;
    uint8_t fileover = 0;
    uint8_t i = 0;
    uint8_t res;
    res = jpeg_core_init(&hjpgd);   /* ��ʼ��JPEG�ں� */

    if (res)return 1;

    ftemp = (FIL *)mymalloc(SRAMITCM, sizeof(FIL)); /* �����ڴ� */

    if (f_open(ftemp, filename, FA_READ) != FR_OK)  /* ��ͼƬʧ�� */
    {
        jpeg_core_destroy(&hjpgd);
        myfree(SRAMITCM, ftemp);    /* �ͷ��ڴ� */
        return 2;
    }

    rgb565buf = mymalloc(SRAMEX, lcddev.width * lcddev.height * 2);         /* ������֡�ڴ� */
    jpeg_decode_init(&hjpgd);       /* ��ʼ��Ӳ��JPEG������ */

    for (i = 0; i < JPEG_DMA_INBUF_NB; i++)
    {
        res = f_read(ftemp, hjpgd.inbuf[i].buf, JPEG_DMA_INBUF_LEN, &br);   /* ���������������ݻ����� */

        if (res == FR_OK && br)
        {
            hjpgd.inbuf[i].size = br;   /* ��ȡ */
            hjpgd.inbuf[i].sta = 1;     /* ���buf�� */
        }

        if (br == 0)break;
    }

    jpeg_in_dma_init((uint32_t)hjpgd.inbuf[0].buf, hjpgd.inbuf[0].size);/* ��������DMA */
    jpeg_in_callback = jpeg_dma_in_callback;        /* JPEG DMA��ȡ���ݻص����� */
    jpeg_out_callback = jpeg_dma_out_callback;      /* JPEG DMA������ݻص����� */
    jpeg_eoc_callback = jpeg_endofcovert_callback;  /* JPEG ��������ص����� */
    jpeg_hdp_callback = jpeg_hdrover_callback;      /* JPEG Header������ɻص����� */
    jpeg_in_dma_start();    /* ����DMA IN����,��ʼ����JPEGͼƬ */

    while (1)
    {
        if (hjpgd.inbuf[hjpgd.inbuf_write_ptr].sta == 0 && fileover == 0)   /* ��bufΪ�� */
        {
            res = f_read(ftemp, hjpgd.inbuf[hjpgd.inbuf_write_ptr].buf, JPEG_DMA_INBUF_LEN, &br);   /* ����һ�������� */

            if (res == FR_OK && br)
            {
                hjpgd.inbuf[hjpgd.inbuf_write_ptr].size = br;   /* ��ȡ */
                hjpgd.inbuf[hjpgd.inbuf_write_ptr].sta = 1;     /* buf�� */
            }
            else if (br == 0)
            {
                timecnt = 0;    /* �����ʱ�� */
                fileover = 1;   /* �ļ������� */
            }

            if (hjpgd.indma_pause == 1 && hjpgd.inbuf[hjpgd.inbuf_read_ptr].sta == 1)   /* ֮ǰ����ͣ����,�������� */
            {
                jpeg_in_dma_resume((uint32_t)hjpgd.inbuf[hjpgd.inbuf_read_ptr].buf, hjpgd.inbuf[hjpgd.inbuf_read_ptr].size);    /* ������һ��DMA���� */
                hjpgd.indma_pause = 0;
            }

            hjpgd.inbuf_write_ptr++;

            if (hjpgd.inbuf_write_ptr >= JPEG_DMA_INBUF_NB)hjpgd.inbuf_write_ptr = 0;
        }

        if (hjpgd.outbuf[hjpgd.outbuf_read_ptr].sta == 1)   /* buf����������Ҫ���� */
        {
            SCB_CleanInvalidateDCache();    /* ���D catch */
            jpeg_dma2d_yuv2rgb_conversion(&hjpgd, (uint32_t *)rgb565buf);   /* ����DMA2D,��YUVͼ��ת��RGB565ͼ�� */

            //SCB_CleanInvalidateDCache();                  /* ���D catch */
            hjpgd.outbuf[hjpgd.outbuf_read_ptr].sta = 0;    /* ���bufΪ�� */
            hjpgd.outbuf[hjpgd.outbuf_read_ptr].size = 0;   /* ��������� */
            hjpgd.outbuf_read_ptr++;

            if (hjpgd.outbuf_read_ptr >= JPEG_DMA_OUTBUF_NB)hjpgd.outbuf_read_ptr = 0;  /* ���Ʒ�Χ */

            if (hjpgd.yuvblk_curheight >= hjpgd.Conf.ImageHeight)break; /* ��ǰ�߶ȵ��ڻ��߳���ͼƬ�ֱ��ʵĸ߶�,��˵�����������,ֱ���˳� */
        }
        else if (hjpgd.outdma_pause == 1 && hjpgd.outbuf[hjpgd.outbuf_write_ptr].sta == 0)  /* out��ͣ,�ҵ�ǰwritebuf�Ѿ�Ϊ����,��ָ�out��� */
        {
            jpeg_out_dma_resume((uint32_t)hjpgd.outbuf[hjpgd.outbuf_write_ptr].buf, hjpgd.yuvblk_size); /* ������һ��DMA���� */
            hjpgd.outdma_pause = 0;
        }

        timecnt++;

        if (hjpgd.state == JPEG_STATE_ERROR)    /* �������,ֱ���˳� */
        {
            res = 2;
            break;
        }

        if (fileover)   /* �ļ�������,��ʱ�˳�,��ֹ��ѭ�� */
        {
            if (hjpgd.state == JPEG_STATE_NOHEADER && hjpgd.indma_pause == 1)   /* ��ǰ������ͣ״̬����û�н�����JPEGͷ */
            {
                break;  /* ����JPEGͷʧ���� */
            }

            if (timecnt > 0X3FFFF)break;    /* ��ʱ�˳� */
        }
    }
    
    if (hjpgd.state == JPEG_STATE_FINISHED) /* ���������, һ���������LCD */
    {
        pic_phy.fillcolor(picinfo.S_XOFF, picinfo.S_YOFF, hjpgd.Conf.ImageWidth, hjpgd.Conf.ImageHeight, rgb565buf);
    }

    f_close(ftemp);             /* �ر��ļ� */
    myfree(SRAMITCM, ftemp);    /* �ͷ�������ڴ� */
    myfree(SRAMEX, rgb565buf);  /* �ͷ��ڴ� */
    jpeg_core_destroy(&hjpgd);  /* ����JPEG����,�ͷ��ڴ� */
    return res;
}











