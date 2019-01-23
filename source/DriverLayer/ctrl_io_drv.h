/******************************************************************************
* Copyright 2010-2014 ABB Genway Co.,Ltd.
* FileName: 	 led.h 
* Desc:
* 
* 
* Author: 	 Kimson
* Date: 	 2014/03/12
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2014/03/12, Kimson create this file
* 
******************************************************************************/
#ifndef _CTRL_IO_DRV_H_     
#define _CTRL_IO_DRV_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
 #include "stm32f0xx.h" 
 #include "hal_ctrl_io.h" 
/*----------------------------- Global Defines ------------------------------*/


/*----------------------------- Global Typedefs -----------------------------*/
typedef enum {
  CTRL_MODE_ON = 0, //常亮
  CTRL_MODE_OFF, //常闭
  CTRL_MODE_FLASH, //快闪
  CTRL_MODE_FLASH_SLOW, //慢闪
  CTRL_MODE_BLINK, //闪一次，暂未支持 
  CTRL_MODE_TOGGLE, //翻转
  CTRL_MODE_NUM,
}CTRL_MODE;
 
#if (CTRL_NUM > 32)
#error too many LED, it must not bigger than 32.
#endif
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
void ctrl_io_init(void);
void ctrl_io_real_time(void);
void ctrl_io_set(CTRL_INDEX index, CTRL_MODE mode, uint32_t timeout);
uint8_t ctrl_io_get(uint8_t index);
#endif //_LED_H_

