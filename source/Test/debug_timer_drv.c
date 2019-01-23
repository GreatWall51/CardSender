/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 debug_timer_drv.c
* Desc:
*
*
* Author: 	 LiuWeiQiang
* Date: 	 2018/07/30
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/30, LiuWeiQiang create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "timer_set.h"
#include "timer_drv.h"
#include "timer_rtc.h"
/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/

timer_set_t timer_set_test;
/*----------------------- Function Implement --------------------------------*/
DEBUG_CMD_FN(timer_test)
{
    dbg_cmd_split_t *dbg_cmd_split;

    dbg_cmd_split = get_dbg_cmd_split();
    if(strncmp(dbg_cmd_split->arg[1], "timer_set", strlen("timer_set")) == 0)
	{
	    uint32_t interval = 0;
	    if(dbg_cmd_split->arg_cnt != 3)
	    {
    	    debug("param cnt error!\r\n");
    	    return;
	    }
	    interval = atoi(dbg_cmd_split->arg[2]);
	    debug("set timer except = %d,current time = %d\r\n",interval,get_sys_timer());
	    timer_set(&timer_set_test,interval);
	}
	else if(strncmp(dbg_cmd_split->arg[1], "timer_reset", strlen("timer_reset")) == 0)
	{
	    timer_reset(&timer_set_test);
	    debug("reset timer,current time = %d\r\n",get_sys_timer());
	}
	else if(strncmp(dbg_cmd_split->arg[1], "timer_restart", strlen("timer_restart")) == 0)
	{
	    timer_restart(&timer_set_test);
	    debug("restart timer,current time = %d\r\n",get_sys_timer());
	}
	else if(strncmp(dbg_cmd_split->arg[1], "timer_disable", strlen("timer_disable")) == 0)
	{
	    timer_disable(&timer_set_test);
	    debug("disable timer\r\n");
	}
	else if(strncmp(dbg_cmd_split->arg[1], "timer_expired", strlen("timer_expired")) == 0)
	{
	    //ÔÚreal_timeÖÐÖ´ÐÐ
//	    debug_timer_drv_real_time();
	}
	else if(strncmp(dbg_cmd_split->arg[1], "get_time_by_tm", strlen("get_time_by_tm")) == 0)
	{
			timer_rtc_t *timer_rtc;
		timer_rtc = get_time_by_tm();
		debug("current time:%d <==> %d/%d/%d %d:%d:%d\r\n",get_timestamp(),timer_rtc->tm_year,timer_rtc->tm_mon,timer_rtc->tm_mday,timer_rtc->tm_hour,timer_rtc->tm_min,timer_rtc->tm_sec);
	}
	else if(strncmp(dbg_cmd_split->arg[1], "set_time_by_tm", strlen("set_time_by_tm")) == 0)
	{
			timer_rtc_t timer_rtc;
			
			if(dbg_cmd_split->arg_cnt != 8)
	    {
    	    debug("param cnt error!\r\n");
    	    return;
	    }
			timer_rtc.tm_year = atoi(dbg_cmd_split->arg[2]);
			timer_rtc.tm_mon = atoi(dbg_cmd_split->arg[3]);
			timer_rtc.tm_mday = atoi(dbg_cmd_split->arg[4]);
			timer_rtc.tm_hour = atoi(dbg_cmd_split->arg[5]);
			timer_rtc.tm_min = atoi(dbg_cmd_split->arg[6]);
			timer_rtc.tm_sec = atoi(dbg_cmd_split->arg[7]);
			set_time_by_tm(&timer_rtc);
			debug("set time:%d/%d/%d %d:%d:%d\r\n",timer_rtc.tm_year,timer_rtc.tm_mon,timer_rtc.tm_mday,timer_rtc.tm_hour,timer_rtc.tm_min,timer_rtc.tm_sec);
	}
	else
	{
			debug("unsupport function\r\n");
	}
}

void debug_timer_drv_real_time(void)
{
    if(timer_expired(&timer_set_test))
    {
				
//	    timer_disable(&timer_set_test);
        debug("timer_expired occured!,current time = %d\r\n",get_sys_timer());
			timer_restart(&timer_set_test);
    }
}
void timer_rtc_init(void)
{
		timer_rtc_t timer_rtc;
	
	timer_rtc.tm_year = 2018;
	timer_rtc.tm_mon = 1;
	timer_rtc.tm_mday = 1;
	timer_rtc.tm_hour = 8;
	timer_rtc.tm_min = 0;
	timer_rtc.tm_sec = 0;
	set_time_by_tm(&timer_rtc);
}
void timer_rtc_real_time(void)
{
	static uint32_t cnt = 0;
	
	if(IS_TIMER_EVT(TIME_EVT_8))
	{
			cnt++;
			if(cnt%125 == 0)
			{
				second_irq_call_back();
			}
	}
}
/*---------------------------------------------------------------------------*/

