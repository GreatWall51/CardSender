/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 led_drv.c
* Desc:
*
*
* Author: 	 LiuWeiQiang
* Date: 	 2018/11/14
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/11/14, LiuWeiQiang create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "led_drv.h"
#include "ctrl_io_drv.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
void led_drv_init(void)
{
	led_set_mode(LED_RED_ON);
}

void led_set_mode(LED_MODE_E mode)
{
	if(mode >= LED_MODE_MAX) return ;
	switch(mode)
	{		
		case LED_NONE:
		break;
		case LED_GREEN_ON:
			ctrl_io_set(CTRL_LED_RED,CTRL_MODE_OFF,0);
		    ctrl_io_set(CTRL_LED_GREEN,CTRL_MODE_ON,0);
		break;
		
		case LED_RED_ON:
			ctrl_io_set(CTRL_LED_RED,CTRL_MODE_ON,0);
		    ctrl_io_set(CTRL_LED_GREEN,CTRL_MODE_OFF,0);
		break;
		
		case LED_RED_FLASH:
			ctrl_io_set(CTRL_LED_RED,CTRL_MODE_FLASH,0);
		    ctrl_io_set(CTRL_LED_GREEN,CTRL_MODE_OFF,0);
		break;
		
		case LED_GREEN_FLASH:
			ctrl_io_set(CTRL_LED_RED,CTRL_MODE_OFF,0);
		    ctrl_io_set(CTRL_LED_GREEN,CTRL_MODE_FLASH,0);
		break;
		
		case LED_OFF:
			ctrl_io_set(CTRL_LED_RED,CTRL_MODE_OFF,0);
		    ctrl_io_set(CTRL_LED_GREEN,CTRL_MODE_OFF,0);
		break;
		
		default:
			ctrl_io_set(CTRL_LED_RED,CTRL_MODE_ON,0);
		    ctrl_io_set(CTRL_LED_GREEN,CTRL_MODE_OFF,0);
		break;
	}
}

/*---------------------------------------------------------------------------*/

