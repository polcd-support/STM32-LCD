/**
 ****************************************************************************************************
 * @file        lyric.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2022-10-28
 * @brief       APP-歌词显示实现 代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.1 20221028
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "lyric.h"
#include "./FATFS/source/diskio.h"

/**
 * @brief       把后缀名改为sfx
 * @param       name            : 文件名(必须为x.x结构)
 * @param       sfx             : 后缀
 * @retval      无
 */
void lrc_chg_suffix(uint8_t *name, uint8_t *sfx)
{
    while (*name != '\0')name++;    /* 找到结尾 */

    while (*name != '.')name--;     /* 找到'.' */

    *(++name) = sfx[0];
    *(++name) = sfx[1];
    *(++name) = sfx[2];
    *(++name) = '\0';               /* 添加结束符 */
}

/**
 * @brief       创建歌词结构体
 * @param       无
 * @retval      无
 */
_lyric_obj *lrc_creat(void)
{
    _lyric_obj *lrc_crt;
    lrc_crt = (_lyric_obj *)gui_memin_malloc(sizeof(_lyric_obj));   /* 分配内存 */

    if (lrc_crt == NULL)return NULL;/* 内存分配不够 */

    gui_memset((uint8_t *)lrc_crt, 0, sizeof(_lyric_obj));  /* 清零 */
    lrc_crt->flrc = (FIL *)gui_memin_malloc(sizeof(FIL));   /* 分配内存 */

    if (lrc_crt->flrc == NULL)      /* 内存分配不够 */
    {
        gui_memin_free(lrc_crt);    /* 删除前面申请的 */
    }

    gui_memset((uint8_t *)lrc_crt->flrc, 0, sizeof(FIL));   /* 清零 */
    return lrc_crt;
}

/**
 * @brief       释放LRC所申请的内存
 * @param       无
 * @retval      无
 */
void lrc_delete(_lyric_obj *lcrdelete)
{
    gui_memin_free(lcrdelete->flrc);/* 删除文件结构体 */
    gui_memin_free(lcrdelete);
}

/**
 * @brief       将2个字符转为数字
 * @param       str             : 字符串(以':'/'.'/'\0'结束,且只包含数字/':'/'.')
 * @retval      转换后的结果
 */
uint8_t lrc_str2num(uint8_t *str)
{
    uint8_t chr;

    if (*str == ':' || *str == '.' || *str == '\0')return 0; /* 第一个数据就是:/.,错误的数据 */

    chr = *str;
    str++;

    if (*str == ':' || *str == '.' || *str == '\0')
    {
        return chr - '0';   /* 只有一个数据位 */
    }

    return (chr - '0') * 10 + (*str - '0'); /* 2位数据 */
}

/**
 * @brief       歌词TAG分析
 * @param       lrcx            : 歌词结构体
 * @param       lrcpos          : 当前string的起始地址
 * @param       str             : 当前string
 * @retval      无
 */
void lrc_analyze(_lyric_obj *lrcx, uint16_t lrcpos, uint8_t *str)
{
    uint8_t *p;
    uint8_t i = 0;
    uint16_t lrcoffset;
    uint8_t sta = 0;    /* 最高位表示是否是时间TAG */
                        /* bit1:0,处于TAG外;1,处于TAG内 */
                        /* bit0:0,时间TAG正确;1,时间TAG错误 */
    uint8_t tagbuf[9];  /* tag最长为9个字节 */
    uint32_t pstime = 0;/* 10毫秒 */

    lrcoffset = 0;
    p = str;

    while (1)
    {
        if (*p == '[')sta |= 1 << 0;
        else if (*p == ']')sta = 0;
        else if (sta == 0 && *p != ' ')break; /* 在tag外发现了非空格的字符 */

        lrcoffset++;    /* 统计歌词开始的地方 */
        *p++;
    }

    if (lrcoffset == 0)return;  /* 如果没有TAG存在,直接退出 */

    sta = 0;

    while (1)
    {
        if (*str == '[')
        {
            sta |= 1 << 7;      /* 默认标记这是一个时间头 */
            sta |= 1 << 1;      /* 标记在TAG里面 */
            i = 0;
        }
        else if (*str == ']')   /* 找到了一个TAG的结尾 */
        {
            if (sta & (1 << 7)) /* 是一个合法的时间TAG */
            {
                tagbuf[i] = '\0'; /* 在末尾添加结束符 */
                p = tagbuf;
                pstime = (uint32_t)lrc_str2num(p) * 6000; /* 一分钟是6000个10ms */

                while (*p != ':' && *p != '.' && *p != '\0')p++; /* 偏移到下一个位置 */

                if (*p != 0) /* 还没有结束 */
                {
                    p++;
                    pstime += (uint32_t)lrc_str2num(p) * 100; /* 一秒钟是100个10ms */

                    while (*p != ':' && *p != '.' && *p != '\0')p++;        /* 偏移到下一个位置 */

                    if (*p != '\0') /* 仍然没有结束 */
                    {
                        p++;
                        pstime += (uint32_t)lrc_str2num(p);                 /* 就是10ms为单位的数据 */
                    }

                    lrcx->time_tbl[lrcx->indexsize] = pstime;               /* 记录时间:单位,10ms */
                    lrcx->addr_tbl[lrcx->indexsize] = lrcpos + lrcoffset;   /* 记录地址 */
                    lrcx->indexsize++;                                      /* 总数增1 */
                }
            }

            sta = 0;    /* 标记结束一次查找 */
            i = 0;
        }
        else if (*str != ' ')       /* 非空格 */
        {
            if (sta & (1 << 1))     /* 在TAG内部 */
            {
                if (sta & (1 << 7)) /* 是一个时间TAG */
                {
                    if ((*str <= '9' && *str >= '0') || *str == ':' || *str == '.')
                    {
                        if (i > 8)sta &= ~(1 << 7); /* 错误的TAG,太长了 */
                        else
                        {
                            tagbuf[i] = *str;   /* 记录数据 */
                            i++;
                        }
                    }
                    else sta &= ~(1 << 7);      /* 不是时间TAG */
                }
            }
            else break;     /* 出现了TAG外的非空格,表示结束了 */
        }

        if (*str == '\0')break; /* 是结束符 */

        str++;
    }
}

/**
 * @brief       歌词排序
 *  note        按从小打到的时间顺序排序
 * @param       lrcx            : 歌词结构体
 * @retval      无
 */
void lrc_sequence(_lyric_obj *lrcx)
{
    uint16_t i, j;
    uint16_t temp;

    if (lrcx->indexsize == 0)return; /* 没有数据,直接不用排序 */

    for (i = 0; i < lrcx->indexsize - 1; i++) /* 排序 */
    {
        for (j = i + 1; j < lrcx->indexsize; j++)
        {
            if (lrcx->time_tbl[i] > lrcx->time_tbl[j]) /* 升序排列 */
            {
                temp = lrcx->time_tbl[i];
                lrcx->time_tbl[i] = lrcx->time_tbl[j];
                lrcx->time_tbl[j] = temp;

                temp = lrcx->addr_tbl[i];
                lrcx->addr_tbl[i] = lrcx->addr_tbl[j];
                lrcx->addr_tbl[j] = temp;
            }
        }
    }
}

/**
 * @brief       读取歌词
 * @param       lrcx            : 歌词结构体
 * @param       path            : 路径名字
 * @param       name            : MP3歌曲的名字
 * @retval      0, 有LRC,读取正常;
 *              其他, 错误;
 */
uint8_t lrc_read(_lyric_obj *lrcx, uint8_t *path, uint8_t *name)
{
    uint8_t *pname;
    uint16_t fpos;
    uint8_t *p;
    uint8_t res = 0;
    /* 清空之前的数据 */
    lrcx->indexsize = 0;
    lrcx->curindex = 0;
    lrcx->oldostime = 0;
    lrcx->curtime = 0;
    lrcx->detatime = 0;
    lrcx->updatetime = 0;
    lrcx->namelen = 0;
    lrcx->curnamepos = 0;

    pname = gui_memin_malloc(strlen((const char *)path) + strlen((const char *)name) + 2); /* 申请内存 */

    if (pname == NULL)res = 1; /* 申请失败 */

    if (res == 0)
    {
        pname = gui_path_name(pname, path, name);   /* 文件名加入路径 */

        if (pname)
        {
            lrc_chg_suffix(pname, "lrc");           /* 修改后缀为.lrc */
            res = f_open(lrcx->flrc, (const TCHAR *)pname, FA_READ); /* 打开文件 */

            if (res == 0)
            {
                while (1)
                {
                    fpos = f_tell(lrcx->flrc);      /* 得到片偏移前的位置 */
                    p = (uint8_t *)f_gets((TCHAR *)lrcx->buf, LYRIC_BUFFER_SIZE, lrcx->flrc); /* 读取歌词 */

                    if (p == 0)break;               /* 读数结束了 */
                    else
                    {
                        lrc_analyze(lrcx, fpos, p);
                    }
                }

                lrc_sequence(lrcx);
            }
        }
    }

    gui_memset((uint8_t *)lrcx->buf, 0, LYRIC_BUFFER_SIZE); /* 缓冲区数据清零 */
    gui_memin_free(pname);
    return res;
}

/**
 * @brief       显示一行歌词
 * @param       lrcx            : 歌词结构体
 * @param       x,y,width,height: 坐标,当height=0时,不显示歌词
 * @retval      无
 */
void lrc_show_linelrc(_lyric_obj *lrcx, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    uint8_t *p;
    f_lseek(lrcx->flrc, lrcx->addr_tbl[lrcx->curindex]);
    p = (uint8_t *)f_gets((TCHAR *)lrcx->buf, LYRIC_BUFFER_SIZE, lrcx->flrc);   /* 读取歌词 */

    if (p)
    {
        lrcx->namelen = strlen((const char *)p) * (lrcx->font / 2);             /* 本句歌词所占点数 */

        //gui_phy.back_color=lrcx->bkcolor;                         /* 设置背景色 */
        if (height)gui_show_strmid(x, y, width, height, lrcx->color, lrcx->font, p); /* 显示歌词 */

        lrcx->namelen = strlen((const char *)p) * lrcx->font / 2;   /* 得到本句歌词所占点的长度 */
        lrcx->curnamepos = 0;
        lrcx->detatime = 0;	/* 时间清零 */
    }
}

/**
 * @brief       歌词测试
 * @param       path            : 路径
 * @param       name            : 名字
 * @retval      测试结果
 */
uint8_t lrc_test(uint8_t *path, uint8_t *name)
{
    _lyric_obj *lrcx;
    uint8_t i;
    uint8_t *p;
    uint8_t res = 0;
    lrcx = lrc_creat(); /* 创建 */

    if (lrcx)
    {
        res = lrc_read(lrcx, path, name);

        if (res == 0)
        {
            printf("歌词条数:%d\r\n", lrcx->indexsize);

            for (i = 0; i < lrcx->indexsize; i++)
            {
                printf("%3d 时间:%5d ", i, lrcx->time_tbl[i]);
                f_lseek(lrcx->flrc, lrcx->addr_tbl[i]);
                p = (uint8_t *)f_gets((TCHAR *)lrcx->buf, LYRIC_BUFFER_SIZE, lrcx->flrc); /* 读取歌词 */
                printf("歌词:%s\r\n", p);
            }
        }
    }

    lrc_delete(lrcx);
    return res;
}



























