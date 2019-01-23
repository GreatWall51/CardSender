/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 debug.h
* Desc:
*
*
* Author: 	 LiuWeiQiang
* Date: 	 2018/03/31
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/03/31, LiuWeiQiang create this file
*
******************************************************************************/
#ifndef _DEBUG_H_
#define _DEBUG_H_


/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>
#include "debug_opt.h"
#include "hal_debug.h"

/*----------------------------- Global Defines ------------------------------*/
#define CFG_ENABLE_DEBUG     1
#define CFG_ENABLE_PRINTF_DBG      1    //使能打印
#define CFG_ENABLE_TIMEOUT_DBG      0   //超时打印
#define TIMROUT_PRINT_TIME 2000         //超时打印的超时时间ms
#define CFG_ENABLE_CMD_DBG 1

extern int g_dbg_last_line;
extern char* g_dbg_last_file;

extern int g_dbg_pre_line;
extern char* g_dbg_pre_file;

extern uint32_t g_dbg_chk_timeout;
extern size_t g_dbg_chk_val;

#define sbi(ADDRESS,BIT) 	((ADDRESS->BSRR = BIT))	// Set bit

/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
#if CFG_ENABLE_DEBUG
#define debug(P...)  do{         \
                     printf(P);  \
                     }while(0)

#define debug_hex(data,len) do{ const int l=(len); int x;               \
                     for(x=0 ; x<l ; x++) debug("0x%02x ",*((data)+x)); \
                     debug("\r\n");}while(0)

#define debug_char(data,len) do{ const int l=(len); int x;         \
                     for(x=0 ; x<l ; x++) debug("%c",*((data)+x)); \
                     debug("\r\n");}while(0)

#define debug_dec(data,len) do{ const int l=(len); int x;          \
                     for(x=0 ; x<l ; x++) debug("%d ",*((data)+x));\
                     debug("\r\n");}while(0)
#else
#define debug(P...) do{}while(0)
#define debug_hex(data,len) do{}while(0)
#define debug_char(data,len) do{}while(0)
#define debug_dec(data,len) do{}while(0)
#endif

#if CFG_ENABLE_PRINTF_DBG
/* 定时监视变量值 */
#define DEBUG_CHK_LINE_VAL(VAL) do{g_dbg_chk_val = (size_t)(VAL);}while(0)

/* 在要监视处执行 */
#define DEBUG_CHK_LINE() do{           \
    g_dbg_pre_file= g_dbg_last_file;   \
    g_dbg_pre_line = g_dbg_last_line;  \
    g_dbg_last_file = __FILE__;        \
    g_dbg_last_line = __LINE__;        \
    g_dbg_chk_timeout = 0;             \
}while(0)

/* 在Timer中断内执行，TIME_OUT为超时打印计数*ms */
#define DEBUG_CHK_LINE_TIMER_INT(TIME_OUT) do{    \
    if(g_dbg_chk_timeout > TIME_OUT)              \
    {                                             \
        debug("![%x]! %s:%d -> %s:%d ->!\r\n", g_dbg_chk_val, g_dbg_pre_file, g_dbg_pre_line, g_dbg_last_file, g_dbg_last_line);\
        g_dbg_chk_timeout = 0;                    \
    }                                             \
    g_dbg_chk_timeout++;                          \
}while(0)

#else
#define DEBUG_CHK_LINE() do{}while(0)
#define DEBUG_CHK_LINE_VAL(VAL) do{}while(0)
#define DEBUG_CHK_LINE_TIMER_INT(TIME_OUT) do{}while(0)
#endif

void debug_init ( void );
void debug_real_time ( void );
/*---------------------------------------------------------------------------*/
#endif //_DEBUG_H_
