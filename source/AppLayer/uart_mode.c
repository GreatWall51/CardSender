/******************************************************************************
* Copyright 2016-2021 liuweiqiang@leelen.cn
* FileName: 	 uart_mode.c 
* Desc:
* 
* 
* Author: 	 lwq
* Date: 	 2016/07/25
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2016/07/25, lwq create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include "uart_mode.h"
#include "hal_com.h"
#include "mac_com.h"
#include "uart_protocol_func.h"
#include "string.h"
#include "uart_drv.h"
//#include "debug.h"
/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/

 
/*----------------------- Variable Declarations -----------------------------*/
 uart_cmd_fun_t uart_cmd_fun[UART_MAX_REG_FUNC];
 static uint8_t  uart_already_reg_func_num = 0;
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
static void com_drv_init(void)
{
	hal_com_init();
	mac_com_init();
}
static void com_drv_real_time(void)
{
    hal_com_real_time();
    mac_com_real_time();
}

/******************************************************************************
* Name: 	 uart_callback_register 
*
* Desc: 	 回调函数注册
* InParam: 	 cmd->命令字，callback->回调函数
* OutParam: 	 
* Return: 	 1->注册成功，0->注册失败
* Note: 	 重复注册的话，后注册的函数会覆盖新注册的函数
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/17, Create this function by LiuWeiQiang
 ******************************************************************************/
uint8_t uart_callback_register(uint8_t cmd, uart_analy_frame callback)
{
	uint8_t i;

	if(uart_already_reg_func_num >= UART_MAX_REG_FUNC) return 0;
	
	for(i=0; i<uart_already_reg_func_num; i++)//判断是否已经注册
	{
		if(uart_cmd_fun[i].cmd == cmd)
		{
			uart_cmd_fun[i].call_back = callback;
            return 0;
		}
	}
	if(i == uart_already_reg_func_num)
	{
        uart_cmd_fun[i].cmd = cmd;
        uart_cmd_fun[i].call_back = callback;
        uart_already_reg_func_num++;
	}
	return 1;
}
/******************************************************************************
* Name: 	 uart_callback_unregister 
*
* Desc: 	 取消回调函数注册
* InParam: 	 cmd->命令字
* OutParam: 	 
* Return: 	
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/17, Create this function by LiuWeiQiang
 ******************************************************************************/
void uart_callback_unregister(uint8_t cmd)
{
	uint8_t i;

	if(uart_already_reg_func_num >= UART_MAX_REG_FUNC) return ;
	
	for(i=0; i<uart_already_reg_func_num; i++)
	{
		if(uart_cmd_fun[i].cmd == cmd)
		{
			uart_cmd_fun[i].call_back = NULL;
            return;
		}
	}
}

/******************************************************************************
* Name: 	 check_uart_evt_real_time 
*
* Desc: 	 事件处理实时函数，根据注册的命令字执行相应的回调函数
* InParam: 	 
* OutParam: 	 
* Return: 	 
* Note: 	 
* Author: 	 LiuWeiQiang
* -----------------------------------------------------------------------------
* Log: 	 2017/06/17, Create this function by LiuWeiQiang
 ******************************************************************************/
static void check_uart_evt_real_time(void)
{
    uint8_t len,i;
    uint8_t buf[250];
    uart_frame_head_t *uart_frame_head;

    if(mac_com_get_event(PORT_CARD_SENDER) == EVT_COM_MAC_N_FRAME)
    {
        len = mac_com_get_new_frame(PORT_CARD_SENDER, buf);
        if(len < sizeof(uart_frame_head_t)) return;
        uart_frame_head = (uart_frame_head_t *)buf;
        for(i=0; i<uart_already_reg_func_num; i++)
        {   
            if(uart_frame_head->cmd == uart_cmd_fun[i].cmd)
            {
                uart_cmd_fun[i].call_back(buf);
            }
        }
    }
}



com_mac_send_packet_t *apply_send_packet(ENUM_COM_PORT_T port)
{
	uint8_t id;
    uart_frame_head_t *p_frame;
    com_mac_send_packet_t *p_packet;    
    
    id = mac_com_send(port, &p_packet, APPEND_QUEUE_TAIL);

    if(port == PORT_CARD_SENDER)
    {
        p_frame = (uart_frame_head_t *)p_packet->data;
        p_frame->frame_id = id;
    }
    
    return p_packet;
}
uint8_t *set_send_packet(com_mac_send_packet_t *packet, uint16_t len, uint8_t times, uint16_t need_ack)
{
    com_mac_send_packet_t *p_packet;

    if(packet == NULL) return NULL;
    
    p_packet = packet;
    
    p_packet->len = len;
    p_packet->times = times+1;
    p_packet->flag.all = need_ack;

    return p_packet->data;
}
uint8_t *set_frame_head(uint8_t *addr, uint8_t cmd, uint8_t len, uint8_t dest_addr)
{
    uart_frame_head_t *p;

    if(addr == NULL) return NULL;
    
    p = (uart_frame_head_t *)addr;

    p->head = 0xaa;
	p->len = len;
	p->dest_addr = dest_addr;
	p->src_addr = 0x1d;
    p->cmd = cmd;
    
    return &(p->payload);
}

/******************************************************************************
* Name: 	 uart_send_data 
*
* Desc:       	 数据发送测试接口
* Param(in):  	 port:        : 端口
                 buf          : 发送数据缓冲区
                 len          : 发送数据长度
                 need_ask     : 是否需要应答
                 repeat_times : 重发次数
* Param(out): 	 
* Return:     	 
* Global:     	 
* Note:       	 
* Author: 	 LiuWeiQiang
* -------------------------------------
* Log: 	 2017/06/17, Create this function by LiuWeiQiang
 ******************************************************************************/
#if 1
void uart_send_data(ENUM_COM_PORT_T port,uint8_t *buf,uint8_t len,uint8_t need_ask,uint8_t repeat_times)
{
	com_mac_send_packet_t *p_packet;
	mac_com_send(port, &p_packet,APPEND_QUEUE_TAIL);
	p_packet->len = len;
	p_packet->flag.f.need_ack = need_ask;
	p_packet->times = repeat_times+1;
	memcpy(&p_packet->data,buf,len);
}
#endif
/******************************************************************************
* Name: 	 card_fram_deal 
*
* Desc:       	 串口数据处理回调函数测试
* Param(in):  	 
* Param(out): 	 
* Return:     	 
* Global:     	 
* Note:       	 需调用uart_callback_register进行注册
* Author: 	 LiuWeiQiang
* -------------------------------------
* Log: 	 2018/11/09, Create this function by LiuWeiQiang
 ******************************************************************************/
 #if 0
typedef struct
{
    uint8_t aux_cmd;
    uint8_t uid[4];
    uint8_t adf[2];
	uint8_t key_cnt;
	uint8_t key_len;
    uint8_t key[16];
    uint8_t check_sum;
}inner_authon_t;
static void card_fram_deal(uint8_t *p_context)
{
    //命令帧:  AA 06 10 00 00 05 E5
    //应答帧:AA 06 00 10 00 85 65
	
	//命令帧: AA 1F 10 00 02 05 20 8D 0A 5C EF 10 05 05 10 2A 84 99 97 DE 79 57 5C 7F BF 40 4D 3D 1A A6 A7 47 
    //应答帧: AA 06 00 10 02 85 63 
	//处理结果帧上报：AA 06 00 10 00 05 E5 

    uint8_t ask_test[7]={0x0AA,0x06,0x00,0x10,0x00,0x05,0xE5};
    uart_frame_head_t* uart_frame_head;
    inner_authon_t *inner_authon;
    
    uart_frame_head = (uart_frame_head_t *)p_context;
    if(uart_frame_head->cmd == CARD_OPERATION)
    {
//		debug("get inner authon\r\n");
		inner_authon = (inner_authon_t *)&uart_frame_head->payload;
        if(inner_authon->aux_cmd == CARD_INNER_AUTH)
        {
            uart_send_data(PORT_CARD_SENDER,ask_test,7,1,2);//重发2次，上位机会收到3帧
        }
    }
}
#endif

void uart_mode_init(void)
{
    com_drv_init();
    memset(uart_cmd_fun,0,(sizeof(uart_cmd_fun)/sizeof(uart_cmd_fun[0])));
//    uart_callback_register(CARD_OPERATION,card_fram_deal);
}
void uart_mode_real_time(void)
{
    com_drv_real_time();
	check_uart_evt_real_time();
}

