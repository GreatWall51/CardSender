/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 hal_com.c 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/06/13
* Notes:     ʹ��˵��:
			 1.����ʼ���ͷ�����غ���ע�ᵽע���c_hal_fun_config��
			 2.������/���ռ�ʱ����hal_com_timer_real_time����1ms��ʱ����
			 3.�����ջص�����hal_com_receive_irq_callback�������һ�ֽ��жϴ�������
			 4.�����ͻص�����hal_com_send_irq_callback���뷢��һ�ֽ��жϴ�������
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/13, LiuWeiQiang create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include "hal_com.h" 
#include "com_config.h"
#include "timer_drv.h"
#include <string.h>
#include "hal_com_func.h" 
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/
 
 
/*------------------- Global Definitions and Declarations -------------------*/
const hal_com_fun_t c_hal_fun_config[CFG_COM_PORT_CNT] = 
{
    {usart1_init, usart1_start_send,usart1_stop_send,usart1_send},
};
hal_com_port_t s_hal_port[CFG_COM_PORT_CNT];
 
/*----------------------- Variable Declarations -----------------------------*/
 
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/



/*----------------------- Function Prototype --------------------------------*/
/******************************************************************************
* Name: 	 hal_com_init 
*
* Desc: 	 hal_com��ʼ��
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
void hal_com_init(void)
{
    uint32_t i;
    
    /* Port's data initial */
    for(i = 0; i < sizeof(s_hal_port)/sizeof(s_hal_port[0]); i++)
    {
        /*!< Clear all things */
        memset(&s_hal_port[i], 0, sizeof(hal_com_port_t));
       	/*!< initial hal portable functions */
       	s_hal_port[i].hal_funs =  c_hal_fun_config[i];
        /* init the next hw layer */
       	if(s_hal_port[i].hal_funs.p_fn_hal_init != NULL) 
       	{
       	    s_hal_port[i].hal_funs.p_fn_hal_init();
       	}
    }
} 

/******************************************************************************
* Name: 	 hal_com_real_time 
*
* Desc: 	 hal_com��ʵʱ����
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
void hal_com_real_time(void)
{		 
    uint8_t i;
    for(i = 0; i < CFG_COM_PORT_CNT; i++)
    {
        if(s_hal_port[i].s_status == ENUM_HAL_SEND_STA_PRE_SEND)
        {
            s_hal_port[i].s_status = ENUM_HAL_SEND_STA_SENDING;
            
            ENTER_CRITICAL();
            s_hal_port[i].s_timer = CFG_COM_HAL_MAX_SEND_TIME;
            EXIT_CRITICAL();
            
            if(s_hal_port[i].hal_funs.p_fn_hal_start_send != NULL)
                    s_hal_port[i].hal_funs.p_fn_hal_start_send();
        }
        else if(s_hal_port[i].s_status == ENUM_HAL_SEND_STA_FINISH)
        {
            if(s_hal_port[i].hal_funs.p_fn_hal_stop_send != NULL)
               s_hal_port[i].hal_funs.p_fn_hal_stop_send();
            s_hal_port[i].s_status = ENUM_HAL_SEND_STA_IDLE;
            ENTER_CRITICAL();
            s_hal_port[i].s_timer = 0;
            EXIT_CRITICAL();
            s_hal_port[i].s_len = 0;
            s_hal_port[i].s_sent_point = 0;
        }
#if 0 //����1ms�¼����ܻᶪʧ���ʶ�����void hal_com_timer_real_time(void)���뵥���Ķ�ʱ���ж���
        if(IS_TIMER_EVT(TIME_EVT_1))
        {
            if(s_hal_port[i].s_timer)
            {
                s_hal_port[i].s_timer--;
                if(s_hal_port[i].s_timer == 0)
                {	/*!< Something wrong, restart */
                    s_hal_port[i].s_status = ENUM_HAL_SEND_STA_FINISH;
                }
            }
            ENTER_CRITICAL();
            if(s_hal_port[i].r_interval_timer < CFG_COM_HAL_MAX_INTERVAL_CNT) s_hal_port[i].r_interval_timer++;
            if(s_hal_port[i].s_interval_timer < CFG_COM_HAL_MAX_INTERVAL_CNT) s_hal_port[i].s_interval_timer++;
            EXIT_CRITICAL();
        }
#endif /* 0 */
    }
    return;
}
/******************************************************************************
* Name: 	 hal_com_timer_real_time 
*
* Desc: 	 ����/���տ��м�ʱ
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
void hal_com_timer_real_time(void)
{
    uint32_t i;
    for(i = 0; i < CFG_COM_PORT_CNT; i++)
    {
        if(s_hal_port[i].s_timer)
        {
            s_hal_port[i].s_timer--;
            if(s_hal_port[i].s_timer == 0)
            {	/*!< Something wrong, restart */
                s_hal_port[i].s_status = ENUM_HAL_SEND_STA_FINISH;
            }
        }
        ENTER_CRITICAL();
        if(s_hal_port[i].r_interval_timer < CFG_COM_HAL_MAX_INTERVAL_CNT) s_hal_port[i].r_interval_timer++;
        if(s_hal_port[i].s_interval_timer < CFG_COM_HAL_MAX_INTERVAL_CNT) s_hal_port[i].s_interval_timer++;
        EXIT_CRITICAL();        
    }
}


/******************************************************************************
* Name: 	 com_hal_receive_irq_callback 
*
* Desc: 	 ����һ�ֽ������жϻص�����
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
void hal_com_receive_irq_callback(ENUM_COM_PORT_T port, uint8_t data)
{
    hal_com_port_t *p_port;
    p_port = &s_hal_port[port];
    p_port->r_cyc_buffer[p_port->r_cyc_tail & COM_HAL_CYC_MASK] = data;
    p_port->r_cyc_interval[p_port->r_cyc_tail & COM_HAL_CYC_MASK] = p_port->r_interval_timer;

    ENTER_CRITICAL();
    p_port->r_cyc_tail++;
    p_port->r_cyc_number++;
    p_port->r_interval_timer = 0;
    EXIT_CRITICAL();
}
/******************************************************************************
* Name: 	 com_hal_send_irq_callback 
*
* Desc: 	 ����һ�ֽ������жϻص�����
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
void hal_com_send_irq_callback(ENUM_COM_PORT_T port)
{
    hal_com_port_t *p_port;

    p_port = &s_hal_port[port];

    if(p_port->s_len > p_port->s_sent_point)
    {    
        p_port->hal_funs.p_fn_hal_send(p_port->s_frame[p_port->s_sent_point]);
        ENTER_CRITICAL();
        p_port->s_sent_point++;
        p_port->s_interval_timer = 0;
        EXIT_CRITICAL();            
    }
    else
    {
        p_port->s_status = ENUM_HAL_SEND_STA_FINISH;
        if(p_port->hal_funs.p_fn_hal_stop_send != NULL)
        {
            p_port->hal_funs.p_fn_hal_stop_send();
        }
    }
}


/******************************************************************************
* Name: 	 com_hal_receive_data 
*
* Desc: 	 hal_com��������drv������
* InParam: 	 port->ͨ�Ŷ˿�
* OutParam:  p_interval->���ֽ�ʱ����
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t hal_com_receive_data(ENUM_COM_PORT_T port, uint8_t *p_interval)
{
    hal_com_port_t *p_port;
    uint8_t data = 0;
    
    p_port = &s_hal_port[port];
    
    if(p_port->r_cyc_number > 0)
    {
        data = p_port->r_cyc_buffer[p_port->r_cyc_head & COM_HAL_CYC_MASK]; //�ӻ��λ�������������
        *p_interval = p_port->r_cyc_interval[p_port->r_cyc_head & COM_HAL_CYC_MASK];
        ENTER_CRITICAL();
        p_port->r_cyc_head++; //���մ���ָ��ǰ��
        p_port->r_cyc_number--; //δ���������1
        EXIT_CRITICAL();
    }
    return data;
}
/******************************************************************************
* Name: 	 com_hal_have_new_data 
*
* Desc: 	 �����ж�hal_com�Ƿ���յ�����
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t hal_com_have_new_data(ENUM_COM_PORT_T port)
{
    return s_hal_port[port].r_cyc_number;
}


/******************************************************************************
* Name: 	 com_hal_send 
*
* Desc: 	 hal_com��������
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
 uint8_t hal_com_send(ENUM_COM_PORT_T port, uint8_t *p_buf, uint16_t len)
{
    hal_com_port_t *p_port;

    if((port >= CFG_COM_PORT_CNT) || (len > CFG_COM_HAL_SEND_BUF_SIZE) || (p_buf == NULL)) return 0;
    
    p_port = s_hal_port + port;
    if(p_port->s_status != ENUM_HAL_SEND_STA_IDLE) return 0;
    memcpy(p_port->s_frame, p_buf, len);
    p_port->s_sent_point = 0;
    p_port->s_len = len;
    p_port->s_status = ENUM_HAL_SEND_STA_PRE_SEND;
    return 1;
}

/******************************************************************************
* Name: 	 com_hal_is_sending 
*
* Desc: 	 �жϷ��ͽӿ��Ƿ����
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t hal_com_is_sending(ENUM_COM_PORT_T port)
{
    return (s_hal_port[port].s_status != ENUM_HAL_SEND_STA_IDLE);
}

/******************************************************************************
* Name: 	 hal_com_get_idle_time 
*
* Desc: 	 ��ȡ���ͻ���տ���ʱ�䣬��������ʱ�䣬��Ϊ���ͻ����һ֡�������
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
uint32_t hal_com_get_idle_time(ENUM_COM_PORT_T port, ENUM_HAL_IDLE_TIME_T type)
{
    uint32_t s_time, r_time;
    uint32_t time = 0;

    s_time = s_hal_port[port].s_interval_timer;
    r_time = s_hal_port[port].r_interval_timer;

    
    if(type == ENUM_HAL_IDLE_SEND)
    {
        time = s_time*TIME_BASE;
    }
    else if(type == ENUM_HAL_IDLE_REV)
    {
        time = r_time*TIME_BASE;
    }
    else
    {
        if(s_time > r_time)
        {
            time = r_time;
        }
        else
        {
            time = s_time;
        }

        time *= TIME_BASE;
    }
    return time;
}

/******************************************************************************
* Name: 	 hal_com_get_port_info 
*
* Desc: 	 ��ȡ�˿���Ϣ
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/15, Create this function by LiuWeiQiang
 ******************************************************************************/
hal_com_port_t *hal_com_get_port_info(ENUM_COM_PORT_T port)
{
     hal_com_port_t *p_port;
     p_port = &s_hal_port[port];

     return p_port;
} 
