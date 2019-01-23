/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 com_func.c 
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
 
 
/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "hal_com_func.h"
#include "hal_com.h" 
#include "uart_drv.h" 
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/
 
 
/*------------------- Global Definitions and Declarations -------------------*/
 
 
/*----------------------- Variable Declarations -----------------------------*/
 
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
 
void usart1_init(void)
{
    uart_drv_init(USART_1);
}

void usart1_start_send(void)
{
    uint8_t value;
    hal_com_port_t *hal_port;

    hal_port = hal_com_get_port_info(PORT_CARD_SENDER);
    value = hal_port->s_frame[0];
    hal_port->s_sent_point++;
    USART_SendData(USART1, value);  
}

void usart1_send(uint8_t value)
{
    USART_SendData(USART1, value);   
}

void usart1_stop_send(void)
{

}

/*---------------------------------------------------------------------------*/
