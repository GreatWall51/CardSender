/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 uart_mode.h 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/06/17
* Notes: 
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/17, LiuWeiQiang create this file
* 
******************************************************************************/
#ifndef _UART_MODE_H_     
#define _UART_MODE_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "com_config.h"
#include "mac_com.h"
/*---------------------- Constant / Macro Definitions -----------------------*/
#define UART_MAX_REG_FUNC    10 //回调函数可注册的最大个数
 
/*----------------------------- Global Typedefs -----------------------------*/
typedef void (*uart_analy_frame)(uint8_t *p_context);

typedef struct
{
  uint8_t cmd;
  uart_analy_frame call_back;
}uart_cmd_fun_t;
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/

uint8_t uart_callback_register(uint8_t cmd, uart_analy_frame callback);
void uart_callback_unregister(uint8_t cmd);
void uart_mode_init(void);
void uart_mode_real_time(void);
com_mac_send_packet_t *apply_send_packet(ENUM_COM_PORT_T port);
uint8_t *set_send_packet(com_mac_send_packet_t *packet, uint16_t len, uint8_t times, uint16_t need_ack);
uint8_t *set_frame_head(uint8_t *addr, uint8_t cmd, uint8_t len, uint8_t dest_addr);

#endif //_UART_MODE_H_
