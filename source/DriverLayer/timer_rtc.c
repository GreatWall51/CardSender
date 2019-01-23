/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 timer_rtc.c 
* Desc:     获取/设置时间及日期
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/10/14
* Notes:    使用该模块需要做以下操作:
            1.根据需求修改时区 TIME_ZONE
            2.将second_irq_call_back放进1秒中断处理函数中执行
            3.时间的设置:(1)根据时间戳设置 set_timestamp
                         (2)根据时间结构体设置 set_time_by_tm
            4.时间的获取:(1)获取时间戳 get_timestamp
                         (2)获取时间结构体 get_time_by_tm
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/10/14, LiuWeiQiang create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdio.h>
#include "timer_rtc.h"

/*---------------------- Constant / Macro Definitions -----------------------*/
#define DAYS       (24*3600)
#define FOURYEARS (365*3+366)
#define TIMESTAMP_OFFSET (TIME_ZONE*3600)
static uint32_t sys_time_cnt = TIMESTAMP_OFFSET;
static struct tm sys_time;
 
/*----------------------- Type Declarations ---------------------------------*/
 
 
/*------------------- Global Definitions and Declarations -------------------*/
 
 
/*----------------------- Variable Declarations -----------------------------*/
 
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name: 	 second_irq_call_back 
*
* Desc: 	 秒中断处理函数
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
void second_irq_call_back(void)
{
    sys_time_cnt++;
}
/******************************************************************************
* Name: 	 get_timestamp 
*
* Desc: 	 获取当前时间的时间戳
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
time_t get_timestamp(void)
{
    return sys_time_cnt-TIMESTAMP_OFFSET;
}
/******************************************************************************
* Name: 	 set_timestamp 
*
* Desc: 	 根据时间戳设置当前时间
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
void set_timestamp(time_t time_cnt)
{
	sys_time_cnt = time_cnt+TIMESTAMP_OFFSET;
}
/******************************************************************************
* Name: 	 my_mktime 
*
* Desc: 	 将tm结构体表示的时间转换成时间戳
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
time_t my_mktime(const struct tm *date_time)
{
	unsigned int mon = date_time->tm_mon, year = date_time->tm_year;

	/*将时间后移两个月1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + date_time->tm_mday) +
		  year*365 - 719499
	    )*24 + date_time->tm_hour /* now have hours */
	  )*60 + date_time->tm_min /* now have minutes */
	)*60 + date_time->tm_sec; /* finally seconds */
}
/******************************************************************************
* Name: 	 get_moth_day 
*
* Desc: 	 根据输入的月份返回该月份的天数
* InParam: 	 bLeapYear:1->闰年,0->平年
             moth:月份(0-11)
* OutParam: 	 
* Return: 	 该月份的天数
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t get_moth_day(uint8_t bLeapYear,uint8_t moth)
{
    uint8_t days = 0;
    switch(moth+1)
    {
        case 1: days = 31;
        break;
        case 2:
            if(1 == bLeapYear) days = 29;
            else days = 28;
        break;
        case 3:   days = 31;
        break;
        case 4:   days = 30;
        break;
        case 5:   days = 31;
        break;
        case 6:   days = 30;
        break;
        case 7:   days = 31;
        break;
        case 8:   days = 31;
        break;
        case 9:   days = 30;
        break;
        case 10:  days = 31;
        break;
        case 11: days = 30;
        break;
        case 12: days = 31;
        break;
        default:  days = 0;
        break;
    }
		return days;
}
/******************************************************************************
* Name: 	 getMothAndDay 
*
* Desc: 	 获取月份和日期
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
void getMothAndDay(uint8_t bLeapYear, int nDays, uint8_t *nMoth, uint8_t *nDay)
{
    uint8_t i = 0;
    int nTmp = 0;

    for(i=0;i<12;i++)
    {
        nTmp = nDays - get_moth_day(bLeapYear,i);
        if (nTmp <= 0)
        {
            *nMoth = i;
            if (nTmp == 0)
            {
                *nDay = get_moth_day(bLeapYear,i);
            }
            else
            {
                *nDay = nDays;
            }
            break;
        }
        nDays = nTmp;
    }
    return;
}
/******************************************************************************
* Name: 	 localtime_r_s 
*
* Desc: 	 将输入的秒数转换成本地时间
* InParam: 	 timep:当前时间戳
* OutParam: 	 
* Return: 	 当前时间
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
struct tm *my_localtime(time_t *timep)
{
    int nDays = (*timep)/DAYS + (((*timep)%DAYS)?1:0);
    int nYear4 = nDays/FOURYEARS;
    int nRemain = nDays%FOURYEARS;
    int nDecyear = 70 + nYear4*4; //1970-1900 = 70
    uint8_t nDecmoth = 0;
    uint8_t nDecday = 0;
    int nSecond = 0;
    uint8_t bLeapyear = 0;
    struct tm *p_tm = &sys_time;

    if(nRemain < 365)
    {
        ;
    }
    else if(nRemain < 365*2)
    {
        nDecyear += 1;
        nRemain -= 365;
    }
    else if(nRemain < 365*3)
    {
        nDecyear += 2;
        nRemain -= 365*2;
    }
    else
    {
        nDecyear += 3;
        nRemain -= 365*3;
        bLeapyear = 1;
    }
    getMothAndDay(bLeapyear, nRemain, &nDecmoth, &nDecday);
    p_tm->tm_year = nDecyear;
    p_tm->tm_mon = nDecmoth;
    p_tm->tm_mday = nDecday;
    p_tm->tm_wday = (nDays+3)%7;//星期从0到6，依次为星期天，星期1，星期2.。。，因为1970年为星期4，所以加3

    nSecond = (*timep)%DAYS;
    p_tm->tm_hour = nSecond/3600;
    p_tm->tm_min = (nSecond%3600)/60;
    p_tm->tm_sec = (nSecond%3600)%60;

    return p_tm;
}
/******************************************************************************
* Name: 	 my_time 
*
* Desc: 	 获取本机的秒数计时
* InParam: 	 
* OutParam: 	 timer:本机的秒数
* Return: 	 本机的秒数
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
time_t my_time(time_t *timer)
{
	if(timer != NULL) 
	{
		*timer = sys_time_cnt;
	}
    return sys_time_cnt;
}
/******************************************************************************
* Name: 	 timer_rtc_get_time 
*
* Desc: 	 获取本地时间
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
timer_rtc_t *get_time_by_tm(void)
{
	uint32_t rtc = my_time(NULL);
	timer_rtc_t *p_tm;
		
    p_tm = my_localtime(&rtc);
    p_tm->tm_year += 1900;
	p_tm->tm_mon  += 1;
    return p_tm;
}
/******************************************************************************
* Name: 	 set_time_by_tm 
*
* Desc: 	 通过日期设置系统时间
* InParam: 	 
* OutParam: 	 
* Return: 	 0->成功,1->失败
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/10/14, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t set_time_by_tm(timer_rtc_t *date_time)
{
    time_t time_cnt = 0;
    if(date_time == NULL) return 1;
    time_cnt = my_mktime(date_time);
    sys_time_cnt = time_cnt;
    return 0;
}
/*---------------------------------------------------------------------------*/

