/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 com_func.h 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/06/13
* Notes: 
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/13, LiuWeiQiang create this file
* 
******************************************************************************/
#ifndef _COM_FUNC_H_     
#define _COM_FUNC_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
 
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------------- Global Typedefs -----------------------------*/
 
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/

void usart1_init(void);
void usart1_start_send(void);
void usart1_send(uint8_t value);
void usart1_stop_send(void);

#endif //_COM_FUNC_H_
