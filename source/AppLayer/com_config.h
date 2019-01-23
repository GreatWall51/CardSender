/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 com_config.h 
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
#ifndef _COM_CONFIG_H_     
#define _COM_CONFIG_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
 
 
/*---------------------- Constant / Macro Definitions -----------------------*/
#define Version1 0x01
#define Version2 0x00
#define  ENTER_CRITICAL()     do{__disable_irq();}while(0)
#define  EXIT_CRITICAL()      do{__enable_irq();}while(0)

/** global config **/
#define CFG_COM_USE_HEAP         1 	/*!< we use heap or not ? */
#define CFG_COM_DECISION_LEN 	   0/*!< we decide to len or not ? */
#define CFG_COM_MAX_VACANCY_NUM      10

/**  HAL Layer config **/
#define CFG_COM_HAL_SEND_BUF_SIZE      64
#define CFG_COM_HAL_MAX_INTERVAL_CNT    250 /* 最大的字符间隔计数 */
#define CFG_COM_HAL_MAX_SEND_TIME (300/TIME_BASE) /* 300ms *///最大发送时间
#define CFG_COM_HAL_RECEIVE_CYC_SIZE 32 	/*!< 2*N */
#define COM_HAL_CYC_MASK ((CFG_COM_HAL_RECEIVE_CYC_SIZE)-1)

/**  MAC Layer config **/
#define CFG_COM_MAC_PACKET_SIZE 50

#if CFG_COM_USE_HEAP
#else
#define CFG_COM_MAC_POOL_SIZE      20  	/*!< if we don't use heap, need a fixed pool size */
#endif /* CFG_COM_USE_HEAP */

/** Protocol Frame Size **/
#define CFG_COM_PROTOCOL_FRAME_SIZE 250
/*----------------------------- Global Typedefs -----------------------------*/
 typedef enum {
    PORT_CARD_SENDER,
	 
    CFG_COM_PORT_CNT,  
}ENUM_COM_PORT_T;
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 
#endif //_COM_CONFIG_H_
