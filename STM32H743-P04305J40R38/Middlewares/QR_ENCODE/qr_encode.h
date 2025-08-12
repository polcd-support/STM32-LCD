/**
 ****************************************************************************************************
 * @file        qr_encode.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-06-02
 * @brief       ��ά����� ����
 *  @note       ��������ֲ����������Ķ�ά�����ɹ���, �ش˸�л!
 *              ��ά������ԭ����� http://coolshell.cn/articles/10590.html#jtss-tsina
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
 *
 ****************************************************************************************************
 */

#ifndef __QR_ENCODE_H
#define __QR_ENCODE_H

#include "./SYSTEM/sys/sys.h"


#define QR_TRUE             1       /* TRUE���� */
#define QR_FALSE            0       /* FALSE���� */


#define min(a,b)  (((a) < (b)) ? (a) : (b))

/* ����ȼ�(4��) */
#define QR_LEVEL_L          0       /* 7%������ɱ����� */
#define QR_LEVEL_M          1       /* 15%������ɱ����� */
#define QR_LEVEL_Q          2       /* 25%������ɱ����� */
#define QR_LEVEL_H          3       /* 30%������ɱ����� */

/* ������ʽ */
#define QR_MODE_NUMERAL     0
#define QR_MODE_ALPHABET    1
#define QR_MODE_8BIT        2
#define QR_MODE_KANJI       3
#define QR_MODE_CHINESE     4

/**
 * Number of bits per length field
 * Encoding  Ver.1�C9     10�C26       27�C40
 * Numeric      10          12          14
 * Alphanumeric 9           11          13
 * uint8_t           8           16          16
 * Kanji        8           10          12
 * Chinese
 * P17 �ַ�����ָʾ��λ��
 */
#define QR_VRESION_S        0
#define QR_VRESION_M        1
#define QR_VRESION_L        2

#define QR_MARGIN           4
#define	QR_VER1_SIZE        21      /*  �汾�������� */

#define MAX_ALLCODEu32      3706    /* 400  P14,P35 ��������[����]* (E) (VER:40), ��������Ϊ8λ */
#define MAX_DATACODEu32     2956    /* 400  P27     �����Ϣ����(Ver��40-L)����������Ϊ8λ */
#define MAX_CODEBLOCK       153     /* ���������� Ver��36.37.38_L_�ڶ��� */
#define MAX_MODULESIZE      177     /**
                                     * 21:Version=1,����ַ�=17(8.5������)
                                     * 25:Version=2,����ַ�=32(16������)
                                     * 29:Version=3,����ַ�=49(24.5������)
                                     * 33:Version=4,����ַ�=78(39������)
                                     * 37:Version=5,����ַ�=106(53������)
                                     * 41:Version=6,����ַ�=134(67������)
                                     * 45:Version=7,����ַ�=154(77������)
                                     * 49:Version=8,����ַ�=192(96������)
                                     * 53:
                                     * P14 ÿ�ߵ�ģ������A�� (VER:40   ) Ver��40 = 21+��Ver-1��*4
                                     */

/* QR ENCODE������ */
typedef struct
{
    int m_nSymbleSize;
    uint8_t m_byModuleData[MAX_MODULESIZE][MAX_MODULESIZE];

    int m_ncDataCodeu32Bit;
    uint8_t m_byDataCodeu32[MAX_DATACODEu32];

    int m_ncDataBlock;
    uint8_t m_byBlockMode[MAX_DATACODEu32];
    uint8_t m_nBlockLength[MAX_DATACODEu32];

    int m_ncAllCodeu32;
    uint8_t m_byAllCodeu32[MAX_ALLCODEu32];
    uint8_t m_byRSWork[MAX_CODEBLOCK];

    int m_nLevel;
    int m_nVersion;
    uint8_t m_bAutoExtent;
    int m_nMaskingNo;
} _qr_encode;

extern _qr_encode *qrx;     /* ��QR_ENCODE.C���涨��.ʹ��ǰ,���������ڴ� */

uint8_t EncodeData(char *lpsSource);
int GetEncodeVersion(int nVersion, char *lpsSource, int ncLength);
int EncodeSourceData(char *lpsSource, int ncLength, int nVerGroup);
int GetBitLength(uint8_t nMode, int ncData, int nVerGroup);
int SetBitStream(int nIndex, uint32_t wData, int ncData);
uint8_t IsNumeralData(unsigned char c);
uint8_t IsAlphabetData(unsigned char c);
uint8_t IsKanjiData(unsigned char c1, unsigned char c2);
uint8_t IsChineseData(unsigned char c1, unsigned char c2);
uint8_t AlphabetToBinaly(unsigned char c);
uint32_t KanjiToBinaly(uint32_t wc);
uint32_t ChineseToBinaly(uint32_t wc);
void GetRSCodeu32(uint8_t *lpbyRSWork, int ncDataCodeu32, int ncRSCodeu32);
void FormatModule(void);
void SetFunctionModule(void);
void SetFinderPattern(int x, int y);
void SetAlignmentPattern(int x, int y);
void SetVersionPattern(void);
void SetCodeu32Pattern(void);
void SetMaskingPattern(int nPatternNo);
void SetFormatInfoPattern(int nPatternNo);
int CountPenalty(void);

#endif


















