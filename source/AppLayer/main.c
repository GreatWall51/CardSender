/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 main.c
* Desc:
*
*
* Author: 	 liuwq
* Date: 	 2017/09/23
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2017/09/23, liuwq create this file
*
******************************************************************************/
/*------------------------------- Includes ----------------------------------*/
#include "opt.h"
#include <stm32f0xx.h>
#include "wdg_drv.h"
#include "timer_drv.h"
#include "reg_table.h"
#include "device_config.h"
#include "uart_mode.h"
#include "parameter_mg.h"
#include "ctrl_io_drv.h"
#include "led_drv.h"

#include "beep_drv.h"
#include "debug.h"
#if CARD_READ
#include "fm17x2_drv.h"
#include "card_read_drv.h"
#include "card_rw_drv.h"
#include "card_sender.h"
#endif

extern void debug_timer_drv_real_time(void);
extern void timer_rtc_init(void);
extern void timer_rtc_real_time(void);


void drv_init ( void )
{
	init_timer();
#if CFG_ENABLE_DEBUG
	debug_init();
#else
#endif
//	timer_rtc_init();


	beep_drv_init();
#if (!CFG_ENABLE_DEBUG)
	ctrl_io_init();
	led_drv_init();
	wdg_drv_init();
#endif
}

void app_init ( void )
{
	reg_table_init();
	device_config_init();
	uart_mode_init();
	parameter_mg_init();
#if (CARD_READ)
	//需在表初始化结束之后再初始化卡参数
	card_read_drv_init();
	card_rw_drv_init();
	card_sender_init();
#endif
}

void drv_run_real_time ( void )
{
	timer_real_time();
	beep_real_time();
#if CFG_ENABLE_DEBUG
#else
	ctrl_io_real_time();
#endif
}

void app_run_real_time ( void )
{
#if CFG_ENABLE_DEBUG
	debug_real_time();
#else
#endif
	reg_table_real_time();
	device_config_real_time();
	uart_mode_real_time();
//#if (!CFG_ENABLE_DEBUG)
	card_sender_real_time();
//#endif
}



int main ( void )
{
//	flash_read_protect();
	drv_init();
	app_init();
	while ( 1 )
	{
//		uint32_t start_time,end_time;		
//		start_time = get_sys_timer();
		
		drv_run_real_time();
		app_run_real_time();
//		delay_ms(5);
//		end_time = get_sys_timer();

//		debug("total time = %d\r\n",end_time-start_time);




#if (!CFG_ENABLE_DEBUG)
		wdg_feed_real_time();
#endif
	}
}
/*---------------------------------------------------------------------------*/
