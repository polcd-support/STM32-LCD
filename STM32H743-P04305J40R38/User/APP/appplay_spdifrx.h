#ifndef __APPPLAY_SPDIFRX_H
#define __APPPLAY_SPDIFRX_H

#include "common.h"  
#include "./BSP/SPDIF/spdif.h"


#define	SPDIF_DBUF_SIZE                 1024    //定义SPDIF数据接收缓冲区的大小,1K*4字节

extern uint8_t*const SPDIF_RECORD_PIC[3];

void spdif_show_samplerate(uint16_t y,uint32_t samplerate,uint8_t fsr);
void sai_dma_tx_callback(void);
void spdif_rx_stopplay_callback(void);
uint8_t spdif_vu_get(uint16_t signallevel);
void spdif_vu_meter(uint8_t width,uint16_t x,uint16_t y,uint8_t level); 
uint8_t appplay_spdifrx(uint8_t* caption);
#endif



