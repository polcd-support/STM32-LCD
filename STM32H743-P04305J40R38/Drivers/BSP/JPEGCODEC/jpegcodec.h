/**
 ****************************************************************************************************
 * @file        jpegcodec.h
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

#ifndef __JPEGCODEC_H
#define __JPEGCODEC_H

#include "./SYSTEM/sys/sys.h"


#define JPEG_DMA_INBUF_LEN          4096                /* ����MDMA IN  BUF�Ĵ�С */
#define JPEG_DMA_INBUF_NB           10                  /* MDMA IN  BUF�ĸ��� */
#define JPEG_DMA_OUTBUF_NB          2                   /* MDMA OUT BUF�ĸ��� */


#define JPEG_QUANT_TABLE_SIZE       ((uint32_t)64U)     /* JPEG Quantization Table Size */

/* JPEG���ݻ���ṹ�� */
typedef struct
{
    uint8_t sta;        /* ״̬:0,������;1,������ */
    uint8_t *buf;       /* JPEG���ݻ����� */
    uint16_t size;      /* JPEG���ݳ��� */
} jpeg_databuf_type;

#define JPEG_STATE_NOHEADER         0                   /* HEADERδ��ȡ,��ʼ״̬ */
#define JPEG_STATE_HEADEROK         1                   /* HEADER��ȡ�ɹ� */
#define JPEG_STATE_FINISHED         2                   /* ������� */
#define JPEG_STATE_ERROR            3                   /* ������� */

#define JPEG_GRAYSCALE_COLORSPACE   ((uint32_t)0x00000000U)
#define JPEG_YCBCR_COLORSPACE       JPEG_CONFR1_COLORSPACE_0
#define JPEG_CMYK_COLORSPACE        JPEG_CONFR1_COLORSPACE

#define JPEG_444_SUBSAMPLING        ((uint32_t)0x00000000U)     /* Chroma Subsampling 4:4:4 */
#define JPEG_420_SUBSAMPLING        ((uint32_t)0x00000001U)     /* Chroma Subsampling 4:2:0 */
#define JPEG_422_SUBSAMPLING        ((uint32_t)0x00000002U)     /* Chroma Subsampling 4:2:2 */

#define DMA2D_NO_CSS                ((uint32_t)0x00000000U)     /* No chroma sub-sampling 4:4:4 */
#define DMA2D_CSS_422               ((uint32_t)0x00000001U)     /* chroma sub-sampling 4:2:2 */
#define DMA2D_CSS_420               ((uint32_t)0x00000002U)     /* chroma sub-sampling 4:2:0 */


/* JPEG�ļ���Ϣ�ṹ�� */
typedef struct
{
    uint8_t  ColorSpace;            /* ͼ�����ɫ�ռ�: gray-scale/YCBCR/RGB/CMYK */
    uint8_t  ChromaSubsampling;     /* YCBCR/CMYK��ɫ�ռ��ɫ�ȳ������:0,4:4:4;1,4:2:2;2,4:1:1;3,4:2:0 */
    uint32_t ImageHeight;           /* ͼ��߶� */
    uint32_t ImageWidth;            /* ͼ���� */
    uint8_t  ImageQuality;          /* ͼ���������:1~100 */
} JPEG_ConfTypeDef;


/* jpeg�������ƽṹ�� */
typedef struct
{
    JPEG_ConfTypeDef    Conf;                       /* ��ǰJPEG�ļ���ز��� */
    jpeg_databuf_type inbuf[JPEG_DMA_INBUF_NB];     /* MDMA IN buf */
    jpeg_databuf_type outbuf[JPEG_DMA_OUTBUF_NB];   /* MDMA OUT buf */
    volatile uint8_t inbuf_read_ptr;                /* MDMA IN buf��ǰ��ȡλ�� */
    volatile uint8_t inbuf_write_ptr;               /* MDMA IN buf��ǰд��λ�� */
    volatile uint8_t indma_pause;                   /* ����MDMA��ͣ״̬��ʶ */
    volatile uint8_t outbuf_read_ptr;               /* MDMA OUT buf��ǰ��ȡλ�� */
    volatile uint8_t outbuf_write_ptr;              /* MDMA OUT buf��ǰд��λ�� */
    volatile uint8_t outdma_pause;                  /* ����MDMA��ͣ״̬��ʶ */
    volatile uint8_t state;                         /* ����״̬;0,δʶ��Header;1,ʶ����Header;2,�������; */
    uint32_t yuvblk_size;                           /* YUV������ֽ���,ʹ�����һ��DMA2D YUV2RGBת��,�պ���ͼƬ��ȵ�������
                                                     * YUV420ͼƬ,ÿ������ռ1.5��YUV�ֽ�,ÿ�����16��,yuvblk_size=ͼƬ���*16*1.5
                                                     * YUV422ͼƬ,ÿ������ռ2��YUV�ֽں�RGB565һ��,ÿ�����8��,yuvblk_size=ͼƬ���*8*2
                                                     * YUV444ͼƬ,ÿ������ռ3��YUV�ֽ�,ÿ�����8��,yuvblk_size=ͼƬ���*8*3
                                                     */
                                                    
    uint16_t yuvblk_height;                         /* ÿ��YUV��������صĸ߶�,����YUV420,Ϊ16,����YUV422/YUV444Ϊ8 */
    uint16_t yuvblk_curheight;                      /* ��ǰ����߶�,0~�ֱ��ʸ߶� */
} jpeg_codec_typedef;

/* MDMA�ص�����(��Ҫ�ⲿʵ��) */
extern void (*jpeg_in_callback)(void);              /* JPEG MDMA����ص����� */
extern void (*jpeg_out_callback)(void);             /* JPEG MDMA��� �ص����� */
extern void (*jpeg_eoc_callback)(void);             /* JPEG ������� �ص����� */
extern void (*jpeg_hdp_callback)(void);             /* JPEG Header������� �ص����� */


/* �ӿں��� */

void jpeg_dma_stop(void);
void jpeg_in_dma_start(void);
void jpeg_out_dma_start(void);
void jpeg_in_dma_resume(uint32_t memaddr, uint32_t memlen);
void jpeg_out_dma_resume(uint32_t memaddr, uint32_t memlen);
void jpeg_in_dma_init(uint32_t meminaddr, uint32_t meminsize);
void jpeg_out_dma_init(uint32_t memoutaddr, uint32_t memoutsize);

uint8_t jpeg_get_quality(void);
void jpeg_get_info(jpeg_codec_typedef *tjpeg);
void jpeg_decode_init(jpeg_codec_typedef *tjpeg);
void jpeg_core_destroy(jpeg_codec_typedef *tjpeg);
uint8_t jpeg_core_init(jpeg_codec_typedef *tjpeg);
uint8_t jpeg_dma2d_yuv2rgb_conversion(jpeg_codec_typedef *tjpeg, uint32_t *pdst);

#endif




















