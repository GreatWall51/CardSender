/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 calendar_drv.h 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/10/14
* Notes: 
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/10/14, LiuWeiQiang create this file
* 
******************************************************************************/
#ifndef _TIMER_RTC_H_     
#define _TIMER_RTC_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
/*---------------------- Constant / Macro Definitions -----------------------*/
#define TIME_ZONE 8
#ifndef time_t
#define time_t uint32_t
#endif


struct tm
{
    int tm_sec;   /* 秒 – 取值区间为[0,59] */
    int tm_min;   /* 分 - 取值区间为[0,59] */
    int tm_hour; /* 时 - 取值区间为[0,23] */
    int tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
    int tm_mon;   /* 月份 - 取值区间为[1,12] */
    int tm_year; /* 年份，其值等于实际年份减去1900 */
    int tm_wday; /* 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 */
    int tm_yday; /* 从每年的1月1日开始的天数 – 取值区间为[0,365]，其中0代表1月1日，1代表1月2日*/
    int tm_isdst;/* 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的时候，tm_isdst为0；不了解情况时，tm_isdst为负。*/
};
typedef struct tm timer_rtc_t;
/*----------------------------- Global Typedefs -----------------------------*/
 
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 void second_irq_call_back(void);
 
 void set_timestamp(time_t time_cnt);
 time_t get_timestamp(void);
 uint8_t set_time_by_tm(timer_rtc_t *date_time);
 timer_rtc_t *get_time_by_tm(void);
 

#endif //_TIMER_RTC_H_
