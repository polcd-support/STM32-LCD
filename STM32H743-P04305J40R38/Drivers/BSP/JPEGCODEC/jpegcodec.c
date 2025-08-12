/**
 ****************************************************************************************************
 * @file        jpegcodec.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       JPEGӲ��������� ��������
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
 * V1.0 20220324
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./MALLOC/malloc.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/JPEGCODEC/jpegcodec.h"
#include "os.h"


/* JPEG�淶(ISO/IEC 10918-1��׼)������������
 * ��ȡJPEGͼƬ����ʱ��Ҫ�õ�
 */
const uint8_t JPEG_LUM_QuantTable[JPEG_QUANT_TABLE_SIZE] =
{
    16, 11, 10, 16, 24, 40, 51, 61, 12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56, 14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77, 24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99
};

const uint8_t JPEG_ZIGZAG_ORDER[JPEG_QUANT_TABLE_SIZE] =
{
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

/**
 * @brief       JPEGӲ����������MDMA����
 * @param       meminaddr   : JPEG����MDMA�洢����ַ
 * @param       meminsize   : ����MDMA���ݳ���,0~262143,���ֽ�Ϊ��λ
 * @retval      ��
 */
void jpeg_in_dma_init(uint32_t meminaddr, uint32_t meminsize)
{
    uint32_t regval = 0;
    uint32_t addrmask = 0;
    RCC->AHB3ENR |= 1 << 0;             /* ʹ��MDMAʱ�� */
    MDMA_Channel7->CCR = 0;             /* ����MDMA���� */

    while (MDMA_Channel7->CCR & 0X01);  /* �ȴ�MDMA_Channel7�ر���� */

    MDMA_Channel7->CIFCR = 0X1F;        /* �жϱ�־���� */
    MDMA_Channel7->CCR |= 1 << 2;       /* CTCIE=1,ʹ��ͨ����������ж� */
    MDMA_Channel7->CCR |= 2 << 6;       /* PL[1:0]=2,�����ȼ� */
    MDMA_Channel7->CBNDTR = meminsize;  /* ���䳤��Ϊmeminsize */
    MDMA_Channel7->CDAR = (uint32_t)&JPEG->DIR; /* Ŀ���ַΪ:JPEG->DIR */
    MDMA_Channel7->CSAR = meminaddr;    /* meminaddr��ΪԴ��ַ */
    regval = 0 << 28;                   /* TRGM[1:0]=0,ÿ��MDMA���󴥷�һ��buffer���� */
    regval |= 1 << 25;                  /* PKE=1,���ʹ�� */
    regval |= (32 - 1) << 18;           /* TLEN[6:0]=31,buffer���䳤��Ϊ32�ֽ� */
    regval |= 4 << 15;                  /* DBURST[2:0]=4,Ŀ��ͻ�����䳤��Ϊ16 */
    regval |= 4 << 12;                  /* SBURST[2:0]=4,Դͻ�����䳤��Ϊ16 */
    regval |= 0 << 8;                   /* SINCOS[1:0]=0,Դ��ַ�仯��λΪ8λ(�ֽ�) */
    regval |= 2 << 6;                   /* DSIZE[1:0]=2,Ŀ��λ��Ϊ32λ */
    regval |= 0 << 4;                   /* SSIZE[1:0]=0,Դλ��Ϊ8λ */
    regval |= 0 << 2;                   /* DINC[1:0]=0,Ŀ���ַ�̶� */
    regval |= 2 << 0;                   /* SINC[1:0]=2,Դ��ַ���� */
    MDMA_Channel7->CTCR = regval;       /* ����CTCR�Ĵ��� */
    MDMA_Channel7->CTBR = 17 << 0;      /* MDMA��Ӳ������ͨ��17����inmdma,ͨ��17=JPEG input FIFO threshold
                                         * ���<STM32H7xx�ο��ֲ�>577ҳ,table 95
                                         */
    addrmask = meminaddr & 0XFF000000;  /* ��ȡ���� */

    if (addrmask == 0X20000000 || addrmask == 0)MDMA_Channel7->CTBR |= 1 << 16; /* ʹ��AHBS���߷���DTCM/ITCM */

    sys_nvic_init(2, 3, MDMA_IRQn, 2);  /* ��ռ2�������ȼ�3����2 */
}

/**
 * @brief       JPEGӲ���������MDMA����
 * @param       memoutaddr  : JPEG���MDMA�洢����ַ
 * @param       memoutsize  : ���MDMA���ݳ���,0~262143,���ֽ�Ϊ��λ
 * @retval      ��
 */
void jpeg_out_dma_init(uint32_t memoutaddr, uint32_t memoutsize)
{
    uint32_t regval = 0;
    uint32_t addrmask = 0;
    RCC->AHB3ENR |= 1 << 0;             /* ʹ��MDMAʱ�� */
    MDMA_Channel6->CCR = 0;             /* ���MDMA���� */

    while (MDMA_Channel6->CCR & 0X01);  /* �ȴ�MDMA_Channel6�ر���� */

    MDMA_Channel6->CIFCR = 0X1F;        /* �жϱ�־���� */
    MDMA_Channel6->CCR |= 3 << 6;       /* PL[1:0]=2,������ȼ� */
    MDMA_Channel6->CCR |= 1 << 2;       /* CTCIE=1,ʹ��ͨ����������ж� */
    MDMA_Channel6->CBNDTR = memoutsize; /* ���䳤��Ϊmeminsize */
    MDMA_Channel6->CDAR = memoutaddr;   /* Ŀ���ַΪ:memoutaddr */
    MDMA_Channel6->CSAR = (uint32_t)&JPEG->DOR; /* JPEG->DOR��ΪԴ��ַ */
    regval = 0 << 28;                   /* TRGM[1:0]=0,ÿ��MDMA���󴥷�һ��buffer���� */
    regval |= 1 << 25;                  /* PKE=1,���ʹ�� */
    regval |= (32 - 1) << 18;           /* TLEN[6:0]=31,buffer���䳤��Ϊ32�ֽ� */
    regval |= 4 << 15;                  /* DBURST[2:0]=4,Ŀ��ͻ�����䳤��Ϊ16 */
    regval |= 4 << 12;                  /* SBURST[2:0]=4,Դͻ�����䳤��Ϊ16 */
    regval |= 0 << 10;                  /* DINCOS[1:0]=0,Ŀ���ַ�仯��λΪ8λ(�ֽ�) */
    regval |= 0 << 6;                   /* DSIZE[1:0]=0,Ŀ��λ��Ϊ8λ */
    regval |= 2 << 4;                   /* SSIZE[1:0]=2,Դλ��Ϊ32λ */
    regval |= 2 << 2;                   /* DINC[1:0]=2,Ŀ���ַ���� */
    regval |= 0 << 0;                   /* SINC[1:0]=0,Դ��ַ�̶� */
    MDMA_Channel6->CTCR = regval;       /* ����CTCR�Ĵ��� */
    MDMA_Channel6->CTBR = 19 << 0;      /* MDMA��Ӳ������ͨ��19����outmdma,ͨ��19=JPEG output FIFO threshold */
    /* ���<STM32H7xx�ο��ֲ�>577ҳ,table 95 */
    addrmask = memoutaddr & 0XFF000000; /* ��ȡ���� */

    if (addrmask == 0X20000000 || addrmask == 0)MDMA_Channel6->CTBR |= 1 << 17; /* ʹ��AHBS���߷���DTCM/ITCM */

    /* ��jpeg_in_dma_init�����Ѿ��������жϳ�ʼ��, ����������Բ����� */
    //sys_nvic_init(2, 3, MDMA_IRQn, 2);    /* ��ռ2�������ȼ�3����2 */
}

void (*jpeg_in_callback)(void);         /* JPEG MDMA����ص����� */
void (*jpeg_out_callback)(void);        /* JPEG MDMA��� �ص����� */
void (*jpeg_eoc_callback)(void);        /* JPEG ������� �ص����� */
void (*jpeg_hdp_callback)(void);        /* JPEG Header������� �ص����� */

/**
 * @brief       MDMA�жϷ�����
 *   @note      ����Ӳ��JPEG����ʱ����/���������
 * @param       ��
 * @retval      ��
 */
void MDMA_IRQHandler(void)
{
    OSIntEnter();
    if (MDMA_Channel7->CISR & (1 << 1))     /* CTCIF,ͨ��7�������(����) */
        if (MDMA_Channel7->CISR & (1 << 1)) /* CTCIF,ͨ��7�������(����) */
        {
            MDMA_Channel7->CIFCR |= 1 << 1; /* ���ͨ����������ж� */
            JPEG->CR &= ~(0X7E);            /* �ر�JPEG�ж�,��ֹ����� */
            jpeg_in_callback();             /* ִ������ص�����,������ȡ���� */
            JPEG->CR |= 3 << 5;             /* ʹ��EOC��HPD�ж� */
        }

    if (MDMA_Channel6->CISR & (1 << 1))     /* CTCIF,ͨ��6�������(���) */
    {
        MDMA_Channel6->CIFCR |= 1 << 1;     /* ���ͨ����������ж� */
        JPEG->CR &= ~(0X7E);                /* �ر�JPEG�ж�,��ֹ����� */
        jpeg_out_callback();                /* ִ������ص�����,������ת����RGB */
        JPEG->CR |= 3 << 5;                 /* ʹ��EOC��HPD�ж� */
    }
    OSIntExit();
}

/**
 * @brief       JPEG�����жϷ�����
 * @param       ��
 * @retval      ��
 */
void JPEG_IRQHandler(void)
{
    OSIntEnter();
    if (JPEG->SR & (1 << 6))    /* JPEG Header������� */
    {
        jpeg_hdp_callback();
        JPEG->CR &= ~(1 << 6);  /* ��ֹJpeg Header��������ж� */
        JPEG->CFR |= 1 << 6;    /* ���HPDFλ(header�������λ) */
    }

    if (JPEG->SR & (1 << 5))    /* JPEG������� */
    {
        jpeg_dma_stop();
        jpeg_eoc_callback();
        JPEG->CFR |= 1 << 5;    /* ���EOCλ(�������λ) */
        MDMA_Channel6->CCR &= ~(1 << 0);    /* �ر�MDMAͨ��6 */
        MDMA_Channel7->CCR &= ~(1 << 0);    /* �ر�MDMAͨ��7 */
    }
    OSIntExit();
}

/**
 * @brief       ��ʼ��Ӳ��JPEG�ں�
 * @param       tjpeg       : JPEG�������ƽṹ��
 * @retval      0, �ɹ�; 1, ʧ��;
 */
uint8_t jpeg_core_init(jpeg_codec_typedef *tjpeg)
{
    uint8_t i;
    RCC->AHB3ENR |= 1 << 5;             /* ʹ��Ӳ��jpegʱ�� */

    for (i = 0; i < JPEG_DMA_INBUF_NB; i++)
    {
        tjpeg->inbuf[i].buf = mymalloc(SRAMDTCM, JPEG_DMA_INBUF_LEN);

        if (tjpeg->inbuf[i].buf == NULL)
        {
            jpeg_core_destroy(tjpeg);
            return 1;
        }
    }

    JPEG->CR = 0;                       /* ������ */
    JPEG->CR |= 1 << 0;                 /* ʹ��Ӳ��JPEG */
    JPEG->CONFR0 &= ~(1 << 0);          /* ֹͣJPEG�������� */
    JPEG->CR |= 1 << 13;                /* �������fifo */
    JPEG->CR |= 1 << 14;                /* ������fifo */
    JPEG->CFR = 3 << 5;                 /* ��ձ�־ */
    sys_nvic_init(1, 3, JPEG_IRQn, 2);  /* ��ռ1�������ȼ�3����2 */
    JPEG->CONFR1 |= 1 << 8;             /* ʹ��header���� */
    return 0;
}

/**
 * @brief       �ر�Ӳ��JPEG�ں�,���ͷ��ڴ�
 * @param       tjpeg       : JPEG�������ƽṹ��
 * @retval      ��
 */
void jpeg_core_destroy(jpeg_codec_typedef *tjpeg)
{
    uint8_t i;
    jpeg_dma_stop();    /* ֹͣMDMA���� */

    for (i = 0; i < JPEG_DMA_INBUF_NB; i++)
    {
        myfree(SRAMDTCM, tjpeg->inbuf[i].buf);  /* �ͷ��ڴ� */
    }
    for (i = 0; i < JPEG_DMA_OUTBUF_NB; i++)
    {
        myfree(SRAMIN, tjpeg->outbuf[i].buf);   /* �ͷ��ڴ� */
    }
}

/**
 * @brief       ��ʼ��Ӳ��JPEG������
 * @param       tjpeg       : JPEG�������ƽṹ��
 * @retval      ��
 */
void jpeg_decode_init(jpeg_codec_typedef *tjpeg)
{
    uint8_t i;
    tjpeg->inbuf_read_ptr = 0;
    tjpeg->inbuf_write_ptr = 0;
    tjpeg->indma_pause = 0;
    tjpeg->outbuf_read_ptr = 0;
    tjpeg->outbuf_write_ptr = 0;
    tjpeg->outdma_pause = 0;
    tjpeg->state = JPEG_STATE_NOHEADER; /* ͼƬ���������־ */

    for (i = 0; i < JPEG_DMA_INBUF_NB; i++)
    {
        tjpeg->inbuf[i].sta = 0;
        tjpeg->inbuf[i].size = 0;
    }

    for (i = 0; i < JPEG_DMA_OUTBUF_NB; i++)
    {
        tjpeg->outbuf[i].sta = 0;
        tjpeg->outbuf[i].size = 0;
    }

    MDMA_Channel6->CCR = 0;         /* MDMAͨ��6��ֹ */
    MDMA_Channel7->CCR = 0;         /* MDMAͨ��7��ֹ */
    MDMA_Channel6->CIFCR = 0X1F;    /* �жϱ�־���� */
    MDMA_Channel7->CIFCR = 0X1F;    /* �жϱ�־���� */

    JPEG->CONFR1 |= 1 << 3;         /* Ӳ��JPEG����ģʽ */
    JPEG->CONFR0 &= ~(1 << 0);      /* ֹͣJPEG�������� */
    JPEG->CR &= ~(0X3F << 1);       /* �ر������ж� */
    JPEG->CR |= 1 << 13;            /* �������fifo */
    JPEG->CR |= 1 << 14;            /* ������fifo */
    JPEG->CR |= 1 << 6;             /* ʹ��Jpeg Header��������ж� */
    JPEG->CR |= 1 << 5;             /* ʹ�ܽ�������ж� */
    JPEG->CFR = 3 << 5;             /* ��ձ�־ */
    JPEG->CONFR0 |= 1 << 0;         /* ʹ��JPEG�������� */
}

/**
 * @brief       ���� jpeg in mdma, ��ʼ����JPEG
 * @param       ��
 * @retval      ��
 */
void jpeg_in_dma_start(void)
{
    MDMA_Channel7->CCR |= 1 << 0;   /* ʹ��MDMAͨ��7�Ĵ��� */
}

/**
 * @brief       ���� jpeg out mdma, ��ʼ���YUV����
 * @param       ��
 * @retval      ��
 */
void jpeg_out_dma_start(void)
{
    MDMA_Channel6->CCR |= 1 << 0;   /* ʹ��MDMAͨ��6�Ĵ��� */
}

/**
 * @brief       ֹͣJPEG MDMA�������
 * @param       ��
 * @retval      ��
 */
void jpeg_dma_stop(void)
{
    JPEG->CONFR0 &= ~(1 << 0);      /* ֹͣJPEG�������� */
    JPEG->CR &= ~(0X3F << 1);       /* �ر������ж� */
    JPEG->CFR = 3 << 5;             /* ��ձ�־ */
}

/**
 * @brief       �ָ�MDMA IN����
 * @param       memaddr     : �洢���׵�ַ
 * @param       memlen      : Ҫ�������ݳ���(���ֽ�Ϊ��λ)
 * @retval      ��
 */
void jpeg_in_dma_resume(uint32_t memaddr, uint32_t memlen)
{
    if (memlen % 4)memlen += 4 - memlen % 4;    /* ��չ��4�ı��� */

    MDMA_Channel7->CIFCR = 0X1F;    /* �жϱ�־���� */
    MDMA_Channel7->CBNDTR = memlen; /* ���䳤��Ϊmemlen */
    MDMA_Channel7->CSAR = memaddr;  /* memaddr��ΪԴ��ַ */
    MDMA_Channel7->CCR |= 1 << 0;   /* ʹ��MDMAͨ��7�Ĵ��� */
}

/**
 * @brief       �ָ�MDMA OUT����
 * @param       memaddr     : �洢���׵�ַ
 * @param       memlen      : Ҫ�������ݳ���(���ֽ�Ϊ��λ)
 * @retval      ��
 */
void jpeg_out_dma_resume(uint32_t memaddr, uint32_t memlen)
{
    if (memlen % 4)memlen += 4 - memlen % 4;    /* ��չ��4�ı��� */

    MDMA_Channel6->CIFCR = 0X1F;    /* �жϱ�־���� */
    MDMA_Channel6->CBNDTR = memlen; /* ���䳤��Ϊmemlen */
    MDMA_Channel6->CDAR = memaddr;  /* memaddr��ΪԴ��ַ */
    MDMA_Channel6->CCR |= 1 << 0;   /* ʹ��MDMAͨ��6�Ĵ��� */
}

/**
 * @brief       ��ȡͼ����Ϣ
 * @param       tjpeg       : JPEG�������ƽṹ��
 * @retval      ��
 */
void jpeg_get_info(jpeg_codec_typedef *tjpeg)
{
    uint32_t yblockNb, cBblockNb, cRblockNb;

    switch (JPEG->CONFR1 & 0X03)
    {
        case 0:/* grayscale,1 color component */
            tjpeg->Conf.ColorSpace = JPEG_GRAYSCALE_COLORSPACE;
            break;

        case 2:/* YUV/RGB,3 color component */
            tjpeg->Conf.ColorSpace = JPEG_YCBCR_COLORSPACE;
            break;

        case 3:/* CMYK,4 color component */
            tjpeg->Conf.ColorSpace = JPEG_CMYK_COLORSPACE;
            break;
    }

    tjpeg->Conf.ImageHeight = (JPEG->CONFR1 & 0XFFFF0000) >> 16;    /* ���ͼ��߶� */
    tjpeg->Conf.ImageWidth = (JPEG->CONFR3 & 0XFFFF0000) >> 16;     /* ���ͼ���� */

    if ((tjpeg->Conf.ColorSpace == JPEG_YCBCR_COLORSPACE) || (tjpeg->Conf.ColorSpace == JPEG_CMYK_COLORSPACE))
    {
        yblockNb  = (JPEG->CONFR4 & (0XF << 4)) >> 4;
        cBblockNb = (JPEG->CONFR5 & (0XF << 4)) >> 4;
        cRblockNb = (JPEG->CONFR6 & (0XF << 4)) >> 4;

        if ((yblockNb == 1) && (cBblockNb == 0) && (cRblockNb == 0))
        {
            tjpeg->Conf.ChromaSubsampling = JPEG_422_SUBSAMPLING;   /* 16x8 block */
        }
        else if ((yblockNb == 0) && (cBblockNb == 0) && (cRblockNb == 0))
        {
            tjpeg->Conf.ChromaSubsampling = JPEG_444_SUBSAMPLING;
        }
        else if ((yblockNb == 3) && (cBblockNb == 0) && (cRblockNb == 0))
        {
            tjpeg->Conf.ChromaSubsampling = JPEG_420_SUBSAMPLING;
        }
        else
        {
            tjpeg->Conf.ChromaSubsampling = JPEG_444_SUBSAMPLING;
        }
    }
    else
    {
        tjpeg->Conf.ChromaSubsampling = JPEG_444_SUBSAMPLING;   /* Ĭ����4:4:4 */
    }
    
    tjpeg->Conf.ImageQuality = 0;   /* ͼ����������������ͼƬ����ĩβ,�տ�ʼ��ʱ��,���޷���ȡ��,����ֱ������Ϊ0 */
}

/**
 * @brief       �õ�JPEGͼ������
 *   @note      �ڽ�����ɺ�,���Ե��ò������ȷ�Ľ��.
 * @param       ��
 * @retval      ͼ������, 0~100
 */
uint8_t jpeg_get_quality(void)
{
    uint32_t quality = 0;
    uint32_t quantRow, quantVal, scale, i, j;
    uint32_t *tableAddress = (uint32_t *)JPEG->QMEM0;
    i = 0;

    while (i < JPEG_QUANT_TABLE_SIZE)
    {
        quantRow = *tableAddress;

        for (j = 0; j < 4; j++)
        {
            quantVal = (quantRow >> (8 * j)) & 0xFF;

            if (quantVal == 1)quality += 100;   /* 100% */
            else
            {
                scale = (quantVal * 100) / ((uint32_t)JPEG_LUM_QuantTable[JPEG_ZIGZAG_ORDER[i + j]]);

                if (scale <= 100)
                {
                    quality += (200 - scale) / 2;
                }
                else
                {
                    quality += 5000 / scale;
                }
            }
        }

        i += 4;
        tableAddress++;
    }

    return (quality / ((uint32_t)64));
}

/**
 * @brief       ��YUV����ת����RGB����
 *   @note      ����DMA2D, ��JPEG�����YUV����ת����RGB����, ȫӲ�����, �ٶȷǳ���
 * @param       tjpeg       : JPEG�������ƽṹ��
 * @param       pdst        : ��������׵�ַ
 * @retval      0, �ɹ�; 1, ��ʱ,ʧ��;
 */
uint8_t jpeg_dma2d_yuv2rgb_conversion(jpeg_codec_typedef *tjpeg, uint32_t *pdst)
{
    uint32_t regval = 0;
    uint32_t cm = 0;            /* ������ʽn */
    uint32_t timeout = 0;
    uint32_t destination = 0;

    if (tjpeg->Conf.ChromaSubsampling == JPEG_420_SUBSAMPLING)
    {
        cm = DMA2D_CSS_420;     /* YUV420תRGB */
    }
    else if (tjpeg->Conf.ChromaSubsampling == JPEG_422_SUBSAMPLING)
    {
        cm = DMA2D_CSS_422;     /* YUV422תRGB */
    }
    else if (tjpeg->Conf.ChromaSubsampling == JPEG_444_SUBSAMPLING)
    {
        cm = DMA2D_NO_CSS;      /* YUV444תRGB */
    }
    
    destination = (uint32_t)pdst + (tjpeg->yuvblk_curheight * tjpeg->Conf.ImageWidth) * 2;  /* ����Ŀ���ַ���׵�ַ */
    
    RCC->AHB3ENR |= 1 << 4;     /* ʹ��DMA2Dʱ�� */
    RCC->AHB3RSTR |= 1 << 4;    /* ��λDMA2D */
    RCC->AHB3RSTR &= ~(1 << 4); /* ������λ */
    DMA2D->CR &= ~(1 << 0);     /* ��ֹͣDMA2D */
    DMA2D->CR = 1 << 16;        /* MODE[2:0]=001,�洢�����洢��,��PFCģʽ */
    DMA2D->OPFCCR = 2 << 0;     /* CM[2:0]=010,���ΪRGB565��ʽ */
    DMA2D->OOR = 0;             /* ������ƫ��Ϊ0 */
    DMA2D->IFCR |= 1 << 1;      /* ���������ɱ�־ */
    regval = 11 << 0;           /* CM[3:0]=1011,��������ΪYCbCr��ʽ */
    regval |= cm << 18;         /* CSS[1:0]=cm,Chroma Sub-Sampling:0,4:4:4;1,4:2:2;2,4:2:0 */
    DMA2D->FGPFCCR = regval;    /* ����FGPCCR�Ĵ��� */
    DMA2D->FGOR = 0;            /* ǰ������ƫ��Ϊ0 */
    DMA2D->NLR = tjpeg->yuvblk_height | (tjpeg->Conf.ImageWidth << 16); /* �趨�����Ĵ��� */
    DMA2D->OMAR = destination;  /* ����洢����ַ */
    DMA2D->FGMAR = (uint32_t)tjpeg->outbuf[tjpeg->outbuf_read_ptr].buf; /* Դ��ַ */
    DMA2D->CR |= 1 << 0;        /* ����DMA2D */

    while ((DMA2D->ISR & (1 << 1)) == 0)    /* �ȴ�������� */
    {
        timeout++;

        if (timeout > 0X1FFFFFF)break;      /* ��ʱ�˳� */
    }

    tjpeg->yuvblk_curheight += tjpeg->yuvblk_height;/* ƫ�Ƶ���һ���ڴ��ַ */
    /* YUV2RGBת�������,�ٸ�λһ��DMA2D */
    RCC->AHB3RSTR |= 1 << 4;    /* ��λDMA2D */
    RCC->AHB3RSTR &= ~(1 << 4); /* ������λ */

    if (timeout > 0X1FFFFFF)return 1;

    return 0;
}





























