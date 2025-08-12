/**
 ****************************************************************************************************
 * @file        nand.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       NAND FLASH 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230324
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef _NAND_H
#define _NAND_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/
/* NAND_RB 引脚 定义 */

#define NAND_RB_GPIO_PORT               GPIOD
#define NAND_RB_GPIO_PIN                SYS_GPIO_PIN6
#define NAND_RB_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 3; }while(0)   /* PD口时钟使能 */

/******************************************************************************************/

/* IO操作函数 */
#define NAND_RB         sys_gpio_pin_get(NAND_RB_GPIO_PORT, NAND_RB_GPIO_PIN)   /* NAND Flash的闲/忙引脚 */



#define NAND_MAX_PAGE_SIZE          4096        /* 定义NAND FLASH的最大的PAGE大小（不包括SPARE区），默认4096字节 */
#define NAND_ECC_SECTOR_SIZE        512         /* 执行ECC计算的单元大小，默认512字节 */


/* NAND FLASH操作相关延时函数 */
#define NAND_TADL_DELAY             30          /* tADL等待延迟,最少70ns */
#define NAND_TWHR_DELAY             25          /* tWHR等待延迟,最少60ns */
#define NAND_TRHW_DELAY             35          /* tRHW等待延迟,最少100ns */
#define NAND_TPROG_DELAY            200         /* tPROG等待延迟,典型值200us,最大需要700us */
#define NAND_TBERS_DELAY            4           /* tBERS等待延迟,典型值3.5ms,最大需要10ms */


/* NAND属性结构体 */
typedef struct
{
    uint16_t page_totalsize;        /* 每页总大小，main区和spare区总和 */
    uint16_t page_mainsize;         /* 每页的main区大小 */
    uint16_t page_sparesize;        /* 每页的spare区大小 */
    uint8_t  block_pagenum;         /* 每个块包含的页数量 */
    uint16_t plane_blocknum;        /* 每个plane包含的块数量 */
    uint16_t block_totalnum;        /* 总的块数量 */
    uint16_t good_blocknum;         /* 好块数量 */
    uint16_t valid_blocknum;        /* 有效块数量(供文件系统使用的好块数量) */
    uint32_t id;                    /* NAND FLAS*(vu8*)(0X80000000|(1<<17))=CMD；*(vu8*)(0X80000000|(1<<17))=CMD；H ID */
    uint16_t *lut;                  /* LUT表，用作逻辑块-物理块转换 */
    uint32_t ecc_hard;              /* 硬件计算出来的ECC值 */
    uint32_t ecc_hdbuf[NAND_MAX_PAGE_SIZE / NAND_ECC_SECTOR_SIZE]; /* ECC硬件计算值缓冲区 */
    uint32_t ecc_rdbuf[NAND_MAX_PAGE_SIZE / NAND_ECC_SECTOR_SIZE]; /* ECC读取的值缓冲区 */
}nand_attriute;      

extern nand_attriute nand_dev;              /* nand重要参数结构体 */



#define NAND_ADDRESS            0X80000000  /* nand flash的访问地址,接NCE3,地址为:0X8000 0000 */
#define NAND_CMD                1 << 16     /* 发送命令 */
#define NAND_ADDR               1 << 17     /* 发送地址 */

/* NAND FLASH命令 */
#define nand_readID             0X90        /* 读ID指令 */
#define NAND_FEATURE            0XEF        /* 设置特性指令 */
#define NAND_RESET              0XFF        /* 复位NAND */
#define NAND_READSTA            0X70        /* 读状态 */
#define NAND_AREA_A             0X00   
#define NAND_AREA_TRUE1         0X30  
#define NAND_WRITE0             0X80
#define NAND_WRITE_TURE1        0X10
#define NAND_ERASE0             0X60
#define NAND_ERASE1             0XD0
#define NAND_MOVEDATA_CMD0      0X00
#define NAND_MOVEDATA_CMD1      0X35
#define NAND_MOVEDATA_CMD2      0X85
#define NAND_MOVEDATA_CMD3      0X10

/* NAND FLASH状态 */
#define NSTA_READY              0X40        /* nand已经准备好 */
#define NSTA_ERROR              0X01        /* nand错误 */
#define NSTA_TIMEOUT            0X02        /* 超时 */
#define NSTA_ECC1BITERR         0X03        /* ECC 1bit错误 */
#define NSTA_ECC2BITERR         0X04        /* ECC 2bit以上错误 */


/* NAND FLASH型号和对应的ID号 */
#define MT29F4G08ABADA          0XDC909556  /* MT29F4G08ABADA */
#define MT29F16G08ABABA         0X48002689  /* MT29F16G08ABABA */


uint8_t nand_init(void);
uint8_t nand_modeset(uint8_t mode);
uint32_t nand_readid(void);
uint8_t nand_readstatus(void);
uint8_t nand_wait_for_ready(void);
uint8_t nand_reset(void);
uint8_t nand_waitrb(__IO uint8_t rb);
void nand_delay(__IO uint32_t i);
uint8_t nand_readpage(uint32_t pagenum, uint16_t colnum, uint8_t *pbuffer, uint16_t numbytetoread);
uint8_t nand_readpagecomp(uint32_t PageNum, uint16_t ColNum, uint32_t CmpVal, uint16_t NumByteToRead, uint16_t *NumByteEqual);
uint8_t nand_writepage(uint32_t pagenum, uint16_t colnum, uint8_t *pbuffer, uint16_t numbytetowrite);
uint8_t nand_write_pageconst(uint32_t pagenum, uint16_t colnum, uint32_t cval, uint16_t numbytetowrite);
uint8_t nand_copypage_withoutwrite(uint32_t source_pagenum, uint32_t dest_pagenum);
uint8_t nand_copypage_withwrite(uint32_t source_pagenum, uint32_t dest_pagenum, uint16_t colnum, uint8_t *pbuffer, uint16_t numbytetowrite);
uint8_t nand_readspare(uint32_t pagenum, uint16_t colnum, uint8_t *pbuffer, uint16_t numbytetoread);
uint8_t nand_writespare(uint32_t pagenum, uint16_t colnum, uint8_t *pbuffer, uint16_t numbytetowrite);
uint8_t nand_eraseblock(uint32_t blocknum);
void nand_erasechip(void);

uint16_t nand_ecc_get_oe(uint8_t oe, uint32_t eccval);
uint8_t nand_ecc_correction(uint8_t* data_buf, uint32_t eccrd, uint32_t ecccl);

#endif



















