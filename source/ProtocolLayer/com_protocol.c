/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 com_protocol.c 
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
 
 
/*------------------------------- Includes ----------------------------------*/
#include "com_config.h"
#include "com_protocol.h"
#include "uart_protocol_func.h"
#include <stdio.h>
 
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/
typedef struct protocol_port_config_str{
    com_mac_protocol_fun_t mac_funs; /* protocol funs for MAC layer */
}protocol_port_config_t;
 
/*------------------- Global Definitions and Declarations -------------------*/
static  const protocol_port_config_t c_protocol_fun_config[CFG_COM_PORT_CNT] = 
{
    {com_pro_uart_process_frame, com_pro_uart_mac_ack, com_pro_uart_respone_frame, com_pro_uart_get_mac_timeout_t, NULL}, /* the protocol funs of PORT_UART_CC2530 */
};
 
/*----------------------- Variable Declarations -----------------------------*/
 
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/

/******************************************************************************
* Name: 	 com_protocol_mac_init 
*
* Desc: 	 mac²ãº¯Êý³õÊ¼»¯
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/17, Create this function by LiuWeiQiang
 ******************************************************************************/
void com_protocol_mac_init(ENUM_COM_PORT_T port, com_mac_protocol_fun_t *p_fun)
{
    if(port >= CFG_COM_PORT_CNT) return;
    *p_fun = c_protocol_fun_config[port].mac_funs;
}

/*---------------------------------------------------------------------------*/
