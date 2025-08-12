/**
 ****************************************************************************************************
 * @file        ltdc.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       LTDC 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230322
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/LCD/lcd.h"
#include "./BSP/LCD/ltdc.h"
#include "./SYSTEM/delay/delay.h"


#if !(__ARMCC_VERSION >= 6010050)   /* 不是AC6编译器，即使用AC5编译器时 */

/* 根据不同的颜色格式,定义帧缓存数组 */
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    uint32_t ltdc_lcd_framebuf[1280][800] __attribute__((at(LTDC_FRAME_BUF_ADDR)));     /* 定义最大屏分辨率时,LCD所需的帧缓存数组大小 */
#else
    uint16_t ltdc_lcd_framebuf[1280][800] __attribute__((at(LTDC_FRAME_BUF_ADDR)));     /* 定义最大屏分辨率时,LCD所需的帧缓存数组大小 */
    //uint16_t ltdc_lcd_framebuf1[1280][800] __attribute__((at(LTDC_FRAME_BUF_ADDR + 1280 * 800 * 2)));  /* 使能LTDC层2时使用(默认只使用LTDC层1) */
#endif

#else   /* 使用AC6编译器时 */

/* 根据不同的颜色格式,定义帧缓存数组 */
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    uint32_t ltdc_lcd_framebuf[1280][800] __attribute__((section(".bss.ARM.__at_0XC0000000")));     /* 定义最大屏分辨率时,LCD所需的帧缓存数组大小 */
#else
    uint16_t ltdc_lcd_framebuf[1280][800] __attribute__((section(".bss.ARM.__at_0XC0000000")));     /* 定义最大屏分辨率时,LCD所需的帧缓存数组大小 */
#endif

#endif


uint32_t *g_ltdc_framebuf[2];       /* LTDC LCD帧缓存数组指针,必须指向对应大小的内存区域 */
_ltdc_dev lcdltdc;                  /* 管理LCD LTDC的重要参数 */


/**
 * @brief       LTDC开关
 * @param       sw          : 1,打开; 0,关闭;
 * @retval      无
 */
void ltdc_switch(uint8_t sw)
{
    if (sw)
    {
        LTDC->GCR |= 1 << 0;    /* 打开LTDC */
    }
    else
    {
        LTDC->GCR &= ~(1 << 0); /* 关闭LTDC */
    }
}

/**
 * @brief       LTDC开关指定层
 * @param       layerx      : 0,第一层; 1,第二层;
 * @param       sw          : 1,打开;   0,关闭;
 * @retval      无
 */
void ltdc_layer_switch(uint8_t layerx, uint8_t sw)
{
    if (sw)
    {
        if (layerx == 0)LTDC_Layer1->CR |= 1 << 0;      /* 开启层1 */
        else LTDC_Layer2->CR |= 1 << 0;                 /* 开启层2 */
    }
    else
    {
        if (layerx == 0)LTDC_Layer1->CR &= ~(1 << 0);   /* 关闭层1 */
        else LTDC_Layer2->CR &= ~(1 << 0);              /* 关闭层2 */
    }

    LTDC->SRCR |= 1 << 0;                               /* 重新加载配置 */
}

/**
 * @brief       LTDC选择层
 * @param       layerx      : 0,第一层; 1,第二层;
 * @retval      无
 */
void ltdc_select_layer(uint8_t layerx)
{
    lcdltdc.activelayer = layerx;
}

/**
 * @brief       LTDC显示方向设置
 * @param       dir         : 0,竖屏; 1,横屏;
 * @retval      无
 */
void ltdc_display_dir(uint8_t dir)
{
    lcdltdc.dir = dir;      /* 显示方向 */

    if (dir == 0)           /* 竖屏 */
    {
        lcdltdc.width = lcdltdc.pheight;
        lcdltdc.height = lcdltdc.pwidth;
    }
    else if (dir == 1)      /* 横屏 */
    {
        lcdltdc.width = lcdltdc.pwidth;
        lcdltdc.height = lcdltdc.pheight;
    }
}

/**
 * @brief       LTDC画点函数
 * @param       x,y         : 坐标
 * @param       color       : 颜色值
 * @retval      无
 */
void ltdc_draw_point(uint16_t x, uint16_t y, uint32_t color)
{
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888

    if (lcdltdc.dir)    /* 横屏 */
    {
        *(uint32_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x)) = color;
    }
    else                /* 竖屏 */
    {
        *(uint32_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)) = color;
    }

#else

    if (lcdltdc.dir)    /* 横屏 */
    {
        *(uint16_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x)) = color;
    }
    else                /* 竖屏 */
    {
        *(uint16_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)) = color;
    }

#endif
}

/**
 * @brief       LTDC读点函数
 * @param       x,y         : 坐标
 * @retval      颜色值
 */
uint32_t ltdc_read_point(uint16_t x, uint16_t y)
{
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888

    if (lcdltdc.dir)    /* 横屏 */
    {
        return *(uint32_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x));
    }
    else                /* 竖屏 */
    {
        return *(uint32_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y));
    }

#else

    if (lcdltdc.dir)    /* 横屏 */
    {
        return *(uint16_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x));
    }
    else                /* 竖屏 */
    {
        return *(uint16_t *)((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y));
    }

#endif
}

/**
 * @brief       LTDC填充矩形, DMA2D填充
 * @note       (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 *              注意:sx,ex,不能大于lcddev.width - 1; sy,ey,不能大于lcddev.height - 1
 * @param       sx,sy       : 起始坐标
 * @param       ex,ey       : 结束坐标
 * @param       color       : 填充的颜色
 * @retval      无
 */
void ltdc_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{
    uint32_t psx, psy, pex, pey;    /* 以LCD面板为基准的坐标系,不随横竖屏变化而变化 */
    uint32_t timeout = 0;
    uint16_t offline;
    uint32_t addr;

    /* 坐标系转换 */
    if (lcdltdc.dir)    /* 横屏 */
    {
        psx = sx;
        psy = sy;
        pex = ex;
        pey = ey;
    }
    else                /* 竖屏 */
    {
        if(ex >= lcdltdc.pheight)ex = lcdltdc.pheight - 1;  /* 限制范围 */
        if(sx >= lcdltdc.pheight)sx = lcdltdc.pheight - 1;  /* 限制范围 */
        
        psx = sy;
        psy = lcdltdc.pheight - ex - 1;
        pex = ey;
        pey = lcdltdc.pheight - sx - 1;
    }

    offline = lcdltdc.pwidth - (pex - psx + 1);
    addr = ((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));
    RCC->AHB3ENR |= 1 << 4;         /* 使能DM2D时钟 */
    DMA2D->CR &= ~(1 << 0);         /* 先停止DMA2D */
    DMA2D->CR = 3 << 16;            /* 寄存器到存储器模式 */
    DMA2D->OPFCCR = LTDC_PIXFORMAT; /* 设置颜色格式 */
    DMA2D->OOR = offline;           /* 设置行偏移 */
    DMA2D->OMAR = addr;             /* 输出存储器地址 */
    DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16); /* 设定行数寄存器 */
    DMA2D->OCOLR = color;           /* 设定输出颜色寄存器 */
    DMA2D->CR |= 1 << 0;            /* 启动DMA2D */

    while ((DMA2D->ISR & (1 << 1)) == 0)    /* 等待传输完成 */
    {
        timeout++;

        if (timeout > 0X1FFFFF)break;       /* 超时退出 */
    }

    DMA2D->IFCR |= 1 << 1;          /* 清除传输完成标志 */
}

/**
 * @brief       在指定区域内填充指定颜色块, DMA2D填充
 * @note        此函数仅支持uint16_t,RGB565格式的颜色数组填充.
 *              (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 *              注意:sx,ex,不能大于lcddev.width - 1; sy,ey,不能大于lcddev.height - 1
 * @param       sx,sy       : 起始坐标
 * @param       ex,ey       : 结束坐标
 * @param       color       : 填充的颜色数组首地址
 * @retval      无
 */
void ltdc_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    uint32_t psx, psy, pex, pey;    /* 以LCD面板为基准的坐标系,不随横竖屏变化而变化 */
    uint32_t timeout = 0;
    uint16_t offline;
    uint32_t addr;

    /* 坐标系转换 */
    if (lcdltdc.dir)    /* 横屏 */
    {
        psx = sx;
        psy = sy;
        pex = ex;
        pey = ey;
    }
    else                /* 竖屏 */
    {
        psx = sy;
        psy = lcdltdc.pheight - ex - 1;
        pex = ey;
        pey = lcdltdc.pheight - sx - 1;
    }

    offline = lcdltdc.pwidth - (pex - psx + 1);
    addr = ((uint32_t)g_ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));
    RCC->AHB3ENR |= 1 << 4;             /* 使能DM2D时钟 */
    DMA2D->CR &= ~(1 << 0);             /* 先停止DMA2D */
    DMA2D->CR = 0 << 16;                /* 存储器到存储器模式 */
    DMA2D->FGPFCCR = LTDC_PIXFORMAT;    /* 设置颜色格式 */
    DMA2D->FGOR = 0;                    /* 前景层行偏移为0 */
    DMA2D->OOR = offline;               /* 设置行偏移 */
    DMA2D->FGMAR = (uint32_t)color;     /* 源地址 */
    DMA2D->OMAR = addr;                 /* 输出存储器地址 */
    DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16); /* 设定行数寄存器 */
    DMA2D->CR |= 1 << 0;                /* 启动DMA2D */

    while ((DMA2D->ISR & (1 << 1)) == 0)/* 等待传输完成 */
    {
        timeout++;

        if (timeout > 0X1FFFFF)break;   /* 超时退出 */
    }

    DMA2D->IFCR |= 1 << 1;              /* 清除传输完成标志 */
}

/**
 * @brief       LTDC清屏
 * @param       color       : 颜色值
 * @retval      无
 */
void ltdc_clear(uint32_t color)
{
    ltdc_fill(0, 0, lcdltdc.width - 1, lcdltdc.height - 1, color);
}

/**
 * @brief       LTDC时钟(Fdclk)设置函数
 * @param       pll3n     : PLL3倍频系数(PLL倍频),           取值范围:4~512.
 * @param       pll3m     : PLL3预分频系数(进PLL之前的分频),  取值范围:2~63.
 * @param       pll3r     : PLL3的r分频系数(PLL之后的分频),   取值范围:1~128.
 *
 *  @note       Fvco = Fs * (pll3n / pll3m);
 *              Fr = Fvco / pll3r = Fs * (pll3n / (pll3m * pll3r));
 *              Fdclk = Fr;
 *              其中:
 *              Fvco: VCO频率
 *              Fr: PLL3的r分频输出时钟频率
 *              Fs: PLL3输入时钟频率,可以是HSI,CSI,HSE等.
 *
 *              因为LTDC和SAI共用pll3,所以,当使用SAI的时候,会切换不同的pll3n(一般会
 *              大于300),导致LTDC的plln也变化了,因此这里设置为300,得到的像素时钟在
 *              后面SAI修改pll3n的时候,只会大,不会小.不会降低LCD的刷新频率.
 *              因此,LTDC,我们一般设置pll3m = 25, pll3n = 300, 这样,可以得到Fvco = 300Mhz
 *              然后,只 需要通过修改pll3r, 来匹配不同的液晶屏时序即可.

 *              假设:外部晶振为25M, pllm = 25 的时候, Fs = 25Mhz， pllm分频后频率 为1Mhz.
 *              例如: 要得到33M的LTDC时钟, 则可以设置: pll3n = 300, pllm = 25, pll3r = 9
 *              Fdclk= ((25 / 25) * 300) / 9 = 33 Mhz
 * @retval      0, 成功;
 *              其他, 失败;
 */
uint8_t ltdc_clk_set(uint32_t pll3n, uint32_t pll3m, uint32_t pll3r)
{
    uint16_t retry = 0;
    uint8_t status = 0;
    
    RCC->CR &= ~(1 << 28);  /* 关闭PLL3时钟 */

    while (((RCC->CR & (1 << 29))) && (retry < 0X1FFF))retry++; /* 等待PLL3时钟失锁 */

    if (retry == 0X1FFF)status = 1;         /* LTDC时钟关闭失败 */
    else
    {
        RCC->PLLCKSELR &= ~(0X3F << 20);    /* 清除DIVM3[5:0]原来的设置 */
        RCC->PLLCKSELR |= pll3m << 20;      /* DIVM3[5:0] = 25,设置PLL3的预分频系数 */
        RCC->PLL3DIVR &= ~(0X1FF << 0);     /* 清除DIVN3[8:0]原来的设置 */
        RCC->PLL3DIVR |= (pll3n - 1) << 0;  /* DIVN3[8:0] = pll3n - 1, 设置PLL3的倍频系数,设置值需减1 */
        RCC->PLL3DIVR &= ~(0X7F << 24);     /* 清除DIVR2[6:0]原来的设置 */
        RCC->PLL3DIVR |= (pll3r - 1) << 24; /* DIVR3[8:0] = pll3r - 1,设置PLL3的倍频系数,设置值需减1 */

        RCC->PLLCFGR &= ~(0X0F << 8);       /* 清除PLL3RGE[1:0]/PLL3VCOSEL/PLL3FRACEN的设置 */
        RCC->PLLCFGR |= 0 << 10;            /* PLL3RGE[1:0] = 0,PLL3输入时钟频率在1~2Mhz之间(25 / 25 = 1Mhz) */
        RCC->PLLCFGR |= 0 << 9;             /* PLL3VCOSEL = 0, PLL3的VCO范围,192~836Mhz */
        RCC->PLLCFGR |= 1 << 24;            /* DIVR3EN = 1, 使能pll3_r_ck */
        RCC->CR |= 1 << 28;                 /* PLL3ON = 1, 使能PLL3 */

        while (((RCC->CR & (1 << 29)) == 0) && (retry < 0X1FFF))retry++;    /* 等待PLL3时钟锁定 */

        if (retry == 0X1FFF)status = 2;
    }

    return status;
}

/**
 * @brief       LTDC层窗口设置, 窗口以LCD面板坐标系为基准
 * @note        此函数必须在ltdc_layer_parameter_config之后再设置.另外,当设置的窗口值不等于面板的尺
 *              寸时,GRAM的操作(读/写点函数),也要根据窗口的宽高来进行修改,否则显示不正常(本例程就未做修改).
 * @param       layerx      : 0,第一层; 1,第二层;
 * @param       sx, sy      : 起始坐标
 * @param       width,height: 宽度和高度
 * @retval      无
 */
void ltdc_layer_window_config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint32_t temp;
    uint8_t pixformat = 0;

    if (layerx == 0)
    {
        temp = (sx + width + ((LTDC->BPCR & 0X0FFF0000) >> 16)) << 16;
        LTDC_Layer1->WHPCR = (sx + ((LTDC->BPCR & 0X0FFF0000) >> 16) + 1) | temp;   /* 设置行起始和结束位置 */
        temp = (sy + height + (LTDC->BPCR & 0X7FF)) << 16;
        LTDC_Layer1->WVPCR = (sy + (LTDC->BPCR & 0X7FF) + 1) | temp;    /* 设置列起始和结束位置 */
        pixformat = LTDC_Layer1->PFCR & 0X07;                           /* 得到颜色格式 */

        if (pixformat == 0)temp = 4;                                    /* ARGB8888,一个点4个字节 */
        else if (pixformat == 1)temp = 3;                               /* RGB888,一个点3个字节 */
        else if (pixformat == 5 || pixformat == 6)temp = 1;             /* L8/AL44,一个点1个字节 */
        else temp = 2;                                                  /* 其他格式,一个点2个字节 */

        LTDC_Layer1->CFBLR = (width * temp << 16) | (width * temp + 3); /* 帧缓冲区长度设置(字节为单位) */
        LTDC_Layer1->CFBLNR = height;                                   /* 帧缓冲区行数设置 */
    }
    else
    {
        temp = (sx + width + ((LTDC->BPCR & 0X0FFF0000) >> 16)) << 16;
        LTDC_Layer2->WHPCR = (sx + ((LTDC->BPCR & 0X0FFF0000) >> 16) + 1) | temp;   /* 设置行起始和结束位置 */
        temp = (sy + height + (LTDC->BPCR & 0X7FF)) << 16;
        LTDC_Layer2->WVPCR = (sy + (LTDC->BPCR & 0X7FF) + 1) | temp;    /* 设置列起始和结束位置 */
        pixformat = LTDC_Layer2->PFCR & 0X07;                           /* 得到颜色格式 */

        if (pixformat == 0)temp = 4;                                    /* ARGB8888,一个点4个字节 */
        else if (pixformat == 1)temp = 3;                               /* RGB888,一个点3个字节 */
        else if (pixformat == 5 || pixformat == 6)temp = 1;             /* L8/AL44,一个点1个字节 */
        else temp = 2;                                                  /* 其他格式,一个点2个字节 */

        LTDC_Layer2->CFBLR = (width * temp << 16) | (width * temp + 3); /* 帧缓冲区长度设置(字节为单位) */
        LTDC_Layer2->CFBLNR = height;                                   /* 帧缓冲区行数设置 */
    }

    ltdc_layer_switch(layerx, 1);                                       /* 层使能 */
}


/**
 * @brief       LTDC层基本参数设置
 *  @note       此函数,必须在ltdc_layer_window_config之前设置.
 * @param       layerx      : 0,第一层; 1,第二层;
 * @param       bufaddr     : 层颜色帧缓存起始地址
 * @param       pixformat   : 颜色格式. 0,ARGB8888; 1,RGB888; 2,RGB565; 3,ARGB1555; 4,ARGB4444; 5,L8; 6;AL44; 7;AL88
 * @param       alpha       : 层颜色Alpha值, 0,全透明;255,不透明
 * @param       alpha0      : 默认颜色Alpha值, 0,全透明;255,不透明
 * @param       bfac1       : 混合系数1, 4(100),恒定的Alpha; 6(101),像素Alpha*恒定Alpha
 * @param       bfac2       : 混合系数2, 5(101),恒定的Alpha; 7(111),像素Alpha*恒定Alpha
 * @param       bkcolor     : 层默认颜色,32位,低24位有效,RGB888格式
 * @retval      无
 */
void ltdc_layer_parameter_config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor)
{
    if (layerx == 0)
    {
        LTDC_Layer1->CFBAR = bufaddr;                           /* 设置层颜色帧缓存起始地址 */
        LTDC_Layer1->PFCR = pixformat;                          /* 设置层颜色格式 */
        LTDC_Layer1->CACR = alpha;                              /* 设置层颜色Alpha值,255分频;设置255,则不透明 */
        LTDC_Layer1->DCCR = ((uint32_t)alpha0 << 24) | bkcolor; /* 设置默认颜色Alpha值,以及默认颜色 */
        LTDC_Layer1->BFCR = ((uint32_t)bfac1 << 8) | bfac2;     /* 设置层混合系数 */
    }
    else
    {
        LTDC_Layer2->CFBAR = bufaddr;                           /* 设置层颜色帧缓存起始地址 */
        LTDC_Layer2->PFCR = pixformat;                          /* 设置层颜色格式 */
        LTDC_Layer2->CACR = alpha;                              /* 设置层颜色Alpha值,255分频;设置255,则不透明 */
        LTDC_Layer2->DCCR = ((uint32_t)alpha0 << 24) | bkcolor; /* 设置默认颜色Alpha值,以及默认颜色 */
        LTDC_Layer2->BFCR = ((uint32_t)bfac1 << 8) | bfac2;     /* 设置层混合系数 */
    }
}

/**
 * @brief       LTDC读取面板ID
 * @note        利用LCD RGB线的最高位(R7,G7,B7)来识别面板ID
 *              PG6 = R7(M0); PI2 = G7(M1); PI7 = B7(M2);
 *              M2:M1:M0
 *              0 :0 :0     4.3 寸480*272  RGB屏,ID = 0X4342
 *              0 :0 :1     7   寸800*480  RGB屏,ID = 0X7084
 *              0 :1 :0     7   寸1024*600 RGB屏,ID = 0X7016
 *              0 :1 :1     7   寸1280*800 RGB屏,ID = 0X7018
 *              1 :0 :0     4.3 寸800*480  RGB屏,ID = 0X4348
 *              1 :0 :1     10.1寸1280*800 RGB屏,ID = 0X1018
 * @param       无
 * @retval      0, 非法; 
 *              其他, LCD ID
 */
uint16_t ltdc_panelid_read(void)
{
    uint8_t idx = 0;
    
    RCC->AHB4ENR |= 1 << 6 | 1 << 8;    /* 使能PG/PI时钟 */

    sys_gpio_set(GPIOG, SYS_GPIO_PIN6,
                 SYS_GPIO_MODE_IN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* R7引脚上拉收入 */
    
    sys_gpio_set(GPIOI, SYS_GPIO_PIN2 | SYS_GPIO_PIN7,
                 SYS_GPIO_MODE_IN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* G7, B7引脚上拉收入 */

    delay_us(10);
    idx  = sys_gpio_pin_get(GPIOG, SYS_GPIO_PIN6);      /* 读取M0 */
    idx |= sys_gpio_pin_get(GPIOI, SYS_GPIO_PIN2) << 1; /* 读取M1 */
    idx |= sys_gpio_pin_get(GPIOI, SYS_GPIO_PIN7) << 2; /* 读取M2 */

    switch (idx)
    {
        case 0:
            return 0X4342;      /* 4.3寸屏,480*272分辨率 */

        case 1:
            return 0X7084;      /* 7寸屏,800*480分辨率 */

        case 2:
            return 0X7016;      /* 7寸屏,1024*600分辨率 */

        case 3:
            return 0X7018;      /* 7寸屏,1280*800分辨率 */

        case 4:
            return 0X4384;      /* 4.3寸屏,800*480分辨率 */

        case 5:
            return 0X1018;      /* 10.1寸屏,1280*800分辨率 */

        default:
            return 0X4384;
    }
}

/**
 * @brief       LTDC初始化函数
 * @param       无
 * @retval      无
 */
void ltdc_init(void)
{
    uint32_t tempreg = 0;
    uint16_t lcdid = 0;
    
    lcdid = ltdc_panelid_read();    /* 读取LCD面板ID */

    /* 以下是LTDC信号控制引脚 BL/DE/VSYNC/HSYNC/CLK等的配置 */
    LTDC_BL_GPIO_CLK_ENABLE();      /* LTDC_BL脚时钟使能 */
    LTDC_DE_GPIO_CLK_ENABLE();      /* LTDC_DE脚时钟使能 */
    LTDC_VSYNC_GPIO_CLK_ENABLE();   /* LTDC_VSYNC脚时钟使能 */
    LTDC_HSYNC_GPIO_CLK_ENABLE();   /* LTDC_HSYNC脚时钟使能 */
    LTDC_CLK_GPIO_CLK_ENABLE();     /* LTDC_CLK脚时钟使能 */

    sys_gpio_set(LTDC_BL_GPIO_PORT, LTDC_BL_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);  /* LTDC_BL引脚模式设置(推挽输出) */
          
    sys_gpio_set(LTDC_DE_GPIO_PORT, LTDC_DE_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC_DE引脚模式设置 */

    sys_gpio_set(LTDC_VSYNC_GPIO_PORT, LTDC_VSYNC_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC_VSYNC引脚模式设置 */

    sys_gpio_set(LTDC_HSYNC_GPIO_PORT, LTDC_HSYNC_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC_HSYNC引脚模式设置 */

    sys_gpio_set(LTDC_CLK_GPIO_PORT, LTDC_CLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC_CLK引脚模式设置 */

    sys_gpio_af_set(LTDC_DE_GPIO_PORT, LTDC_DE_GPIO_PIN, 14);       /* LTDC_DE脚, AF14 */
    sys_gpio_af_set(LTDC_VSYNC_GPIO_PORT, LTDC_VSYNC_GPIO_PIN, 14); /* LTDC_VSYNC脚, AF14 */
    sys_gpio_af_set(LTDC_HSYNC_GPIO_PORT, LTDC_HSYNC_GPIO_PIN, 14); /* LTDC_HSYNC脚, AF14 */
    sys_gpio_af_set(LTDC_CLK_GPIO_PORT, LTDC_CLK_GPIO_PIN, 14);     /* LTDC_CLK脚, AF14 */

    /* 以下是LTDC 数据引脚的配置 */
    RCC->APB3ENR |= 1 << 3;             /* 开启LTDC时钟 */
    RCC->AHB4ENR |= 0X7 << 6;           /* 使能PG/PH/PI时钟 */
    
    sys_gpio_set(GPIOG, SYS_GPIO_PIN6 | SYS_GPIO_PIN11,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC 数据引脚模式设置 */
                 
    sys_gpio_set(GPIOH, 0X7F << 9,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC 数据引脚模式设置 */

    sys_gpio_set(GPIOI, 7 << 0 | 0XF << 4,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC 数据引脚模式设置 */

    sys_gpio_af_set(GPIOG, SYS_GPIO_PIN6 | SYS_GPIO_PIN11, 14);     /* LTDC 数据脚, AF14 */
    sys_gpio_af_set(GPIOH, 0X7F << 9, 14);                          /* LTDC 数据脚, AF14 */
    sys_gpio_af_set(GPIOI, 7 << 0 | 0XF << 4, 14);                  /* LTDC 数据脚, AF14 */

    if (lcdid == 0X4342)
    {
        lcdltdc.pwidth = 480;       /* 面板宽度,单位:像素 */
        lcdltdc.pheight = 272;      /* 面板高度,单位:像素 */
        lcdltdc.hsw = 1;            /* 水平同步宽度 */
        lcdltdc.vsw = 1;            /* 垂直同步宽度 */
        lcdltdc.hbp = 40;           /* 水平后廊 */
        lcdltdc.vbp = 8;            /* 垂直后廊 */
        lcdltdc.hfp = 5;            /* 水平前廊 */
        lcdltdc.vfp = 8;            /* 垂直前廊 */
        ltdc_clk_set(300, 25, 33);  /* 设置像素时钟  9Mhz */
    }
    else if (lcdid == 0X7084)
    {
        lcdltdc.pwidth = 800;       /* 面板宽度,单位:像素 */
        lcdltdc.pheight = 480;      /* 面板高度,单位:像素 */
        lcdltdc.hsw = 1;            /* 水平同步宽度 */
        lcdltdc.vsw = 1;            /* 垂直同步宽度 */
        lcdltdc.hbp = 46;           /* 水平后廊 */
        lcdltdc.vbp = 23;           /* 垂直后廊 */
        lcdltdc.hfp = 210;          /* 水平前廊 */
        lcdltdc.vfp = 22;           /* 垂直前廊 */
        ltdc_clk_set(300, 25, 9);   /* 设置像素时钟 33M(如果开双显,需要降低DCLK到:18.75Mhz  300/4/4,才会比较好) */
    }
    else if (lcdid == 0X7016)
    {
        lcdltdc.pwidth = 1024;      /* 面板宽度,单位:像素 */
        lcdltdc.pheight = 600;      /* 面板高度,单位:像素 */
        lcdltdc.hsw = 20;           /* 水平同步宽度 */
        lcdltdc.vsw = 3;            /* 垂直同步宽度 */
        lcdltdc.hbp = 140;          /* 水平后廊 */
        lcdltdc.vbp = 20;           /* 垂直后廊 */
        lcdltdc.hfp = 160;          /* 水平前廊 */
        lcdltdc.vfp = 12;           /* 垂直前廊 */
        ltdc_clk_set(300, 25, 6);   /* 设置像素时钟  50Mhz */
    }
    else if (lcdid == 0X7018)
    {
        lcdltdc.pwidth = 1280;      /* 面板宽度,单位:像素 */
        lcdltdc.pheight = 800;      /* 面板高度,单位:像素 */
        /* 其他参数待定 */
    }
    else if (lcdid == 0X4384)
    {
        lcdltdc.pwidth = 800;                   /* 面板宽度,单位:像素 */
        lcdltdc.pheight = 480;                  /* 面板高度,单位:像素 */
        lcdltdc.hbp = 8;                       /* 水平后廊 */
        lcdltdc.hfp = 8;                       /* 水平前廊 */
        lcdltdc.hsw = 4;                       /* 水平同步宽度 */
        lcdltdc.vbp = 16;                       /* 垂直后廊 */
        lcdltdc.vfp = 16;                       /* 垂直前廊 */
        lcdltdc.vsw = 4;                        /* 垂直同步宽度 */
        ltdc_clk_set(300, 25, 12);               /* 设置像素时钟 25M */
    }
    else if (lcdid == 0X1018)       /* 10.1寸1280*800 RGB屏 */
    {
        lcdltdc.pwidth = 1280;      /* 面板宽度,单位:像素 */
        lcdltdc.pheight = 800;      /* 面板高度,单位:像素 */
        lcdltdc.hbp = 140;          /* 水平后廊 */
        lcdltdc.hfp = 10;           /* 水平前廊 */
        lcdltdc.hsw = 10;           /* 水平同步宽度 */
        lcdltdc.vbp = 10;           /* 垂直后廊 */
        lcdltdc.vfp = 10;           /* 垂直前廊 */
        lcdltdc.vsw = 3;            /* 垂直同步宽度 */
        ltdc_clk_set(300, 25, 6);   /* 设置像素时钟  50Mhz */
    }

    if (lcdid == 0X1018)
    {
        tempreg = 1 << 28;          /* 像素时钟极性:反向 */
    }else
    {
        tempreg = 0 << 28;          /* 像素时钟极性:不反向 */
    }

    tempreg |= 0 << 29;             /* 数据使能极性:低电平有效 */
    tempreg |= 0 << 30;             /* 垂直同步极性:低电平有效 */
    tempreg |= 0 << 31;             /* 水平同步极性:低电平有效 */
    LTDC->GCR = tempreg;            /* 设置全局控制寄存器 */
    tempreg = (lcdltdc.vsw - 1) << 0;   /* 垂直脉宽-1 */
    tempreg |= (lcdltdc.hsw - 1) << 16; /* 水平脉宽-1 */
    LTDC->SSCR = tempreg;               /* 设置同步大小配置寄存器 */

    tempreg = (lcdltdc.vsw + lcdltdc.vbp - 1) << 0;     /* 累加垂直后沿=垂直脉宽+垂直后沿-1 */
    tempreg |= (lcdltdc.hsw + lcdltdc.hbp - 1) << 16;   /* 累加水平后沿=水平脉宽+水平后沿-1 */
    LTDC->BPCR = tempreg;                               /* 设置后沿配置寄存器 */

    tempreg = (lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight - 1) << 0;   /* 累加有效高度=垂直脉宽+垂直后沿+垂直分辨率-1 */
    tempreg |= (lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth - 1) << 16;  /* 累加有效宽度=水平脉宽+水平后沿+水平分辨率-1 */
    LTDC->AWCR = tempreg;                                               /* 设置有效宽度配置寄存器 */

    tempreg = (lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight + lcdltdc.vfp - 1) << 0;     /* 总高度=垂直脉宽+垂直后沿+垂直分辨率+垂直前廊-1 */
    tempreg |= (lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth + lcdltdc.hfp - 1) << 16;    /* 总宽度=水平脉宽+水平后沿+水平分辨率+水平前廊-1 */
    LTDC->TWCR = tempreg;   /* 设置总宽度配置寄存器 */

    LTDC->BCCR = LTDC_BACKLAYERCOLOR;   /* 设置背景层颜色寄存器(RGB888格式) */
    ltdc_switch(1);         /* 开启LTDC */

#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    g_ltdc_framebuf[0] = (uint32_t *)&ltdc_lcd_framebuf;
    lcdltdc.pixsize = 4;    /* 每个像素占4个字节 */
#else
    g_ltdc_framebuf[0] = (uint32_t *)&ltdc_lcd_framebuf;
    //g_ltdc_framebuf[1]=(uint32_t*)&ltdc_lcd_framebuf1;
    lcdltdc.pixsize = 2;    /* 每个像素占2个字节 */
#endif

    /* 层配置 */
    ltdc_layer_parameter_config(0, (uint32_t)g_ltdc_framebuf[0], LTDC_PIXFORMAT, 255, 0, 6, 7, 0X000000);   /* 层参数配置 */
    ltdc_layer_window_config(0, 0, 0, lcdltdc.pwidth, lcdltdc.pheight);                                     /* 层窗口配置,以LCD面板坐标系为基准,不要随便修改! */

    //ltdc_layer_parameter_config(1,(uint32_t)g_ltdc_framebuf[1],LTDC_PIXFORMAT,127,0,6,7,0X000000);        /* 层参数配置 */
    //ltdc_layer_window_config(1,0,0,lcdltdc.pwidth,lcdltdc.pheight);                                       /* 层窗口配置,以LCD面板坐标系为基准,不要随便修改! */

    lcddev.width = lcdltdc.pwidth;      /* 设置lcddev的宽度参数 */
    lcddev.height = lcdltdc.pheight;    /* 设置lcddev的宽度参数 */
    //ltdc_display_dir(0);              /* 默认竖屏，在LCD_Init函数里面设置 */
    ltdc_select_layer(0);               /* 选择第1层 */
    LTDC_BL(1);                         /* 点亮背光 */
    ltdc_clear(0XFFFFFFFF);             /* 清屏 */
}
























