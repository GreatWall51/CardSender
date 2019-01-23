/******************************************************************************
* Copyright 2010-2011 Renyucjs@163.com
* FileName: 	 com_mac_layer.c 
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
 
 
/*------------------------------- Includes ----------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "com_config.h"
#include "timer_drv.h"
#include "hal_com.h"
#include "mac_com.h"

#if CFG_COM_USE_HEAP
//#include "heap_freertos.h"
#endif /* CFG_COM_USE_HEAP */

#if CFG_COM_USE_HEAP
uint16_t mac_vacancy_cnt = 0;
#else

static com_mac_send_pool_item_t s_mac_send_pool[CFG_COM_MAC_POOL_SIZE]; 
static com_mac_send_pool_item_t *p_mac_vacancy_pool_header;

#endif /* CFG_COM_USE_HEAP */

static com_mac_port_t s_mac_port[CFG_COM_PORT_CNT];
static com_mac_send_packet_t s_mac_ack;

#if CFG_COM_DECISION_LEN
/* 应答缓冲区 */
uint8_t s_mac_ack_data[CFG_COM_MAC_PACKET_SIZE];
#endif

/*----------------------- Function Prototype --------------------------------*/

 
/*----------------------- Function Implement --------------------------------*/

/******************************************************************************
* Name: 	 mac_com_init 
*
* Desc: 	 mac_com层初始化函数
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
void mac_com_init()
{
    uint32_t i;
#if CFG_COM_USE_HEAP
#else
    /* Pool initial */
    for(i = 0; i < sizeof(s_mac_send_pool)/sizeof(s_mac_send_pool[0]); i++)
    {
        s_mac_send_pool[i].packet.times = 0;
        s_mac_send_pool[i].next = &s_mac_send_pool[i+1];
        s_mac_send_pool[i].id = 0;
    }
    s_mac_send_pool[i - 1].next = NULL;
    p_mac_vacancy_pool_header = s_mac_send_pool;

#endif /* CFG_COM_USE_HEAP */

    #if CFG_COM_DECISION_LEN
    s_mac_ack.data = s_mac_ack_data;
    #endif
    /* Port's data initial */
    for(i = 0; i < CFG_COM_PORT_CNT; i++)
    {
        memset(&s_mac_port[i], 0, sizeof(s_mac_port[0]));	/*!< Clear all things */
        #if CFG_COM_DECISION_LEN
        s_mac_port[i].sending.packet.data = s_mac_port[i].sending_data;        
        #endif
       	/*!< initial protocol funs of MAC layer */
        com_protocol_mac_init((ENUM_COM_PORT_T)i, &s_mac_port[i].protocol);
    }
}
/******************************************************************************
* Name: 	 mac_com_real_time 
*
* Desc: 	 mac_com层实时函数
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
void mac_com_real_time()
{ 
    uint32_t i;
    uint8_t data;
    uint8_t interval;
    com_mac_port_t * ptr;

    /* mac layer process*/
    for(i = 0; i < CFG_COM_PORT_CNT; i++)
    {    
        ptr = s_mac_port+i;
        /* Clear event */
        ptr->evt = EVT_COM_MAC_NONE;
        /* 主设备模式 */
        if(ptr->protocol.p_fn_judge_mac_send == NULL)
        {
            /*!< Sending... */
            if(ptr->sending.packet.times == 0)//发送完成，发送下一帧
            {
               	/*!< the sending queue is not empty */
                if(ptr->p_send_head != NULL)
               	{  
               	    ptr->sending = *ptr->p_send_head;	/*!< copy the packet */
                    #if CFG_COM_DECISION_LEN
                    ptr->sending.packet.data = ptr->sending_data;  /* 重新分配缓冲区地址 */
                    memcpy(ptr->sending.packet.data,ptr->p_send_head->packet.data,ptr->p_send_head->packet.len);
                    #endif
            	    /*!< recycle the send pool */            	  
               	    mac_com_push_vacancy_pool(ptr->p_send_head);
           	        ptr->p_send_head = ptr->sending.next;          	        
           	        ptr->sending.packet.times++; 	/*!< Increate the send times,至少会发送一次*/
           	        ptr->sent_cnt = 0;
               	}
            }
            else if(ptr->sending.packet.times == 1)	/*!< the last time */
            {
                if(ptr->sending.packet.flag.f.need_ack)//需要应答
                {
                    /*!< check for ACK timeout */
                    uint32_t wait_ack_timeout = 0;
                    //获取设置的应答超时时间
                    if(ptr->protocol.p_fn_get_mac_timeout != NULL)
                    {                    
                        wait_ack_timeout = ptr->protocol.p_fn_get_mac_timeout(0, ENUM_PRO_MAC_TIME_ACK);
                    }
                    if(hal_com_get_idle_time((ENUM_COM_PORT_T) i, ENUM_HAL_IDLE_SEND) >= wait_ack_timeout \
                    && (!hal_com_is_sending((ENUM_COM_PORT_T)i)))
                    {                      
                        //应答超时
                        ptr->evt = EVT_COM_MAC_TIMEOUT;
                        ptr->sending.packet.flag.f.need_ack = 0;
                        ptr->sending.packet.times = 0;                      
                        continue;	/*!< Break, dispath the event first */
                    }       
                }
                else
                {	/*!< don't need wait for ack, just clear everything */
                    ptr->sending.packet.flag.all = 0;
                    ptr->sending.packet.times = 0;
                }
            }
            else
            { 
                uint32_t re_f_interval = 0;//无应答，过re_f_interval时间重发
                if(ptr->protocol.p_fn_get_mac_timeout != NULL)
                {
                    re_f_interval = ptr->protocol.p_fn_get_mac_timeout(ptr->sent_cnt+1, ENUM_PRO_MAC_TIME_REPEAT);
                }
               
                if((!hal_com_is_sending((ENUM_COM_PORT_T)i)) \
                && (hal_com_get_idle_time((ENUM_COM_PORT_T)i, ENUM_HAL_IDLE_ALL) >= re_f_interval))
                {    
                    hal_com_send((ENUM_COM_PORT_T)i, ptr->sending.packet.data, ptr->sending.packet.len);
                    ptr->sending.packet.times--;
                    ptr->sent_cnt++;
                }           
     
            }
        }
        else  /* 从设备模式 */
        {
            if(ptr->protocol.p_fn_judge_mac_send())  ptr->sent_cnt++;                  
            if(ptr->sent_cnt != 0)
            {
                /*!< the sending queue is not empty */
                if(ptr->p_send_head != NULL)
               	{              	              	    
           	        uint32_t re_f_interval = 0;
                    if(ptr->protocol.p_fn_get_mac_timeout != NULL)
                    {
                        re_f_interval = ptr->protocol.p_fn_get_mac_timeout(ptr->sent_cnt+1, ENUM_PRO_MAC_TIME_REPEAT);
                    }
                   if((!hal_com_is_sending((ENUM_COM_PORT_T)i)) \
                    && (hal_com_get_idle_time((ENUM_COM_PORT_T)i, ENUM_HAL_IDLE_ALL) >= re_f_interval))
                    {                           
                        ptr->sending = *ptr->p_send_head;	/*!< copy the packet */
                	    /*!< recycle the send pool */
                	    hal_com_send((ENUM_COM_PORT_T)i, ptr->sending.packet.data, ptr->sending.packet.len);
                        ptr->sent_cnt--;
                        mac_com_push_vacancy_pool(ptr->p_send_head);
               	        ptr->p_send_head = ptr->sending.next;                        
                    }    
               	}
               	else
               	{
               	    ptr->sent_cnt = 0;
               	}
           	}
        } 
        /*!< Receive data from Hal layer */
        while(hal_com_have_new_data((ENUM_COM_PORT_T)i))
        {
            data = hal_com_receive_data((ENUM_COM_PORT_T)i, &interval);
           
            if(ptr->protocol.p_fn_process_mac_frame != NULL)
            {
                if(ptr->protocol.p_fn_process_mac_frame(data, interval, &ptr->r_context))
                {
                    /*!< Here, we get a new frame */
                    ptr->evt = EVT_COM_MAC_N_FRAME;
                    if(ptr->protocol.p_fn_is_mac_ack != NULL)
                    {
                        uint8_t ask;
                        ask = ptr->protocol.p_fn_is_mac_ack(ptr->r_context.r_frame, hal_com_get_idle_time((ENUM_COM_PORT_T)i, ENUM_HAL_IDLE_SEND), ptr->sending.packet.data);
                        
                        if(ask == 1)
                        {                     
                            if(ptr->sending.packet.flag.f.need_ack)
                            {   
                                ptr->evt = EVT_COM_MAC_ACK;	/*!< we get the right ACK */
                                ptr->sending.packet.times = 0;
                                ptr->sending.packet.flag.f.need_ack = 0;
                                ptr->r_context.r_len = 0;   /*!< 应答帧清除 */
                            }
                            else
                            {
                                ptr->evt = EVT_COM_MAC_NONE;
                            }
                        }
                        else if(ask == 2)   //回复的帧带有信息，保留应答帧，做进一步处理
                        {
                             ptr->sending.packet.times = 0;
                             ptr->sending.packet.flag.all = 0;
                        }
                        else if(ask > 2)
                        {
                            ptr->evt = EVT_COM_MAC_NONE;
                        }
                    }
                   	/*!< Respone *///收到新帧自动应答
                    if(ptr->evt == EVT_COM_MAC_N_FRAME)
                    {
                        if(ptr->protocol.p_fn_respone_mac_frame != NULL)
                        {
                            s_mac_ack.len =  ptr->protocol.p_fn_respone_mac_frame(ptr->r_context.r_frame, s_mac_ack.data);
                            if(s_mac_ack.len > 0)
                            {
                                hal_com_send((ENUM_COM_PORT_T) i, s_mac_ack.data, s_mac_ack.len);
                            }
                        }
                    }
                    break;  
                                 
                }
            }
        }
    }
}
/******************************************************************************
* Name: 	 mac_com_get_event 
*
* Desc: 	 获取mac_com层事件
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
ENUM_COM_MAC_EVT_T mac_com_get_event(ENUM_COM_PORT_T port)
{
    return s_mac_port[port].evt;
}
/******************************************************************************
* Name: 	 mac_com_get_new_frame 
*
* Desc: 	 获取新帧
* InParam: 	 port:端口号
* OutParam:  p_frame:获取到的帧的地址
* Return: 	 >0 -- len, 0 -- we haven't get a new frame
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
uint16_t mac_com_get_new_frame(ENUM_COM_PORT_T port, uint8_t *p_frame)
{
    uint16_t len;
    if(port >= CFG_COM_PORT_CNT) return 0;
    
    len = s_mac_port[port].r_context.r_len;
    memcpy(p_frame, s_mac_port[port].r_context.r_frame, len);

    /*Clear the context*/
    s_mac_port[port].r_context.r_len = 0;
    
    return len;
}
/******************************************************************************
* Name: 	 mac_com_get_new_frame_len 
*
* Desc: 	 获取新帧的长度，即判断是否有新帧,一般不用这个函数，直接用
             mac_com_get_event(port)   == EVT_COM_MAC_N_FRAME
* InParam: 	 port:端口号
* OutParam: 	 
* Return: 	 帧长度
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
uint16_t mac_com_get_new_frame_len(ENUM_COM_PORT_T port)
{
    return s_mac_port[port].r_context.r_len;
}
/******************************************************************************
* Name: 	 mac_com_get_sending_frame 
*
* Desc: 	 获取正在发送的帧
* InParam: 	 port端口号
* OutParam:  **ptr:正在发送的帧的地址
* Return: 	 >0 -- mac_id, 0 -- err
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
uint16_t mac_com_get_sending_frame(ENUM_COM_PORT_T port, com_mac_send_packet_t **ptr)
{
    if(port >= CFG_COM_PORT_CNT) return 0;
    
    *ptr = &s_mac_port[port].sending.packet;
    return s_mac_port[port].sending.id;
}
/******************************************************************************
* Name: 	 mac_com_push_vacancy_pool 
*
* Desc: 	 Set a vacancy item to vacancy_pool
             将新包插入链表池的头节点，缓存数据包
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
 void mac_com_push_vacancy_pool(com_mac_send_pool_item_t * p_recycle)
{

    if(!p_recycle) return;
#if CFG_COM_USE_HEAP
    #if CFG_COM_DECISION_LEN
        if(mac_vacancy_cnt > 0) 
            mac_vacancy_cnt--;
        free(p_recycle->packet.data);
        p_recycle->packet.data = NULL;
        free(p_recycle);
    #else
        free(p_recycle);
    #endif
#else    
    p_recycle->packet.times = 0;
    p_recycle->id = 0;
    p_recycle->next = p_mac_vacancy_pool_header;
    p_mac_vacancy_pool_header = p_recycle;

#endif /* CFG_COM_USE_HEAP */
    return;
}
/******************************************************************************
* Name: 	 mac_com_pop_vacancy_pool 
*
* Desc: 	 Get a vacancy item from vacancy_pool
             从链表池中取出一包数据，准备发送
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
 #if CFG_COM_DECISION_LEN
com_mac_send_pool_item_t * mac_com_pop_vacancy_pool(uint8_t len)
{
    com_mac_send_pool_item_t * p;
#if CFG_COM_USE_HEAP
    if(mac_vacancy_cnt > CFG_COM_MAX_VACANCY_NUM) return NULL;
    mac_vacancy_cnt++;
    p = (com_mac_send_pool_item_t *)malloc(sizeof(com_mac_send_pool_item_t));
    p->packet.data = (uint8_t *)malloc(len);
#else
    if(p_mac_vacancy_pool_header == NULL)       
    {
        return NULL; /* pool is empty */
    }
    p = p_mac_vacancy_pool_header;
    p_mac_vacancy_pool_header = p->next;
        
#endif /* CFG_COM_USE_HEAP */
    
    return p;
}
#else
com_mac_send_pool_item_t * mac_com_pop_vacancy_pool(void)
{
    com_mac_send_pool_item_t * p;
    
#if CFG_COM_USE_HEAP
    p = (com_mac_send_pool_item_t *) malloc(sizeof(com_mac_send_pool_item_t));
#else
    if(p_mac_vacancy_pool_header == NULL)       
    {
        return NULL; /* pool is empty */
    }
    p = p_mac_vacancy_pool_header;
    p_mac_vacancy_pool_header = p->next;
        
#endif /* CFG_COM_USE_HEAP */
    
    return p;
}
#endif

/******************************************************************************
* Name: 	 mac_com_assign_send_packet 
*
* Desc: 	 给数据包分配id
* InParam: 	 
* OutParam: 	 
* Return: 	 数据包id
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t mac_com_assign_send_packet(ENUM_COM_PORT_T port, com_mac_send_pool_item_t * p, ENUM_QUEUE_DIR_T dir)
{

    if(port >= CFG_COM_PORT_CNT) return 0;
    p->next = NULL;
    
    if((s_mac_port[port].p_send_head == NULL) ||(s_mac_port[port].p_send_tail == NULL))
    {
        /*!< Send queue is empty */
        s_mac_port[port].p_send_head = p;
        s_mac_port[port].p_send_tail = p;         
    }
    else
    {
        
        if(dir == APPEND_QUEUE_TAIL)
        {
            s_mac_port[port].p_send_tail->next = p;
            s_mac_port[port].p_send_tail = p;
        }
        else
        {
            p->next = s_mac_port[port].p_send_head;
            s_mac_port[port].p_send_head  = p;
        }       
        
    }
    s_mac_port[port].packet_seq++; /*!< increase mac id */
    if(s_mac_port[port].packet_seq == 0) s_mac_port[port].packet_seq++; /*!< we don't use 0 */
    p->id = s_mac_port[port].packet_seq;  
    
    return p->id;
}
/******************************************************************************
* Name: 	 mac_com_send 
*
* Desc: 	 the send api for higher layer
             应用层调用该函数进行发送数据包
* InParam: 	 
* OutParam: 	 
* Return: 	 0 -- fail,  > 0 -- the frame id
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/16, Create this function by LiuWeiQiang
 ******************************************************************************/
 #if CFG_COM_DECISION_LEN
uint8_t mac_com_send(ENUM_COM_PORT_T port, com_mac_send_packet_t **p_packet, ENUM_QUEUE_DIR_T dir, uint8_t len)
{
    com_mac_send_pool_item_t * p;

    if(port >= CFG_COM_PORT_CNT) return 0;
    
    p = mac_com_pop_vacancy_pool(len);
    if(!p) /*!< Pool empty, return fail */
    {
        *p_packet = NULL;
        return 0;	
    }
    *p_packet = &(p->packet);
    
   	/*!< Add to the tail of spec port's send queue */
   	return mac_com_assign_send_packet(port,p,   dir);
}
#else
uint8_t mac_com_send(ENUM_COM_PORT_T port, com_mac_send_packet_t **p_packet, ENUM_QUEUE_DIR_T dir)
{
    com_mac_send_pool_item_t * p;

    if(port >= CFG_COM_PORT_CNT) return 0;
    
    p = mac_com_pop_vacancy_pool();
    if(!p) return 0;	/*!< Pool empty, return fail */
    *p_packet = &(p->packet);
    
   	/*!< Add to the tail of spec port's send queue */
   	return mac_com_assign_send_packet(port,p,   dir);
}
#endif

com_mac_protocol_fun_t *get_mac_com_protocol_addr(ENUM_COM_PORT_T com_port)
{
    return &s_mac_port[com_port].protocol;
}


com_mac_send_pool_item_t *get_mac_com_send_head(ENUM_COM_PORT_T com_port)
{
    return s_mac_port[com_port].p_send_head;
}

com_mac_port_t * get_mac_com_port(ENUM_COM_PORT_T com_port)
{
    return &s_mac_port[com_port];
}
/*---------------------------------------------------------------------------*/
void mac_com_reset(ENUM_COM_PORT_T com_port)
{
    com_mac_send_pool_item_t *p_item_move;

    ENTER_CRITICAL();    
    p_item_move = s_mac_port[com_port].p_send_head;   

    while(p_item_move != NULL)
    {       
        mac_com_push_vacancy_pool(p_item_move);
        p_item_move = p_item_move->next;
    }
    s_mac_port[com_port].p_send_head = NULL;
    memset(&s_mac_port[com_port].r_context,0,sizeof(s_mac_port[com_port].r_context));
    EXIT_CRITICAL();
}
