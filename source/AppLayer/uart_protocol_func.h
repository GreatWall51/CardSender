/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 uart_protocol_func.h 
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
#ifndef _UART_PROTOCOL_H_     
#define _UART_PROTOCOL_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include "com_protocol.h"
 
/*---------------------- Constant / Macro Definitions -----------------------*/
#define COM_PRO_UART_MIN_INTERVAL      30
#define PRO_UART_HEAD             0xAA //帧头首字节
#define PRO_UART_HEAD_SIZE        6     //帧头长度
#define PRO_UART_OFFSET_LEN       1     //帧中的长度字节的偏移地址

#define READ_OBJECT_ATTRIBUTE    0X01
#define WRITE_OBJECT_ATTRIBUTE   0X02
#define CARD_OPERATION           0x05



#define SENDER_EVT_REPORT        0x11
#define INIT_CARD_RESULT_REPORT  0X05
#define CARD_INNER_AUTH          0x20
/*----------------------------- Global Typedefs -----------------------------*/
typedef struct
{
    uint8_t head;
    uint8_t len;
    uint8_t dest_addr;
    uint8_t src_addr;
    uint8_t frame_id;
    uint8_t cmd;
    uint8_t payload;
}uart_frame_head_t;


/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/


/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
uint8_t  get_check_sum(uint8_t *point, uint8_t len);
uint8_t check_sum(uint8_t *point, uint8_t len);
uint8_t com_pro_uart_process_frame(uint8_t in_data, uint8_t interval, mac_process_context_t* p_context);
uint32_t  com_pro_uart_get_mac_timeout_t(uint8_t send_seq, uint8_t type);
uint8_t  com_pro_uart_mac_ack(uint8_t *ack_packet, uint32_t interval, uint8_t *cmd_packet);
uint16_t  com_pro_uart_respone_frame(uint8_t *cmd_packet, uint8_t *ack_packet);

#endif //_UART_PROTOCOL_H_
