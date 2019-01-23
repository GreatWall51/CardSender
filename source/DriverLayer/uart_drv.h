/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 uart_drv.h 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/05/16
* Notes: 
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/05/16, LiuWeiQiang create this file
* 
******************************************************************************/
#ifndef _UART_DRV_H_     
#define _UART_DRV_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
 
 
/*---------------------- Constant / Macro Definitions -----------------------*/
typedef enum
{
    USART_1 = 0,
	USART_2,
    USART_NUM,
}USART_TYPE;
 
/*----------------------------- Global Typedefs -----------------------------*/
 
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 
void uart_drv_init(USART_TYPE com_num);

#endif //_UART_DRV_H_
