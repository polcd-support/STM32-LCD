/**
 ****************************************************************************************************
 * @file        ftl.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       NAND FLASH FTL层算法 代码
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

#ifndef __FTL_H
#define __FTL_H

#include "./SYSTEM/sys/sys.h"

/**
 *坏块搜索控制
 * 如果设置为1,将在ftl_format的时候,搜寻坏块,耗时久(512M,3分钟以上),且会导致RGB屏乱闪
 */
#define FTL_USE_BAD_BLOCK_SEARCH        0       /* 定义是否使用坏块搜索 */



uint8_t ftl_init(void); 
void ftl_badblock_mark(uint32_t blocknum);
uint8_t ftl_check_badblock(uint32_t blocknum); 
uint8_t ftl_used_blockmark(uint32_t blocknum);
uint32_t ftl_find_unused_block(uint32_t sblock, uint8_t flag);
uint32_t ftl_find_same_plane_unusedBlock(uint32_t sblock);
uint8_t ftl_copy_and_write_to_block(uint32_t source_pagenum, uint16_t colnum, uint8_t *pbuffer, uint32_t numbytetowrite);
uint16_t ftl_lbn_to_pbn(uint32_t lbnnum); 
uint8_t ftl_write_sectors(uint8_t *pbuffer, uint32_t sectorno, uint16_t sectorsize, uint32_t sectorcount);
uint8_t ftl_read_sectors(uint8_t *pbuffer, uint32_t sectorno, uint16_t sectorsize, uint32_t sectorcount);
uint8_t ftl_create_lut(uint8_t mode);
uint8_t ftl_blockcompare(uint32_t blockx, uint32_t cmpval);
uint32_t ftl_search_badblock(void);
uint8_t ftl_format(void); 

#endif













