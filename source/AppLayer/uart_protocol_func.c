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
* Desc: 	 ��ȡУ���
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
* Desc: 	 �ж�У����Ƿ���ȷ
* InParam: 	 
* OutParam: 	 
* Return: 	 1->��ȷ��0->����
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
* Desc: 	 ����com_hal������ݣ���װ��֡
* InParam: 	 in_data->com_hal���յ�������
             interval->com_hal���յ�����������һ�����ݵ�ʱ����
* OutParam:  p_context->��װcom_hal�����гɵ�֡
* Return: 	 1->���ݷ�װ�ɹ���У��ɹ���0->ʧ�ܣ�������֡
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/14, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t com_pro_uart_process_frame(uint8_t in_data, uint8_t interval, mac_process_context_t* p_context)
{
    if(interval >= COM_PRO_UART_MIN_INTERVAL / TIME_BASE)
    {
        //����COM_PRO_UART_MIN_INTERVAL����Ϊһ֡������ɣ�׼��������һ֡
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
* Desc: 	 ��ʱ�ط�ʱ���Ӧ��ʱʱ������
* InParam: 	 send_seq:���ݰ�id
			 type: 
             ENUM_PRO_MAC_TIME_REPEAT->������֮֡��ļ������һ֡����Ҫ���
             ENUM_PRO_MAC_TIME_ACK->Ӧ��ʱʱ�䣬����nʱ����û�յ�Ӧ�����ΪӦ��ʱ
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
        //ע��:��Ҫͬʱ�Ӵ�CFG_COM_HAL_MAX_INTERVAL_CNT��ֵ,CFG_COM_HAL_MAX_INTERVAL_CNT>return ֵ
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
* Desc: 	 ���յ���Ӧ��֡�����ж�
* InParam: 	 
* OutParam: 	 
* Return: 	 1->�յ�Ӧ��֡��2->�յ���֡������Ϣ��3->�յ�������Э�������
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
        return 1;//�յ���ȷӦ��֡
    }
    return 3;
}

/******************************************************************************
* Name: 	 com_pro_uart_respone_frame 
*
* Desc: 	 �Զ�Ӧ��֡����
* InParam: 	 cmd_packet->���յ�������֡
* OutParam:  ack_packet->��������֡��ɵ�Ӧ��֡
* Return: 	 
* Note: 	 ע�ⲻҪ����ҪӦ���֡���й���
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

    if(cmd_frame->cmd != CARD_OPERATION)  return 0;//���������ֶԲ���ҪӦ�������֡���й���
    
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
