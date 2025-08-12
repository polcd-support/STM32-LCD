/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-01-07
 * @brief       综合测试 实验
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H473开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./BSP/USART3/usart3.h"
#include "./BSP/ST480MC/st480mc.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LED/led.h"
#include "./PICTURE/gif.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./BSP/ADC/adc.h"
#include "./USMART/usmart.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/24CXX/24cxx.h"
#include "./PICTURE/piclib.h"
#include "./BSP/TPAD/tpad.h"
#include "./BSP/AP3216C/ap3216c.h"
#include "./BSP/SH3001/sh3001.h"
#include "./BSP/NAND/nand.h"
#include "./TEXT/fonts.h"
#include "./BSP/NAND/ftl.h"
#include "./BSP/ADC/adc3.h"
#include "./BSP/MPU/mpu.h"
#include "os.h"
#include "spb.h"
#include "common.h"
#include "ebook.h"
#include "picviewer.h"
#include "settings.h"
#include "calendar.h"
#include "audioplay.h"
#include "videoplay.h"
#include "recorder.h"
#include "usbplay.h"
#include "nesplay.h"
#include "notepad.h"
#include "paint.h"
#include "exeplay.h"
#include "camera.h"
#include "wirelessplay.h"
#include "calculator.h"
#include "vmeterplay.h"
#include "phoneplay.h"
#include "appplay.h"
#include "netplay.h"
#include "usb_app.h"
#include "keyplay.h"
#include "ledplay.h"
#include "beepplay.h"
#include "facereco.h"
#include "qrplay.h"
#include "webcamera.h"
#include "recorder.h"
#include "gyroscope.h"
#include "gradienter.h"
#include "smsplay.h"


#if !(__ARMCC_VERSION >= 6010050)       /* 不是AC6编译器，即使用AC5编译器时 */
#define __ALIGNED_8     __align(8)      /* AC5使用这个 */
#else                                   /* 使用AC6编译器时 */
#define __ALIGNED_8     __ALIGNED(8)    /* AC6使用这个 */
#endif

extern volatile uint8_t system_task_return; /* 任务强制返回标志 */

/******************************************************************************************/
/* UCOSII任务设置 */

/* START 任务 配置 */
#define START_TASK_PRIO                 10                  /* 开始任务的优先级设置为最低 */
#define START_STK_SIZE                  128                 /* 堆栈大小 */

__ALIGNED_8 static OS_STK START_TASK_STK[START_STK_SIZE];   /* 任务堆栈,8字节对齐 */
void start_task(void *pdata);                               /* 任务函数 */


/* 串口 任务 配置 */
#define USART_TASK_PRIO                 7                   /* 任务优先级 */
#define USART_STK_SIZE                  128                 /* 堆栈大小 */

__ALIGNED_8 static OS_STK USART_TASK_STK[USART_STK_SIZE];   /* 任务堆栈,8字节对齐 */
void usart_task(void *pdata);                               /* 任务函数 */


/* 主 任务 配置 */
#define MAIN_TASK_PRIO                  6                  /* 任务优先级 */
#define MAIN_STK_SIZE                   1200                /* 堆栈大小 */

__ALIGNED_8 static OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];     /* 任务堆栈,8字节对齐 */
void main_task(void *pdata);                                /* 任务函数 */


/* 监视 任务 配置 */
#define WATCH_TASK_PRIO                 3                   /* 任务优先级 */
#define WATCH_STK_SIZE                  256                 /* 堆栈大小 */

__ALIGNED_8 static OS_STK WATCH_TASK_STK[WATCH_STK_SIZE];   /* 任务堆栈,8字节对齐 */
void watch_task(void *pdata);                               /* 任务函数 */

/**
 * @brief       外部内存测试(最大支持1M字节内存测试)
 * @param       x, y : 坐标
 * @param       fsize: 字体大小
 * @retval      0,成功; 1,失败.
 */
uint8_t system_exsram_test(uint16_t x, uint16_t y, uint8_t fsize)
{
    uint32_t i = 0;
    uint16_t j = 0;
    uint32_t temp = 0;
    uint32_t sval = 0;  /* 在地址0读到的数据 */
    uint32_t *tempbuf;
    tempbuf = mymalloc(SRAMIN,1024 * 4);
    lcd_show_string(x, y, lcddev.width, y + fsize, fsize, "Ex Memory Test:    0KB", WHITE);

    /* 每隔1K字节,写入一个数据,总共写入1024个数据,刚好是1M字节 */
    for(i = 0;i < 32 * 1024 * 1024;i += 32 * 1024)
    {
        tempbuf[temp]=*(volatile uint32_t*)(BANK5_SDRAM_ADDR + i);/* 保存原来的数据 */
        delay_us(1);
        *(volatile uint32_t*)(BANK5_SDRAM_ADDR + i) = temp; 
        temp++;
    }

    /* 依次读出之前写入的数据,进行校验 */
    for(i = 0;i < 32 * 1024 * 1024;i += 32 * 1024)
    {
        temp=*(volatile uint32_t*)(BANK5_SDRAM_ADDR + i);
        delay_us(1);
        *(volatile uint32_t*)(BANK5_SDRAM_ADDR + i) = tempbuf[j++]; 

        if (i == 0)sval = temp;
        else if (temp <= sval)break;    /* 后面读出的数据一定要比第一次读到的数据大 */

        lcd_show_xnum(x + 15 * (fsize / 2), y, (uint16_t)(temp - sval + 1) * 32, 5, fsize, 0, WHITE); /* 显示内存容量 */
    }

    myfree(SRAMIN,tempbuf);             /* 释放内存 */
    
    if (i >= 1024 * 1024)
    {
//        lcd_show_xnum(x + 15 * (fsize / 2), y, i / 1024, 4, fsize, 0, WHITE);   /* 显示内存值 */
        return 0;   /* 内存正常,成功 */
    }

    return 1;       /* 失败 */
}

/**
 * @brief       显示错误信息,停止运行,持续提示错误信息
 * @param       x, y : 坐标
 * @param       err  : 错误信息
 * @param       fsize: 字体大小
 * @retval      无
 */
void system_error_show(uint16_t x, uint16_t y, uint8_t *err, uint8_t fsize)
{
    uint8_t ledr = 1;

    while (1)
    {
        lcd_show_string(x, y, lcddev.width, lcddev.height, fsize, (char *)err, RED);
        delay_ms(400);
        lcd_fill(x, y, lcddev.width - 1, y + fsize, BLACK);
        delay_ms(100);
        LED0(ledr ^= 1);
    }
}

/**
 * @brief       显示错误信息, 显示以后(2秒), 继续运行
 * @param       x, y : 坐标
 * @param       fsize: 字体大小
 * @param       str  : 字符串
 * @retval      无
 */
void system_error_show_pass(uint16_t x, uint16_t y, uint8_t fsize, uint8_t *str)
{
    pcf8574_write_bit(BEEP_IO,0);
    lcd_show_string(x, y, lcddev.width, lcddev.height, fsize, (char *)str, RED);
    delay_ms(2000);
    pcf8574_write_bit(BEEP_IO,1);
}

/**
 * @brief       擦除整个SPI FLASH(即所有资源都删除),以快速更新系统.
 * @param       x, y : 坐标
 * @param       fsize: 字体大小
 * @retval      0,没有擦除; 1,擦除了;
 */
uint8_t system_files_erase(uint16_t x, uint16_t y, uint8_t fsize)
{
    uint8_t key;
    uint8_t t = 0;

    lcd_show_string(x, y, lcddev.width, lcddev.height, fsize, "Erase all system files?", RED);

    while (1)
    {
        t++;

        if (t == 20)lcd_show_string(x, y + fsize, lcddev.width, lcddev.height, fsize, "KEY0:NO / KEY1:YES", RED);

        if (t == 40)
        {
            gui_fill_rectangle(x, y + fsize, lcddev.width, fsize, BLACK); /* 清除显示 */
            t = 0;
            LED0_TOGGLE();
        }

        key = key_scan(0);

        if (key == KEY0_PRES)   /* 不擦除,用户取消了 */
        {
            gui_fill_rectangle(x, y, lcddev.width, fsize * 2, BLACK); /* 清除显示 */
            g_point_color = WHITE;
            LED0(1);
            return 0;
        }

        if (key == KEY1_PRES)   /* 要擦除,要重新来过 */
        {
            LED0(1);
            lcd_show_string(x, y + fsize, lcddev.width, lcddev.height, fsize, "Erasing SPI FLASH...", RED);

            norflash_erase_chip();  /* 全片擦除 效率高 */

            lcd_show_string(x, y + fsize, lcddev.width, lcddev.height, fsize, "Erasing SPI FLASH OK      ", RED);
            delay_ms(600);
            return 1;
        }

        delay_ms(10);
    }
}

/**
 * @brief       字库更新确认提示.
 * @param       x, y : 坐标
 * @param       fsize: 字体大小
 * @retval      0,不需要更新; 1,确认要更新;
 */
uint8_t system_font_update_confirm(uint16_t x, uint16_t y, uint8_t fsize)
{
    uint8_t ledr = 1;
    uint8_t key;
    uint8_t t = 0;
    uint8_t res = 0;
    g_point_color = RED;
    lcd_show_string(x, y, lcddev.width, lcddev.height, fsize, "Update font?", RED);

    while (1)
    {
        t++;

        if (t == 20)lcd_show_string(x, y + fsize, lcddev.width, lcddev.height, fsize, "KEY0:NO / KEY1:YES", RED);

        if (t == 40)
        {
            gui_fill_rectangle(x, y + fsize, lcddev.width, fsize, BLACK); /* 清除显示 */
            t = 0;
            LED0(ledr ^= 1);
        }

        key = key_scan(0);

        if (key == KEY0_PRES)break; /* 不更新 */

        if (key == KEY1_PRES)
        {
            res = 1;    /* 要更新 */
            break;
        }

        delay_ms(10);
    }

    LED0(1);
    gui_fill_rectangle(x, y, lcddev.width, fsize * 2, BLACK); /* 清除显示 */
    g_point_color = WHITE;
    return res;
}

uint8_t g_tpad_failed_flag = 0;     /* TPAD失效标记，如果失效，则使用WK_UP作为退出按键 */

/**
 * @brief       系统初始化
 * @param       无
 * @retval      无
 */
void system_init(void)
{
    uint16_t okoffset = 162;
    uint16_t ypos = 0;
    uint16_t j = 0;
    uint16_t temp = 0;
    uint8_t res;
    uint32_t dtsize,dfsize;
    uint8_t *stastr = 0;
    uint8_t *version = 0; 
    uint8_t verbuf[12];
    uint8_t fsize;
    uint8_t icowidth;
    
    sys_stm32_clock_init(160,5,2,4);        /* 设置时钟,400Mhz */
    delay_init(400);                        /* 延时初始化 */
    exeplay_app_check();                    /* 检测是否需要运行APP代码 */
    usart_init(100,115200);                 /* 串口初始化为115200 */
    usart3_init(100,115200);                /* 初始化串口3波特率为115200 */
    usmart_dev.init(200);                   /* 初始化USMART */
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化KEY */
    at24cxx_init();                         /* EEPROM初始化 */
    norflash_init();                        /* 初始化SPI FLASH */
    adc_init();                             /* 初始化ADC采集 */
    adc3_init();                            /* 初始化内部温度传感器 */
    pcf8574_init();                         /* PCF8574初始化 */
    my_mem_init(SRAMIN);                    /* 初始化内部内存池 */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池 */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */
    tp_dev.init();                          /* 触摸屏初始化 */
    gui_init();
    piclib_init();                          /* piclib初始化 */
    slcd_dma_init();                        /* SLCD DMA初始化 */
    usbapp_init();
    exfuns_init();                          /* FATFS 申请内存 */

    version = mymalloc(SRAMIN, 31);         /* 申请31个字节内存 */

REINIT: /* 重新初始化 */

    lcd_clear(BLACK);                       /* 黑屏 */
    g_point_color = WHITE;
    g_back_color = BLACK;
    j = 0;

    /* 显示版权信息 */
    ypos = 2;

    if (lcddev.width <= 272)
    {
        fsize = 12;
        icowidth = 24;
        okoffset = 190;
        app_show_mono_icos(5, ypos, icowidth, 24, (uint8_t *)APP_ALIENTEK_ICO2424, YELLOW, BLACK);
    }
    else if (lcddev.width == 320)
    {
        fsize = 16;
        icowidth = 32;
        okoffset = 250;
        app_show_mono_icos(5, ypos, icowidth, 32, (uint8_t *)APP_ALIENTEK_ICO3232, YELLOW, BLACK);
    }
    else if (lcddev.width >= 480)
    {
        fsize = 24;
        icowidth = 48;
        okoffset = 370;
        app_show_mono_icos(5, ypos, icowidth, 48, (uint8_t *)APP_ALIENTEK_ICO4848, YELLOW, BLACK);
    }

    lcd_show_string(icowidth + 5 * 2, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "ALIENTEK Apollo STM32F4/F7/H7", g_point_color);
    lcd_show_string(icowidth + 5 * 2, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "Copyright (C) 2022-2032", g_point_color);
    app_get_version(verbuf, HARDWARE_VERSION, 2);
    strcpy((char *)version, "HARDWARE:");
    strcat((char *)version, (const char *)verbuf);
    strcat((char *)version, ", SOFTWARE:");
    app_get_version(verbuf, SOFTWARE_VERSION, 3);
    strcat((char *)version, (const char *)verbuf);
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, (char *)version, g_point_color);
    sprintf((char *)verbuf, "LCD ID:%04X", lcddev.id);  /* LCD ID打印到verbuf里面 */
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, (char *)verbuf, g_point_color);  /* 显示LCD ID */

    /* 开始硬件检测初始化 */
    es8388_init();          /* 防止喇叭乱叫 */
    app_es8388_volset(0);   /* 关闭音量输出 */

    LED0(0);
    LED1(0);    /* 同时点亮2个LED */
    
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "CPU:STM32H743IIT6 400Mhz", g_point_color);
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "FLASH:2048KB  SRAM:1060KB", g_point_color);
    LED0(1);
    LED1(1);    /* 同时关闭2个LED */

    /* 外部SRAM检测 */
    if (system_exsram_test(5, ypos + fsize * j, fsize)) system_error_show(5, ypos + fsize * j++, "EX Memory Error!", fsize);
    lcd_show_string(5 + okoffset,ypos + fsize * j++,lcddev.width,lcddev.height,fsize,"OK",g_point_color);
    my_mem_init(SRAMEX);    /* SRAM检测后,会搞乱内存分配,得重新初始化内存管理 */

    LED0(1);
    LED1(1);    /* 同时关闭2个LED */
    
    if(nand_init())         /* 检测不到NAND FLASH */
    {
        system_error_show(5,ypos + fsize * j++,"NAND Flash Error!!",fsize); 
    }
    else
    {
        temp = (nand_dev.block_totalnum / 1024) * (nand_dev.page_mainsize / 1024) * nand_dev.block_pagenum; /* NAND容量 */
    }
    
    lcd_show_string(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "NAND Flash:    MB", g_point_color);
    lcd_show_xnum(5+11*(fsize/2),ypos+fsize*j,temp,4,fsize,0, g_point_color);                               /* 显示nand flash大小 */
    lcd_show_string(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK", g_point_color);  
    
    /* SPI FLASH检测 */
    if (norflash_read_id() != W25Q256)  /* 读取QSPI FLASH ID */
    {
        system_error_show(5, ypos + fsize * j++, "SPI Flash Error!!", fsize);
    }
    else temp = 32 * 1024;  /* 32M字节大小 */

    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SPI Flash:     KB", g_point_color);
    lcd_show_xnum(5 + 10 * (fsize / 2), ypos + fsize * j, temp, 5, fsize, 0, g_point_color); /* 显示spi flash大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* 检测是否需要擦除SPI FLASH? */
    res = key_scan(1);

    if (res == WKUP_PRES)   /* 启动的时候，按下WKUP按键，则擦除SPI FLASH字库和文件系统区域 */
    {
        res = system_files_erase(5, ypos + fsize * j, fsize);

        if (res)goto REINIT;
    }

    /* RTC检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "RTC Check...", g_point_color);

    if (rtc_init())system_error_show_pass(5 + okoffset, ypos + fsize * j++, fsize, "ERROR"); /* RTC检测 */
    else
    {
        calendar_get_time(&calendar);/* 得到当前时间 */
        calendar_get_date(&calendar);/* 得到当前日期 */
        lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);
    }

    /* 检查SPI FLASH的文件系统 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "FATFS Check...", g_point_color); /* FATFS检测 */
    f_mount(fs[0], "0:", 1);                    /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                    /* 挂载SPI FLASH */
    f_mount(fs[2], "2:", 1);                    /* 挂载NAND FLASH */
    delay_ms(100);
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* SD卡检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SD Card:     MB", g_point_color); /* FATFS检测 */
    temp = 0;

    do
    {
        temp ++;
        res = exfuns_get_free("0:", &dtsize, &dfsize);  /* 得到SD卡剩余容量和总容量 */
        delay_ms(200);
    } while (res && temp < 5); /* 连续检测5次 */

    if (res == 0)   /* 得到容量正常 */
    {
        gui_phy.memdevflag |= 1 << 0;   /* 设置SD卡在位 */
        temp = dtsize >> 10; /* 单位转换为MB */
        stastr = "OK";
    }
    else
    {
        temp = 0; /* 出错了,单位为0 */
        stastr = "ERROR";
    }

    lcd_show_xnum(5 + 8 * (fsize / 2), ypos + fsize * j, temp, 5, fsize, 0, g_point_color);	/* 显示SD卡容量大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, (char *)stastr, g_point_color);   /* SD卡状态 */
    
    
    /* 25Q128检测,如果不存在文件系统,则先创建 */
    temp = 0;

    do
    {
        temp ++;
        res = exfuns_get_free("1:", &dtsize, &dfsize); /* 得到FLASH剩余容量和总容量 */
        delay_ms(200);
    } while (res && temp < 20); /* 连续检测20次 */

    if (res == 0X0D)   /* 文件系统不存在 */
    {
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SPI Flash Disk Formatting...", g_point_color); /* 格式化FLASH */
        res = f_mkfs("1:", 0, 0, FF_MAX_SS);    /* 格式化SPI FLASH,1:,盘符;0,自动选择文件系统类型 */

        if (res == 0)
        {
            f_setlabel((const TCHAR *)"1:ALIENTEK");        /* 设置Flash磁盘的名字为：ALIENTEK */
            lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color); /* 标志格式化成功 */
            res = exfuns_get_free("1:", &dtsize, &dfsize);  /* 重新获取容量 */
        }
    }

    if (res == 0)   /* 得到FLASH卡剩余容量和总容量 */
    {
        gui_phy.memdevflag |= 1 << 1;   /* 设置SPI FLASH在位 */
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SPI Flash Disk:     KB", g_point_color); /* FATFS检测 */
        temp = dtsize;
    }
    else system_error_show(5, ypos + fsize * (j + 1), "Flash Fat Error!", fsize);   /* flash 文件系统错误 */

    lcd_show_xnum(5 + 15 * (fsize / 2), ypos + fsize * j, temp, 5, fsize, 0, g_point_color);    /* 显示FLASH容量大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);	/* FLASH卡状态 */

    
    /* NAND FLASH检测 */
    temp = 0;
    do
    {
        temp ++;
        res = exfuns_get_free("2:",&dtsize,&dfsize);/* 得到FLASH剩余容量和总容量 */
        delay_ms(200);
    }
    while(res && temp < 20);                        /* 连续检测20次 */
    
    if(res == 0X0D)                                 /* 文件系统不存在 */
    {
        lcd_show_string(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "NAND Flash Disk Formatting...", g_point_color);/* 格式化FLASH */
        res = f_mkfs("2:", 0, 0, FF_MAX_SS);                                    /* 格式化FLASH,2,盘符;1,不需要引导区,8个扇区为1个簇 */
        
        if (res == 0)
        {
            f_setlabel((const TCHAR *)"2:NANDDISK");                    /* 设置Flash磁盘的名字为：NANDDISK */
            lcd_show_string(5 + okoffset,ypos+fsize * j ++,lcddev.width,lcddev.height,fsize, "OK", g_point_color);/* 标志格式化成功 */
            res = exfuns_get_free("2:",&dtsize,&dfsize);                /* 重新获取容量 */
        }
    }
    
    if(res == 0)                                                                                                    /* 得到NAND FLASH卡剩余容量和总容量 */
    {
        gui_phy.memdevflag |= 1 << 2;                                                                               /* 设置NAND FLASH在位. */
        lcd_show_string(5,ypos + fsize * j,lcddev.width,lcddev.height,fsize, "NAND Flash Disk:    MB", g_point_color);  /* FATFS检测 */
        temp = dtsize >> 10;                                                                                        /* 单位转换为MB */
    }else system_error_show(5,ypos + fsize * (j + 1),"NAND Flash Fat Error!",fsize);                                      /* flash 文件系统错误  */
    lcd_show_xnum(5 + 15 * (fsize / 2), ypos + fsize * j, temp, 5, fsize, 0, g_point_color);    /* 显示FLASH容量大小 */
    lcd_show_string(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,"OK", g_point_color);                /* FLASH卡状态 */
    
    /* U盘检测 */
    usbapp_mode_set(0); /* 设置为U盘模式 */
    temp = 0;

    while (usbx.hdevclass == 0 && temp < 1600)   /* 等待U盘被检测,最多等待8秒 */
    {
        usbapp_pulling();

        if ((usbx.bDeviceState & (1 << 6)) == 0 && temp > 300)break; /* 1.5秒钟之内,没有检测到设备插入,则直接跳过,不再等待 */

        delay_ms(5);
        temp++;
    }

    if (usbx.hdevclass == 1)        /* 检测到了U盘 */
    {
        fs[3]->pdrv = 3;
        f_mount(fs[3], "3:", 1);        /* 挂载挂载U盘 */
        res = exfuns_get_free((uint8_t *)"3:", &dtsize, &dfsize);   /* 得到U盘剩余容量和总容量 */
    }
    else res = 0XFF;

    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "U Disk:     MB", g_point_color);  /* U盘容量大小 */

    if (res == 0)   /* 得到容量正常 */
    {
        gui_phy.memdevflag |= 1 << 3;   /* 设置U盘在位 */
        temp = dtsize >> 10; /* 单位转换为MB */
        stastr = "OK";
    }
    else
    {
        temp = 0; /* 出错了,单位为0 */
        stastr = "ERROR";
    }

    lcd_show_xnum(5 + 7 * (fsize / 2), ypos + fsize * j, temp, 5, fsize, 0, g_point_color); /* 显示U盘容量大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, (char *)stastr, g_point_color);  /* U盘状态 */
    
    
    /* TPAD检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "TPAD Check...", g_point_color);

    if (tpad_init(2)||g_tpad_default_val > 1000)
    {
        g_tpad_failed_flag = 1; /* tpad失效了，用wk_up按键替代 */
        system_error_show(5, ypos + fsize * (j + 1), "TPAD Error!", fsize); /* 触摸按键检测 */
    }
    else lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* PCF8574检测 */
    lcd_show_string(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "PCF8574 Check...", g_point_color);
    if(pcf8574_init())system_error_show_pass(5+okoffset,ypos+fsize*j++,fsize,"ERROR");  /* PCF8574检测 */
    else lcd_show_string(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK", g_point_color); 
    
    /* AP3216C检测 */
    lcd_show_string(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "AP3216C Check...", g_point_color);
    if(ap3216c_init())system_error_show_pass(5 + okoffset,ypos+fsize * j++,fsize,"ERROR");  /* AP3216C检测 */
    else lcd_show_string(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK", g_point_color); 
    
    /* 24C02检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "24C02 Check...", g_point_color);

    if (at24cxx_check())system_error_show(5, ypos + fsize * (j + 1), "24C02 Error!", fsize);    /* 24C02检测 */
    else lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* ES8388检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "ES8388 Check...", g_point_color);

    if (es8388_init())system_error_show(5, ypos + fsize * (j + 1), "ES8388 Error!", fsize);
    else lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* SH3001检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SH3001 Check...", g_point_color);

    if (sh3001_init())system_error_show_pass(5+okoffset, ypos + fsize * j++, fsize, "Error");
    else lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* 字库检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "Font Check...", g_point_color);
    res = key_scan(1); /* 检测按键 */

    if (res == KEY1_PRES)   /* KEY1按下，更新？确认 */
    {
        res = system_font_update_confirm(5, ypos + fsize * (j + 1), fsize);
    }
    else res = 0;

    if (fonts_init() || (res == 1))   /* 检测字体,如果字体不存在/强制更新,则更新字库 */
    {
        res = 0; /* 按键无效 */

        if (fonts_update_font(5, ypos + fsize * j, fsize, "0:", g_point_color) != 0)        /* 从SD卡更新 */
        {
            tim8_int_init(100 - 1, 20000 - 1); /* 启动TIM3 轮询USB,10ms中断一次 */  
            
            if (fonts_update_font(5, ypos + fsize * j, fsize, "3:", g_point_color) != 0)    /* 从U盘更新 */
            {
                system_error_show(5, ypos + fsize * (j + 1), "Font Error!", fsize);         /* 字体错误 */
            }
            
            TIM8->CR1 &= ~(1 << 0);         /* 关闭定时器3 */
        }

        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK);     /* 填充底色 */
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "Font Check...", g_point_color);
    }

    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color); /* 字库检测OK */

    /* 系统文件检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SYSTEM Files Check...", g_point_color);

    while (app_system_file_check("1"))      /* 系统文件检测 */
    {
        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK); /* 填充底色 */
        lcd_show_string(5, ypos + fsize * j, (fsize / 2) * 8, fsize, fsize, "Updating", g_point_color); /* 显示updating */
        app_boot_cpdmsg_set(5, ypos + fsize * j, fsize);    /* 设置到坐标 */
        temp = 0;

        if (app_system_file_check("0"))     /* 检查SD卡系统文件完整性 */
        {
            if (app_system_file_check("3"))
            {
                res = 9; /* 标记为不可用的盘 */
            }
            else
            {
                res = 3;                   /* 标记为U盘 */
            }
        }
        else
        {
            res = 0;                       /* 标记为SD卡 */
        }

        if (res == 0 || res == 3)           /* 完整了才更新 */
        {
            sprintf((char *)verbuf, "%d:", res);

            if (app_system_update(app_boot_cpdmsg, verbuf))   /* 更新? */
            {
                system_error_show(5, ypos + fsize * (j + 1), "SYSTEM File Error!", fsize);
            }
        }
        
        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK); /* 填充底色 */
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SYSTEM Files Check...", g_point_color);

        if (app_system_file_check("1"))     /* 更新了一次，再检测，如果还有不全，说明SD卡文件就不全！ */
        {
            system_error_show(5, ypos + fsize * (j + 1), "SYSTEM File Lost!", fsize);
        }
        else break;
    }

    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* 触摸屏检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "Touch Check...", g_point_color);
    res = key_scan(1); /* 检测按键 */

    if (tp_init() || (res == KEY0_PRES && (tp_dev.touchtype & 0X80) == 0))   /* 有更新/按下了KEY0且不是电容屏,执行校准 */
    {
        if (res == 1)tp_adjust();

        res = 0;        /* 按键无效 */
        goto REINIT;    /* 重新开始初始化 */
    }

    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color); /* 触摸屏检测OK */

    /* 系统参数加载 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SYSTEM Parameter Load...", g_point_color);

    if (app_system_parameter_init())system_error_show(5, ypos + fsize * (j + 1), "Parameter Load Error!", fsize); /* 参数加载 */
    else lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SYSTEM Starting...", g_point_color);

    /* 蜂鸣器短叫,提示正常启动 */
    pcf8574_write_bit(BEEP_IO,0);
    delay_ms(100);
    pcf8574_write_bit(BEEP_IO,1);
    myfree(SRAMIN, version);
    delay_ms(1500);
}

int main(void)
{
    system_init();  /* 系统初始化 */

    OSInit();
    OSTaskCreateExt((void(*)(void *) )start_task,               /* 任务函数 */
                    (void *          )0,                        /* 传递给任务函数的参数 */
                    (OS_STK *        )&START_TASK_STK[START_STK_SIZE - 1], /* 任务堆栈栈顶 */
                    (INT8U          )START_TASK_PRIO,           /* 任务优先级 */
                    (INT16U         )START_TASK_PRIO,           /* 任务ID，这里设置为和优先级一样 */
                    (OS_STK *        )&START_TASK_STK[0],       /* 任务堆栈栈底 */
                    (INT32U         )START_STK_SIZE,            /* 任务堆栈大小 */
                    (void *          )0,                        /* 用户补充的存储区 */
                    (INT16U         )OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP); /* 任务选项,为了保险起见，所有任务都保存浮点寄存器的值 */
    OSStart();
}

extern OS_EVENT *audiombox; /* 音频播放任务邮箱 */

/**
 * @brief       开始任务
 * @param       pdata : 传入参数(未用到)
 * @retval      无
 */
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr = 0;
    uint32_t cnts;
    pdata = pdata;
    
    OSStatInit();                           /* 初始化统计任务.这里会延时1秒钟左右 */
    
    /* 根据配置的节拍频率配置SysTick */
    cnts = (CPU_INT32U)((400 * 1000000) / OS_TICKS_PER_SEC);
    OS_CPU_SysTickInit(cnts);
    
    app_srand(OSTime);
    audiombox = OSMboxCreate((void *) 0);   /* 创建邮箱 */
    
    OS_ENTER_CRITICAL();                    /* 进入临界区(无法被中断打断) */
    OSTaskCreateExt((void(*)(void *) )main_task,                /* 任务函数 */
                    (void *          )0,                        /* 传递给任务函数的参数 */
                    (OS_STK *        )&MAIN_TASK_STK[MAIN_STK_SIZE - 1], /* 任务堆栈栈顶 */
                    (INT8U          )MAIN_TASK_PRIO,            /* 任务优先级 */
                    (INT16U         )MAIN_TASK_PRIO,            /* 任务ID，这里设置为和优先级一样 */
                    (OS_STK *        )&MAIN_TASK_STK[0],        /* 任务堆栈栈底 */
                    (INT32U         )MAIN_STK_SIZE,             /* 任务堆栈大小 */
                    (void *          )0,                        /* 用户补充的存储区 */
                    (INT16U         )OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP); /* 任务选项,为了保险起见，所有任务都保存浮点寄存器的值 */

    OSTaskCreateExt((void(*)(void *) )usart_task,              	/* 任务函数 */
                    (void *          )0,                        /* 传递给任务函数的参数 */
                    (OS_STK *        )&USART_TASK_STK[USART_STK_SIZE - 1], /* 任务堆栈栈顶 */
                    (INT8U          )USART_TASK_PRIO,           /* 任务优先级 */
                    (INT16U         )USART_TASK_PRIO,           /* 任务ID，这里设置为和优先级一样 */
                    (OS_STK *        )&USART_TASK_STK[0],       /* 任务堆栈栈底 */
                    (INT32U         )USART_STK_SIZE,            /* 任务堆栈大小 */
                    (void *          )0,                        /* 用户补充的存储区 */
                    (INT16U         )OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP); /* 任务选项,为了保险起见，所有任务都保存浮点寄存器的值 */

    OSTaskCreateExt((void(*)(void *) )watch_task,               /* 任务函数 */
                    (void *          )0,                        /* 传递给任务函数的参数 */
                    (OS_STK *        )&WATCH_TASK_STK[WATCH_STK_SIZE - 1], /* 任务堆栈栈顶 */
                    (INT8U          )WATCH_TASK_PRIO,           /* 任务优先级 */
                    (INT16U         )WATCH_TASK_PRIO,           /* 任务ID，这里设置为和优先级一样 */
                    (OS_STK *        )&WATCH_TASK_STK[0],       /* 任务堆栈栈底 */
                    (INT32U         )WATCH_STK_SIZE,            /* 任务堆栈大小 */
                    (void *          )0,                        /* 用户补充的存储区 */
                    (INT16U         )OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP); /* 任务选项,为了保险起见，所有任务都保存浮点寄存器的值 */

    OSTaskSuspend(START_TASK_PRIO);                             /* 挂起起始任务 */
    OS_EXIT_CRITICAL();                                         /* 退出临界区(可以被中断打断) */
}

/**
 * @brief       主任务
 * @param       pdata : 传入参数(未用到)
 * @retval      无
 */
void main_task(void *pdata)
{
    uint8_t selx;
    uint16_t tcnt = 0;

    spb_init(0);        /* 初始化SPB界面 */
    spb_load_mui();     /* 加载SPB主界面 */

    slcd_frame_show(spbdev.spbahwidth); /* 显示界面 */

    while (1)
    {

        selx = spb_move_chk();
        system_task_return = 0;                 /* 清退出标志 */

        switch (selx)                           /* 发生了双击事件 */
        {
            case 0:
                ebook_play();
                break;                          /* 电子图书 */
            case 1:
                picviewer_play();
                break;                          /* 数码相框 */
            case 2:
                audio_play();
                break;                          /* 音乐播放 */
            case 3:
                video_play();
                break;                          /* 视频播放 */
            case 4:
                calendar_play();
                break;                          /* 时钟 */
            case 5:
                sysset_play();
                break;                          /* 系统设置 */
            case 6:
                nes_play();
                break;                          /* 游戏 */
            case 7:
                notepad_play();
                break;                          /* 记事本 */
            case 8:
                exe_play();
                break;                          /* 运行器 */
            case 9:
                paint_play();
                break;                          /* 手写画笔 */
            case 10:
                camera_play();
                break;                          /* 摄像头 */
            case 11:
                recorder_play();
                break;                          /* 录音 */
            case 12:
                usb_play();
                break;                          /* USB连接 */
            case 13:
                net_play();
                break;                          /* 网络测试 */
            case 14:
                wireless_play();
                break;                          /* 无线传书 */
            case 15:
                calc_play();
                break;                          /* 计算器 */
            case 16:
                qr_play();
                break;                          /* 二维码 */
            case 17:
                webcam_play();
                break;                          /* 网络摄像头 */
            case 18:
                frec_play();
                break;                          /* 人脸识别 */
            case 19:
                gyro_play();
                break;                          /* 6轴传感器 */
            case 20:
                grad_play();
                break;                          /* 水平仪 */
            case 21:
                beep_play();
                break;                          /* 蜂鸣器测试 */
            case 22:
                key_play();
                break;                          /* 按键测试 */
            case 23:
                led_play();
                break;                          /* led测试 */

            case SPB_ICOS_NUM:
                phone_play();
                break;                          /* 电话功能 */

            case SPB_ICOS_NUM + 1:
                app_play();
                break;                          /* APP */
            
            case SPB_ICOS_NUM + 2:
                vmeter_play();
                break;                          /* 电压表 */
        }

        if (selx != 0XFF)spb_load_mui();        /* 显示主界面 */

        delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */
        tcnt++;

        if (tcnt == 500)           /* OS_TICKS_PER_SEC个节拍为1秒钟 */
        {
            tcnt = 0;
            spb_stabar_msg_show(0);             /* 更新状态栏信息 */
        }
    }
}

volatile uint8_t memshow_flag = 1;              /* 默认打印mem使用率 */

/**
 * @brief       串口 任务,执行最不需要时效性的代码
 * @param       pdata : 传入参数(未用到)
 * @retval      无
 */
void usart_task(void *pdata)
{
    uint16_t alarmtimse = 0;
    float psin, psex, pstcm;
    pdata = pdata;

    while (1)
    {
        delay_ms(1000);

        if (alarm.ringsta & 1 << 7)   /* 执行闹钟扫描函数 */
        {
            calendar_alarm_ring(alarm.ringsta & 0x3); /* 闹铃 */
            alarmtimse++;

            if (alarmtimse > 300)   /* 超过300次了,5分钟以上 */
            {
                alarm.ringsta &= ~(1 << 7); /* 关闭闹铃 */
            }
        }
        else if (alarmtimse)
        {
            alarmtimse = 0;
            pcf8574_write_bit(BEEP_IO,1);
        }

        if (gsmdev.mode == 3)           /* 蜂鸣器,来电提醒 */
        {
            phone_ring(); 
        }
        
        if (systemset.lcdbklight == 0)  /* 自动背光控制 */
        {
            app_lcd_auto_bklight(); 
        }
        
        if (memshow_flag == 1)
        {
            psin = my_mem_perused(SRAMIN);
            psex = my_mem_perused(SRAMEX);
            pstcm = my_mem_perused(SRAMDTCM);
            printf("in:%3.1f,ex:%3.1f,ex:%3.1f\r\n", psin / 10, psex / 10, pstcm / 10);  /* 打印内存占用率 */
        }
    }
}

volatile uint8_t system_task_return;    /* 任务强制返回标志. */
volatile uint8_t ledplay_ds0_sta = 0;   /* ledplay任务,DS0的控制状态 */
uint8_t system_lwip_sta;

/**
 * @brief       监视 任务
 * @param       pdata : 传入参数(未用到)
 * @retval      无
 */
void watch_task(void *pdata)
{
    OS_CPU_SR cpu_sr = 0;
    uint8_t t = 0;
    uint8_t rerreturn = 0;
    uint8_t res;
    uint8_t key;
    pdata = pdata;

    while (1)
    {
        if (alarm.ringsta & 1 << 7)   /* 闹钟在执行 */
        {
            calendar_alarm_msg((lcddev.width - 200) / 2, (lcddev.height - 160) / 2); /* 闹钟处理 */
        }

        if (g_gif_decoding)         /* gif正在解码中 */
        {
            key = pic_tp_scan();

            if (key == 1 || key == 3)g_gif_decoding = 0; /* 停止GIF解码 */
        }

        if (ledplay_ds0_sta == 0)   /* 仅当ledplay_ds0_sta等于0的时候,正常熄灭LED0 */
        {
            if (t == 4)LED0(1);     /* 亮100ms左右 */

            if (t == 119)
            {
                LED0(0);            /* 2.5秒钟左右亮一次 */
                t = 0;
            }
        }

        t++;
        
        if (rerreturn)              /* 再次开始TPAD扫描时间减一 */
        {
            rerreturn--;
            delay_ms(15);           /* 补充延时差 */
        }
        else if (g_tpad_failed_flag)    /* TPAD失效，用WK_UP按键替代TPAD的功能 */
        {
            key = key_scan(0);          /* 按键扫描 */
            
            if (key == WKUP_PRES)       /* WKUP按键按下了 */
            {
                system_task_return = 1;
                
                if (g_gif_decoding)g_gif_decoding = 0;  //不再播放gif
            }
        }
        else if (tpad_scan(0))      /* TPAD按下了一次,此函数执行,至少需要15ms */
        {
            rerreturn = 10;         /* 下次必须100ms以后才能再次进入 */
            system_task_return = 1;

            if (g_gif_decoding)g_gif_decoding = 0;  /* 不再播放gif */
        }
        

        if ((t % 60) == 0)   /* 900ms左右检测1次 */
        {
            /* SD卡在位检测 */
            if ((DCMI->CR & 0X01) == 0)   /* 摄像头不工作的时候 */
            {
                OS_ENTER_CRITICAL();    /* 进入临界区(无法被中断打断) */
                res = sdmmc_get_status();    /* 查询SD卡状态 */
                OS_EXIT_CRITICAL();     /* 退出临界区(可以被中断打断) */

                if (res == 0XFF)
                {
                    gui_phy.memdevflag &= ~(1 << 0); /* 标记SD卡不在位 */
                    OS_ENTER_CRITICAL();/* 进入临界区(无法被中断打断) */
                    sd_init();          /* 重新检测SD卡 */
                    OS_EXIT_CRITICAL(); /* 退出临界区(可以被中断打断) */
                }
                else if ((gui_phy.memdevflag & (1 << 0)) == 0)     /* SD不在位? */
                {
                    f_mount(fs[0], "0:", 1);        /* 重新挂载sd卡 */
                    gui_phy.memdevflag |= 1 << 0;   /* 标记SD卡在位了 */
                }
            }

            /* U盘在位检测 */
            if (usbx.hdevclass == 1)
            {
                if ((gui_phy.memdevflag & (1 << 3)) == 0)   /* U盘不在位 */
                {
                    fs[3]->pdrv = 3;
                    f_mount(fs[3], "3:", 1);        /* 重新挂载U盘 */
                    gui_phy.memdevflag |= 1 << 3;   /* 标记U盘在位 */
                }
            }
            else
            {
                gui_phy.memdevflag &= ~(1 << 3);   /* U盘被拔出了 */
            }

            /* gsm检测 */
            if (system_lwip_sta == 0) gsm_status_check();   /* 有lwip任务的时候,不要发送gsm检测! */
        }

        gsm_cmsgin_check();             /* 来电/短信 监测 */

        if (usbx.mode == USBH_MSC_MODE) /* U盘模式,才处理 */
        {
            while ((usbx.bDeviceState & 0XC0) == 0X40)   /* USB设备插入了,但是还没连接成功,猛查询 */
            {
                usbapp_pulling();       /* 轮询处理USB事务 */
                delay_ms(1);            /* 不能像HID那么猛...,U盘比较慢 */
            }

            usbapp_pulling();           /* 处理USB事务 */
        }
        
        delay_ms(10);
    }
}

/**
 * @brief       硬件错误处理
 * @param       无
 * @retval      无
 */
void HardFault_Handler(void)
{
    uint8_t led1sta = 1;
    uint32_t i;
    uint8_t t = 0;
    uint32_t temp;
    temp = SCB->CFSR;               /* fault状态寄存器(@0XE000ED28)包括:MMSR,BFSR,UFSR */
    printf("CFSR:%8X\r\n", temp);   /* 显示错误值 */
    temp = SCB->HFSR;               /* 硬件fault状态寄存器 */
    printf("HFSR:%8X\r\n", temp);   /* 显示错误值 */
    temp = SCB->DFSR;               /* 调试fault状态寄存器 */
    printf("DFSR:%8X\r\n", temp);   /* 显示错误值 */
    temp = SCB->AFSR;               /* 辅助fault状态寄存器 */
    printf("AFSR:%8X\r\n", temp);   /* 显示错误值 */

    while (t < 5)
    {
        t++;
        LED1(led1sta ^= 1);

        for (i = 0; i < 0X1FFFFF; i++);
    }
}
