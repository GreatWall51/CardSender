/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 hal_com.h 
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
#ifndef _HAL_COM_H_     
#define _HAL_COM_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include "com_config.h" 
#include <stdint.h> 
 
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------------- Global Typedefs -----------------------------*/
 enum {
    ENUM_HAL_SEND_STA_IDLE = 0,    //����
    ENUM_HAL_SEND_STA_PRE_SEND,    //׼������
    ENUM_HAL_SEND_STA_SENDING,     //���ڷ���
    ENUM_HAL_SEND_STA_FINISH,      //���ͽ���
};

typedef enum {
    ENUM_HAL_IDLE_SEND, 
    ENUM_HAL_IDLE_REV,
    ENUM_HAL_IDLE_ALL,
}ENUM_HAL_IDLE_TIME_T; 

typedef void fn_hal_void_fun_t(void);
typedef void fn_hal_send_fun_t(uint8_t data);

typedef struct{
    fn_hal_void_fun_t *p_fn_hal_init;
    fn_hal_void_fun_t *p_fn_hal_start_send;
    fn_hal_void_fun_t *p_fn_hal_stop_send;
    fn_hal_send_fun_t *p_fn_hal_send;
}hal_com_fun_t;  

typedef struct{
    /* receive */
    uint8_t r_cyc_buffer[CFG_COM_HAL_RECEIVE_CYC_SIZE];
    uint8_t r_cyc_interval[CFG_COM_HAL_RECEIVE_CYC_SIZE];
    uint8_t r_cyc_head; //ȡ��������������
    uint8_t r_cyc_tail; //�������ݴ������
    uint8_t r_cyc_number;//��ǰ���ջ��������ݸ���
    uint16_t r_interval_timer;	/*!< timer to calculate time between two receive char */

    /* send */
    uint16_t s_len;//�����������ܵĳ���
    uint16_t s_timer;  //���ͳ�ʱʱ��
    uint32_t s_status;//����״̬
    uint8_t s_sent_point;//��ǰ�����ֽڵ�����
    uint8_t s_frame[CFG_COM_HAL_SEND_BUF_SIZE];//�������ݻ�����
    uint16_t s_interval_timer; /*!< time between two send char */
    
    /* hal portable functions */
    hal_com_fun_t hal_funs;
}hal_com_port_t; 
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
void hal_com_init(void); 
void hal_com_real_time(void); 
void hal_com_timer_real_time(void);
uint8_t hal_com_have_new_data(ENUM_COM_PORT_T port);
uint8_t hal_com_receive_data(ENUM_COM_PORT_T port, uint8_t *p_interval);
void hal_com_receive_irq_callback(ENUM_COM_PORT_T port, uint8_t data);
void hal_com_send_irq_callback(ENUM_COM_PORT_T port);
uint8_t hal_com_is_sending(ENUM_COM_PORT_T port);
uint8_t hal_com_send(ENUM_COM_PORT_T port, uint8_t *p_buf, uint16_t len);
uint32_t hal_com_get_idle_time(ENUM_COM_PORT_T port, ENUM_HAL_IDLE_TIME_T type);
hal_com_port_t * hal_com_get_port_info(ENUM_COM_PORT_T port);
#endif //_HAL_COM_H_
