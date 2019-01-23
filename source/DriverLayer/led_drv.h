/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 led_drv.h
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
#ifndef _LED_DRV_H_
#define _LED_DRV_H_


/*------------------------------- Includes ----------------------------------*/


/*----------------------------- Global Defines ------------------------------*/
typedef enum
{
	LED_NONE = 0,
	LED_RED_ON,
	LED_GREEN_ON,
	LED_RED_FLASH,
	LED_GREEN_FLASH,
	LED_OFF,
	
	LED_MODE_MAX,
}LED_MODE_E;

/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
void led_drv_init(void);
void led_set_mode(LED_MODE_E mode);


#endif //_LED_DRV_H_
