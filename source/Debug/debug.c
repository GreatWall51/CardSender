/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 debug.c
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


/*------------------------------- Includes ----------------------------------*/
#include <stm32f0xx.h>
#include "timer_drv.h"
#include "hal_timer.h"
#include "define.h"
#include "debug.h"
#include "hal_debug.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
uint8_t g_fm23_init_flg;

int g_dbg_last_line = 0;
int g_dbg_pre_line = 0;
char* g_dbg_last_file = NULL;
char* g_dbg_pre_file = NULL;
uint32_t g_dbg_chk_timeout = 0;
size_t g_dbg_chk_val = 0;

/*----------------------- Function Prototype --------------------------------*/
extern void putCh ( uint8_t ch );

//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
	/* Whatever you require here. If the only file you are using is */
	/* standard output using printf() for debugging, no file handling */
	/* is required. */
};
/* FILE is typedef’ d in stdio.h. */
FILE __stdout;
int _sys_exit ( int x )
{
	x = x;
	return 0;
}
void _ttywrch ( int ch )
{
	ch = ch;
}

//重定义fputc函数
int fputc ( int ch, FILE* f )
{
	putCh ( ch );
	return ch;
}
/*----------------------- Function Implement --------------------------------*/
static void led_io_init ( void )
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHBPeriphClockCmd ( RCC_AHBPeriph_GPIOA, ENABLE );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init ( GPIOA, &GPIO_InitStructure );
}
static void led_hartbe ( void )
{
	static uint8_t flag = 0;
	if ( !IS_TIMER_EVT ( TIME_EVT_256 ) )
	{
		return ;
	}
	if ( flag == 0 )
	{
		flag = 1;
		cbi ( GPIOA,GPIO_Pin_0 );
	}
	else
	{
		flag = 0;
		sbi ( GPIOA,GPIO_Pin_0 );
	}
}
void debug_init()
{
	debug_uart_init();
	debug ( "..........DEBUG_ENABLE..........\r\n" );
	led_io_init();
}
void debug_real_time ( void )
{
	hal_debug_real_time();
	led_hartbe();
}
/*---------------------------------------------------------------------------*/
