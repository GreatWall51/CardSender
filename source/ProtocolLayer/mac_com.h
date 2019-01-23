/******************************************************************************
* Copyright 2010-2011 Renyucjs@163.com
* FileName: 	 com_mac_layer.h 
* Desc:
* 
* 
* Author: 	 Kimson
* Date: 	 2015/03/02
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2015/03/02, Kimson create this file
* 
******************************************************************************/
#ifndef _COM_MAC_LAYER_H_     
#define _COM_MAC_LAYER_H_     

/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
#include "com_config.h" 
#include "com_protocol.h"
 
/*----------------------------- Global Defines ------------------------------*/

 
/*----------------------------- Global Typedefs -----------------------------*/
typedef enum {
    APPEND_QUEUE_HEAD, //插入队列头
    APPEND_QUEUE_TAIL, //插入队列尾
}ENUM_QUEUE_DIR_T;


typedef enum {
    EVT_COM_MAC_NONE, 	    /*!< nothing happend */
    EVT_COM_MAC_N_FRAME,	/*!< receive new frame */
    EVT_COM_MAC_ACK, 	    /*!< get a ACK */
    EVT_COM_MAC_TIMEOUT 	/*!< timeout without ACK */
}ENUM_COM_MAC_EVT_T;

typedef struct com_mac_send_packet_str {
    union {
        uint16_t all;
        struct {
        uint16_t need_ack: 1;
        uint16_t reserved: 15;
        }f;
    }flag;//是否需要应答
    uint16_t len; 	/*!< len of data */ 
#if CFG_COM_DECISION_LEN
    uint8_t *data;
#else
    uint8_t data[CFG_COM_MAC_PACKET_SIZE];
#endif    
    uint8_t times;	//重发次数/*!< times to be sent, if this count down to 0, means this packet is empty */
                    //比如初始化times=0，发送一次;times=1，重发一次,times=2，重发2次
}com_mac_send_packet_t;//发送数据包

typedef struct com_mac_send_pool_item_str {
    com_mac_send_packet_t packet;
    uint8_t id; /*!< the sequence of the packet to be send, it is used to identify the packet */
    struct com_mac_send_pool_item_str *next;
}com_mac_send_pool_item_t;//发送数据的链表


typedef struct com_mac_port_str {
    ENUM_COM_MAC_EVT_T evt;                  //mac层事件
    /* send */
    com_mac_send_pool_item_t sending;        //发送链表
    com_mac_send_pool_item_t *p_send_head; //发送数据链表头指针
    com_mac_send_pool_item_t *p_send_tail; //发送数据链表尾指针
    
    uint16_t sent_cnt;//重发次数计数
    #if CFG_COM_DECISION_LEN
    /* 正在发送缓冲区 (sending.packet.data)*/
    uint8_t sending_data[CFG_COM_MAC_PACKET_SIZE];
    #endif
    /* receive */
    mac_process_context_t r_context;
    /* mac protocol functions */
    com_mac_protocol_fun_t protocol;
    /* mac packet sequence number */
    uint8_t packet_seq;//数据包序列号，即包的id
}com_mac_port_t;  


/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
void mac_com_init(void);
void mac_com_real_time(void); 

/*
** Hight level send api
** return: 0 -- fail,  > 0 -- the frame id
**
*/
#if CFG_COM_DECISION_LEN
uint8_t mac_com_send(ENUM_COM_PORT_T port, com_mac_send_packet_t ** p_packet,ENUM_QUEUE_DIR_T dir,uint8_t len);
#else
uint8_t mac_com_send(ENUM_COM_PORT_T port, com_mac_send_packet_t ** p_packet,ENUM_QUEUE_DIR_T dir);
#endif
/*
** low level send api 
** step 1. mac_com_pop_vacancy_pool 
** step 2. mac_com_assign_send_packet
*/
#if CFG_COM_DECISION_LEN
com_mac_send_pool_item_t * mac_com_pop_vacancy_pool(uint8_t len); 
#else
com_mac_send_pool_item_t * mac_com_pop_vacancy_pool(void); 
#endif
void mac_com_push_vacancy_pool(com_mac_send_pool_item_t * p_recycle);


ENUM_COM_MAC_EVT_T mac_com_get_event(ENUM_COM_PORT_T port); 
uint16_t mac_com_get_new_frame(ENUM_COM_PORT_T port, uint8_t *p_frame);
uint16_t mac_com_get_new_frame_len(ENUM_COM_PORT_T port);
uint16_t mac_com_get_sending_frame(ENUM_COM_PORT_T port, com_mac_send_packet_t **ptr);
void mac_com_reset(ENUM_COM_PORT_T com_port);

void uart_send_data(ENUM_COM_PORT_T port,uint8_t *buf,uint8_t len,uint8_t need_ask,uint8_t repeat_times);
com_mac_protocol_fun_t *get_mac_com_protocol_addr(ENUM_COM_PORT_T com_port);
com_mac_send_pool_item_t *get_mac_com_send_head(ENUM_COM_PORT_T com_port);
com_mac_port_t * get_mac_com_port(ENUM_COM_PORT_T com_port);

#endif //_COM_MAC_LAYER_H_
