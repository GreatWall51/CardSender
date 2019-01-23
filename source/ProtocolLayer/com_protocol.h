/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 com_protocol.h 
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
#ifndef _COM_PROTOCOL_H_     
#define _COM_PROTOCOL_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include "com_config.h" 
#include <stdint.h>
 
/*---------------------- Constant / Macro Definitions -----------------------*/
enum {
    ENUM_PRO_MAC_TIME_REPEAT,
    ENUM_PRO_MAC_TIME_ACK
};
 
/*----------------------------- Global Typedefs -----------------------------*/
typedef struct mac_process_context_str{
    uint16_t r_len;
    uint16_t r_checksum;    
    uint8_t r_frame[CFG_COM_PROTOCOL_FRAME_SIZE];
}mac_process_context_t;
/*
 * parameter:
 * ack_packet -- the receive packet
   interval -- the time elapse from the last sent cmd
   cmd_packet -- the cmd last sent
 * rtn:
 * 0 -- not a ACK
 * 1 -- it's a ACK to the spec cmd on time. 
 * 2 -- it's the ACK to the spec cmd, but timeout. 
 * 3 -- it's a ACK, but not suit to the spec cmd.
 * >3 -- reserved
 */
typedef uint8_t fn_is_slave_dev_t(void);
typedef uint8_t fn_is_mac_ack_t(uint8_t *ack_packet, uint32_t interval, uint8_t *cmd_packet);
typedef uint8_t fn_process_mac_frame_t(uint8_t in_data, uint8_t interval, mac_process_context_t * p_context);
typedef uint16_t fn_respone_mac_frame_t(uint8_t *cmd_packet, uint8_t *ack_packet);
typedef uint32_t fn_get_mac_timeout_t(uint8_t send_seq, uint8_t type);

typedef struct com_mac_protocol_fun_str {
    fn_process_mac_frame_t * p_fn_process_mac_frame;
    fn_is_mac_ack_t * p_fn_is_mac_ack;
    fn_respone_mac_frame_t * p_fn_respone_mac_frame;
    fn_get_mac_timeout_t * p_fn_get_mac_timeout;
    fn_is_slave_dev_t *p_fn_judge_mac_send;
}com_mac_protocol_fun_t; 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
void com_protocol_mac_init(ENUM_COM_PORT_T port, com_mac_protocol_fun_t *p_fun); 

#endif //_COM_PROTOCOL_H_
