/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 uart_protocol.c 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/06/14
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/14, LiuWeiQiang create this file
* 
******************************************************************************/
 
/*------------------------------- Includes ----------------------------------*/
#include "uart_protocol_func.h"
#include "timer_drv.h"
#include "mac_com.h"
#include <string.h>
#include <stdlib.h>
#include "debug.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/
 
 
/*----------------------- Variable Declarations -----------------------------*/
 
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name: 	 get_check_sum 
*
* Desc: 	 获取校验和
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/14, Create this function by LiuWeiQiang
 ******************************************************************************/
 uint8_t  get_check_sum(uint8_t *point, uint8_t len)
{
    uint8_t *pbuf;
    uint8_t i;
    uint8_t sum = 0;

    if(point == NULL || len < 1) return 0;
    
    pbuf = (uint8_t *)point;
    
    for(i=1; i<(len-1); i++)
    {
        sum += *(pbuf+i);
    }
    return (0 - sum);
}

/******************************************************************************
* Name: 	 check_sum 
*
* Desc: 	 判断校验和是否正确
* InParam: 	 
* OutParam: 	 
* Return: 	 1->正确，0->错误
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/14, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t check_sum(uint8_t *point, uint8_t len)
{
    uint8_t *pbuf;
    uint8_t i;
    uint8_t sum = 0;

    if(point == NULL || len < 1) return 0xff;
    
    pbuf = point;
    
    for(i=1; i<len; i++)
    {
        sum += *(pbuf+i);
    }
    if(sum == 0)    return 1;     
    return 0;    
}
/******************************************************************************
* Name: 	 com_pro_uart_process_frame 
*
* Desc: 	 接收com_hal层的数据，封装成帧
* InParam: 	 in_data->com_hal接收到的数据
             interval->com_hal接收到的数据以上一个数据的时间间隔
* OutParam:  p_context->封装com_hal数据行成的帧
* Return: 	 1->数据封装成功且校验成功，0->失败，丢弃该帧
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/14, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t com_pro_uart_process_frame(uint8_t in_data, uint8_t interval, mac_process_context_t* p_context)
{
    if(interval >= COM_PRO_UART_MIN_INTERVAL / TIME_BASE)
    {
        //超过COM_PRO_UART_MIN_INTERVAL，认为一帧接收完成，准备接收下一帧
        p_context->r_len = 0;
        p_context->r_checksum = 0;
    }
    switch(p_context->r_len)
    {
        case 0: /* waite for header */
        {
            if(in_data == PRO_UART_HEAD)
            {
                p_context->r_frame[p_context->r_len] = in_data;
                p_context->r_len++;
            }
        }break;
        case 1: /* we get len  */
        {
            if((in_data > CFG_COM_PROTOCOL_FRAME_SIZE) || (in_data < PRO_UART_HEAD_SIZE)) /* overflow*/
            {
                p_context->r_len = 0;
            }
            else
            {
                p_context->r_frame[p_context->r_len] = in_data;
                p_context->r_len++;
            }
        }break;
        default: /* receive data untill the end  */
        {
            p_context->r_frame[p_context->r_len] = in_data;
            p_context->r_len++;
            if(p_context->r_len >= (p_context->r_frame[PRO_UART_OFFSET_LEN] + 1))
            {
				debug("com_pro_uart_process_frame:\r\n");
				debug_hex(p_context->r_frame, p_context->r_len);
               if(check_sum(p_context->r_frame, p_context->r_len))      return 1;
            }
        }break;
    }		
    return 0;
}

/******************************************************************************
* Name: 	 com_pro_uart_get_mac_timeout_t 
*
* Desc: 	 超时重发时间和应答超时时间设置
* InParam: 	 send_seq:数据包id
			 type: 
             ENUM_PRO_MAC_TIME_REPEAT->发送两帧之间的间隔，第一帧不需要间隔
             ENUM_PRO_MAC_TIME_ACK->应答超时时间，即在n时间内没收到应答就认为应答超时
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/14, Create this function by LiuWeiQiang
 ******************************************************************************/
uint32_t  com_pro_uart_get_mac_timeout_t(uint8_t send_seq, uint8_t type)
{
    if(type == ENUM_PRO_MAC_TIME_REPEAT)
    {
        if(send_seq <= 1) 
        {
            return 1;/* the first frame, we don't need more interval */
        }
        //注意:需要同时加大CFG_COM_HAL_MAX_INTERVAL_CNT的值,CFG_COM_HAL_MAX_INTERVAL_CNT>return 值
        return 50;// mini repeat frame interval in ms
    }
    else if(type == ENUM_PRO_MAC_TIME_ACK)
    {
        // get ask timeout in ms 
        return 100;
    }
    return 0;
}
/******************************************************************************
* Name: 	 com_pro_uart_mac_ack 
*
* Desc: 	 对收到的应答帧进行判断
* InParam: 	 
* OutParam: 	 
* Return: 	 1->收到应答帧，2->收到的帧带有信息，3->收到不符合协议的数据
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/14, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t  com_pro_uart_mac_ack(uint8_t *ack_packet, uint32_t interval, uint8_t *cmd_packet)
{
    uart_frame_head_t *ack_frame;
    uart_frame_head_t *cmd_frame;
    
    ack_frame = (uart_frame_head_t *)ack_packet;
    cmd_frame = (uart_frame_head_t *)cmd_packet;
    switch(ack_frame->cmd)
    {
        case CARD_OPERATION:
            return 2;
		case READ_OBJECT_ATTRIBUTE:
			return 2;
		case WRITE_OBJECT_ATTRIBUTE:
			return 2;
		
        default:
        break;
    }
    
    if(ack_frame->cmd == (cmd_frame->cmd | 0x80) && (ack_frame->frame_id == cmd_frame->frame_id))
    {
        return 1;//收到正确应答帧
    }
    return 3;
}

/******************************************************************************
* Name: 	 com_pro_uart_respone_frame 
*
* Desc: 	 自动应答帧设置
* InParam: 	 cmd_packet->接收到的命令帧
* OutParam:  ack_packet->根据命令帧组成的应答帧
* Return: 	 
* Note: 	 注意不要对需要应答的帧进行过滤
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/14, Create this function by LiuWeiQiang
 ******************************************************************************/
uint16_t  com_pro_uart_respone_frame(uint8_t *cmd_packet, uint8_t *ack_packet)
{
    uart_frame_head_t *ack_frame;
    uart_frame_head_t *cmd_frame;
    
    ack_frame = (uart_frame_head_t *)ack_packet;
    cmd_frame = (uart_frame_head_t *)cmd_packet;

    if(cmd_frame->cmd != CARD_OPERATION)  return 0;//根据命令字对不需要应答的命令帧进行过滤
    
    ack_frame->head = PRO_UART_HEAD;
    ack_frame->len  = 6;
    ack_frame->dest_addr = cmd_frame->src_addr;
    ack_frame->src_addr  = cmd_frame->dest_addr;
    ack_frame->frame_id  = cmd_frame->frame_id;
    ack_frame->cmd = cmd_frame->cmd | 0x80;
    
    ack_packet[ack_frame->len] = get_check_sum(ack_packet, ack_frame->len+1);
    
    return (ack_frame->len+1);
}
 
/*---------------------------------------------------------------------------*/
