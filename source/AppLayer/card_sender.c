/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 card_sender.c
* Desc:
*
*
* Author: 	 LiuWeiQiang
* Date: 	 2018/11/10
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/11/10, LiuWeiQiang create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "opt.h"
#include "define.h"
#include "beep_drv.h"
#include "card_sender.h"
#include "device_config.h"
#include "card_read_drv.h"
#include "card_rw_drv.h"
#include "timer_set.h"
#include "timer_drv.h"
#include "lib_iso14443Apcd.h"
#include "lib_iso14443Bpcd.h"
#include "lib_iso15693pcd.h"
#include "uart_mode.h"
#include "uart_protocol_func.h"
#include "card_encrypt.h"
#include "led_drv.h"
#include "debug.h"
/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/
extern uint8_t card_opteration_mode_attribute[2];//第一字节进入或退出卡操作模式，第二字节超时时间
static CARD_INFO_T card_data_str;
/*----------------------- Type Declarations ---------------------------------*/

typedef struct{
	uint8_t work_mode;
	uint8_t project_num[16];
	uint8_t adf[2];
	uint8_t inner_key_cnt;
}anticopy_t;

typedef struct{
	uint8_t evt_type;
	CARD_INFO_T card_info;
	uint8_t check_sum;
}card_evt_report_t;

typedef struct{
	uint8_t aux_cmd;
	uint8_t uid[4];
	uint8_t sector;
	uint8_t result;
	uint8_t result_expand;
	uint8_t check_sum;
}init_ic_card_result_report_t;

typedef struct{
	uint8_t aux_cmd;
	uint8_t result;
	uint8_t result_expand;
	uint8_t uid_len;
	uint8_t uid[8];	
	uint8_t check_sum;
}init_cpu_card_result_report_t;

typedef struct{
	uint8_t aux_cmd;
	uint8_t uid[4];
	uint8_t sector;
	uint8_t project[16];
	uint8_t check_sum;
}init_ic_card_t;

typedef struct{
	uint8_t aux_cmd;
	uint8_t key_cnt;//内部认证密钥号
	uint8_t key_type;
	uint8_t key[16];
	uint8_t prj[16];
	uint8_t uid_len;
	uint8_t uid[4];
	uint8_t check_sum;
}init_cpu_card_t;

typedef enum
{
	SENDER_AUTO_INIT_IC_SINGLE_SECTOR = 0, //自动初始化IC卡单个扇区
	SENDER_AUTO_INIT_IC_ALL_SECTOR,        //自动初始化IC卡所有扇区
	SENDER_PARAM_INIT_IC_SINGLE_SECTOR,    //传入参数初始化IC卡单个扇区
	SENDER_INIT_CPU,                       //初始化CPU卡
	SENDER_INIT_ID,                        //初始化ID卡
	SENDER_INIT_SFZ,                       //初始化身份证
}SENDER_ACTION_E;

typedef enum{
	SENDER_STATUES_IDLE = 0, //空闲
	SENDER_STATUES_AUTO,     //自动发卡
	SENDER_STATUES_READY,    //就绪态
	SENDER_STATUES_EDIT,     //卡编辑态
}SENDER_STATUES_E;

typedef enum{
	SENDER_EVT_NONE = 0,
	SENDER_EVT_GET_CARD          = SBIT_16(0),            //刷卡事件
	SENDER_EVT_AUTO_SEND_FINISH  = SBIT_16(1),            //自动发卡完成
	SENDER_EVT_CARD_MOVE         = SBIT_16(2),            //卡移离
	SENDER_EVT_ENTER_EDIT        = SBIT_16(3),            //进入编辑态
	SENDER_EVT_EXIT_EDIT         = SBIT_16(4),            //退出编辑态
}SENDER_STATUES_EVT_E;

typedef enum{
	REPORT_GET_CARD_EVT         = 0X01,
	REPORT_CARD_MOVE_EVT        = 0X02,
	REPORT_ENTRY_CARD_EDIT_EVT  = 0X03,
	REPORT_EXIT_CARD_EDIT_EVT   = 0X04,
}SENDER_EVT_REPORT_E;

typedef struct
{
	SENDER_STATUES_E state;
	SENDER_STATUES_EVT_E evt;
	timer_set_t timer;
}card_sender_state_control_t;

card_sender_state_control_t sender_state_mg;
/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/
void card_evt_report(uint8_t evt,CARD_INFO_T *card_info)
{
	uint8_t *data_buf;
	card_evt_report_t *card_evt_report;
	if(!card_info) return;
	
	data_buf = set_send_packet(apply_send_packet(PORT_CARD_SENDER),10+card_info->card_len,0,1);
	card_evt_report = (card_evt_report_t *)set_frame_head(data_buf,SENDER_EVT_REPORT,9+card_info->card_len,0);
	card_evt_report->evt_type = evt;
	memcpy(&card_evt_report->card_info,card_info,card_info->card_len+2);
	//校验和
	*((card_evt_report->card_info.card_uid)+card_info->card_len) = get_check_sum(data_buf,10+card_info->card_len);
}
void ic_card_init_result_report(uint8_t result,uint8_t result_expand,uint8_t sector,CARD_INFO_T *card_info)
{
	uint8_t *data_buf;
	init_ic_card_result_report_t *init_card_result_report;
	if(!card_info) return;
	
	data_buf = set_send_packet(apply_send_packet(PORT_CARD_SENDER),11+card_info->card_len,0,1);
	init_card_result_report = (init_ic_card_result_report_t *)set_frame_head(data_buf,INIT_CARD_RESULT_REPORT,10+card_info->card_len,0);
	init_card_result_report->aux_cmd = 0x14;
	memcpy(&init_card_result_report->uid,card_info->card_uid,card_info->card_len);
	init_card_result_report->sector = sector;
	init_card_result_report->result = result;
	init_card_result_report->result_expand = result_expand;
	//校验和
	init_card_result_report->check_sum = get_check_sum(data_buf,11+card_info->card_len);
	
}
void cpu_card_init_result_report(uint8_t result,uint8_t result_expand,CARD_INFO_T *card_info)
{
	uint8_t *data_buf;
	init_cpu_card_result_report_t *init_card_result_report;
	
	data_buf = set_send_packet(apply_send_packet(PORT_CARD_SENDER),11+card_info->card_len,0,1);
	init_card_result_report = (init_cpu_card_result_report_t *)set_frame_head(data_buf,INIT_CARD_RESULT_REPORT,10+card_info->card_len,0);
	init_card_result_report->aux_cmd = 0x15;
	init_card_result_report->result = result;
	init_card_result_report->result_expand = result_expand;
	init_card_result_report->uid_len = card_info->card_len;
	memcpy(&init_card_result_report->uid,card_info->card_uid,init_card_result_report->uid_len);
	//校验和
	init_card_result_report->check_sum = get_check_sum(data_buf,11+card_info->card_len);
}


/*----------------------- Function Implement --------------------------------*/
//成功返回1
static uint8_t sender_init_card_handle(SENDER_ACTION_E action,void *parameter)
{
	uint8_t result=0;
	switch(action)
	{
		case SENDER_AUTO_INIT_IC_SINGLE_SECTOR://自动初始化单个扇区
		{
			init_ic_card_t *init_ic_card;
			IC_SENDER_CARD_T card_info;
			
			init_ic_card = (init_ic_card_t*)parameter;
			card_info.cmd_aux = AUTO_SEND ;
			memcpy(card_info.uid,init_ic_card->uid,4);			
			result = ic_card_sender(&card_info);
		}
		break;
		
		case SENDER_PARAM_INIT_IC_SINGLE_SECTOR://传入参数初始化单个扇区
		{
			init_ic_card_t *init_ic_card;
			IC_SENDER_CARD_T card_info;
			uint8_t mastery[4]={0x78,0x77,0x88,0x69};//存取控制位默认值
			
			init_ic_card = (init_ic_card_t*)parameter;
			card_info.cmd_aux = PARAM_SEND;
			memcpy(card_info.uid,init_ic_card->uid,4);
			card_info.block = ((init_ic_card->sector - 0x80)<<2) + 3;
			card_info.admin_key_type = ISO14443A_CMD_AUTH_A;
			memset(card_info.admin_key,0xff,6);
			memcpy(card_info.authority,mastery,4);
			gen_key_ic(0,card_info.uid, (card_info.block >> 2),init_ic_card->project,card_info.key_A);//计算卡的扇区密钥keyA
	        gen_key_ic(1,card_info.uid, (card_info.block >> 2),init_ic_card->project,card_info.key_B);//计算卡的扇区密钥keyB
			result = ic_card_sender(&card_info);
		}
		break;
		
		case SENDER_AUTO_INIT_IC_ALL_SECTOR://初始化所有扇区（自动）
		{
			uint8_t sector = 0;
			uint8_t success_init_cnt = 0;
			init_ic_card_t *init_ic_card;
			IC_SENDER_CARD_T card_info;
			
			init_ic_card = (init_ic_card_t*)parameter;
			card_info.cmd_aux = AUTO_SEND;
			memcpy(card_info.uid,init_ic_card->uid,4);
			for(sector = 0;sector<16;sector++)
			{
				card_info.block = ((sector<<2)+3);
				result = ic_card_sender(&card_info);
				if(CARD_NONE_ERR == result)//初始化成功，记录初始化成功的扇区总个数
				{
					success_init_cnt++;
				}
				else//初始化失败，继续初始化下一个扇区
				{
				}
			}
			if(success_init_cnt == 16)//全部初始化成功
			{
				#if DEBUG_IC_SENDER
				debug("init all sector success!\r\n");
				#endif
			}
			else//部分或全部初始化失败
			{
				#if DEBUG_IC_SENDER
				debug("init %d sector success!\r\n",success_init_cnt);
				#endif
			}
		}
		break;
		
		
		
		case SENDER_INIT_CPU://初始化cpu卡
		{
			uint8_t retry_times = 0;
			init_cpu_card_t *init_cpu_card;
			CPU_SENDER_CARD_T card_info;
			
			init_cpu_card = (init_cpu_card_t *)parameter;
			
			card_info.adf[0] = 0x00;
			card_info.adf[1] = 0x00;
			card_info.admin_key_type = 1;
			memcpy(card_info.uid,init_cpu_card->uid,init_cpu_card->uid_len);
			for(retry_times = 0;retry_times<1;retry_times++)
			{
				result = cpu_card_sender(&card_info);
				if(result == 0)
				{
					result = 1;
					break;
				}
		    }
		}
		break;
		
		case SENDER_INIT_ID://初始化ID卡
		break;
		
		case SENDER_INIT_SFZ://初始化身份证
		break;
		
		default:break;
	}
	return result;
}

static void pc_send_card_frame_deal(uint8_t *p_context)
{
	uart_frame_head_t* recv_uart_frame;
	
	if(p_context == NULL) return ;
	recv_uart_frame = (uart_frame_head_t *)p_context;
//	debug("pc_read_attribute_frame_deal:\r\n");
//	debug_hex(p_context,recv_uart_frame->len+1);
	#if DEBUG_IC_SENDER
	debug("pc_send_card_frame_deal\r\n");
	#endif
    if(recv_uart_frame->cmd == CARD_OPERATION)
    {
		uint8_t aux_cmd;
		
		aux_cmd = recv_uart_frame->payload;
		if(aux_cmd == 0x14)//初始化IC卡
		{
			init_ic_card_t *init_ic_card;
			uint8_t result = 2;
		
		    init_ic_card = (init_ic_card_t *)&recv_uart_frame->payload;

			if(memcmp(init_ic_card->uid,card_data_str.card_uid,card_data_str.card_len) != 0)
			{
				//卡号不一致
				ic_card_init_result_report(0x02,0x02,init_ic_card->sector,&card_data_str);
				return ;
			}
			if((init_ic_card->sector >= 0x80) && (init_ic_card->sector <= 0x8f))//初始化单个扇区
			{
				result = sender_init_card_handle(SENDER_PARAM_INIT_IC_SINGLE_SECTOR,init_ic_card);
			}
			else if(init_ic_card->sector == 0xa0)//初始化所有扇区
			{
				result = sender_init_card_handle(SENDER_AUTO_INIT_IC_ALL_SECTOR,init_ic_card);
			}
			else
			{
				result = 0x02;
			}
			//结果上报
			if(result == 0x01) ic_card_init_result_report(result,0,init_ic_card->sector,&card_data_str);
			else ic_card_init_result_report(0x02,result,init_ic_card->sector,&card_data_str);;
			
		}
		else if(aux_cmd == 0x15)//初始化CPU卡
		{
			init_cpu_card_t *init_cpu_card;
			uint8_t result = 2;
			
			init_cpu_card = (init_cpu_card_t *)&recv_uart_frame->payload;
			if(!memcmp(init_cpu_card->uid,card_data_str.card_uid,init_cpu_card->uid_len))
			{
				//卡号不一致
				cpu_card_init_result_report(0x02,0x02,&card_data_str);
				return ;
			}
			result = sender_init_card_handle(SENDER_INIT_CPU,init_cpu_card);
			//结果上报
			if(result == 0x01) cpu_card_init_result_report(result,0,&card_data_str);
			else cpu_card_init_result_report(0x02,result,&card_data_str);
		}
	}
}

//检测单类型的卡，维护led的状态
void check_one_type_card(CARD_INFO_T *card_info)
{
	if(!card_info) return ;
	
	if(card_data_str.card_type == TYPE_IC || card_data_str.card_type == TYPE_CPU)
	{
		scan_ISO14443A_card(card_data_str.card_uid);
		card_data_str.card_len = 4;
		//判断场内是否有卡
		if(check_have_iso14443a_chard() == 0)
		{
			//卡被移走
			sender_state_mg.evt |= SENDER_EVT_CARD_MOVE;
			//取消卡注册回调函数
			uart_callback_unregister(CARD_OPERATION);
			//上报卡移离事件
			card_evt_report(REPORT_CARD_MOVE_EVT,&card_data_str);
		}
	}
	else if(card_data_str.card_type == TYPE_SFZ)
	{
		scan_ISO14443B_card(card_data_str.card_uid);
		card_data_str.card_len = 8;
		//判断场内是否有卡
		if(check_have_iso14443b_chard() == 0)
		{
			//卡被移走
			sender_state_mg.evt |= SENDER_EVT_CARD_MOVE;
			//取消卡注册回调函数
			uart_callback_unregister(CARD_OPERATION);
			//上报卡移离事件
			card_evt_report(REPORT_CARD_MOVE_EVT,&card_data_str);
		}
	}
	else if(card_data_str.card_type == TYPE_NFC)
	{
		scan_ISO15693_card(card_data_str.card_uid);
		card_data_str.card_len = 8;
	}
	else
	{
		sender_state_mg.state = SENDER_STATUES_IDLE;
		timer_set(&sender_state_mg.timer, 0);
	}
}
//各个状态下的任务处理
void card_sender_state_handle_real_time(void)
{	
	switch(sender_state_mg.state)
	{
		case SENDER_STATUES_IDLE:
		{
			card_read_drv_real_time();
			if(chevt_have_card_evt() == CARD_EVT_EXIT)
			{
				sender_state_mg.evt |= SENDER_EVT_CARD_MOVE;
//				debug("card remove\r\n");
			}
			memset(&card_data_str,0,sizeof(CARD_INFO_T));
			if(card_get_event(&card_data_str) == TRUE)
			{
				//置刷卡事件
				sender_state_mg.evt |= SENDER_EVT_GET_CARD;
				//上报刷卡事件
				card_evt_report(REPORT_GET_CARD_EVT,&card_data_str);
				
				#if DEBUG_IC_SENDER
				debug("get card(type = %x):",card_data_str.card_type);
				#endif
			}
		}
		break;
		
		case SENDER_STATUES_AUTO://自动初始化，此模式仅对IC卡
		{
			uint8_t init_sector = 0;
			uint8_t *anti_copy;
			uint8_t result = 0;
			init_ic_card_t init_ic_card;
			
			config_get_data(CONFIG_ANTICOPY,&anti_copy);
			
			init_sector = anti_copy[0];
			if((init_sector >= 0x80) && (init_sector <= 0x8f))//自动初始化单个扇区
			{
				init_ic_card.aux_cmd = 0x14;
				init_ic_card.sector = init_sector-0x80;
				memcpy(init_ic_card.project,anti_copy+1,16);
				memcpy(init_ic_card.uid,card_data_str.card_uid,card_data_str.card_len);
				result = sender_init_card_handle(SENDER_AUTO_INIT_IC_SINGLE_SECTOR,&init_ic_card);
			}
			else if(init_sector == 0xa0)//自动初始化所有扇区
			{
				memcpy(init_ic_card.uid,card_data_str.card_uid,card_data_str.card_len);
				result = sender_init_card_handle(SENDER_AUTO_INIT_IC_ALL_SECTOR,&init_ic_card);
			}
			//发卡成功后应立即退出
			sender_state_mg.evt |= SENDER_EVT_AUTO_SEND_FINISH;//发卡完成;
			//上报发卡结果
			if(result == 0x01)
			{
				ic_card_init_result_report(0x01,0x00,init_sector,&card_data_str);
			}
			else
			{
				ic_card_init_result_report(0x02,result,init_sector,&card_data_str);
			}
		}
		break;
		
		case SENDER_STATUES_READY:
		{
			//解析pc机的状态控制
			if(card_opteration_mode_attribute[0] == 1)//进入编辑态
			{
				sender_state_mg.evt |= SENDER_EVT_ENTER_EDIT;
				card_opteration_mode_attribute[0] = 0;
				#if DEBUG_IC_SENDER
				debug("operation time = %d\r\n",card_opteration_mode_attribute[1]);
				#endif
				//注册卡操作回调函数
				uart_callback_register(CARD_OPERATION,pc_send_card_frame_deal);
				#if DEBUG_IC_SENDER
				debug("reg CARD_OPERATION callback function\r\n");
				#endif
				//上报进入编辑态事件
				card_evt_report(REPORT_ENTRY_CARD_EDIT_EVT,&card_data_str);
				break;
			}
			//该模式下仅寻此时场内的卡
			check_one_type_card(&card_data_str);
		}
		break;
		
		case SENDER_STATUES_EDIT:
			//解析pc机的状态控制
			if(card_opteration_mode_attribute[0] == 2)//退出编辑态
			{
				sender_state_mg.evt |= SENDER_EVT_EXIT_EDIT;
				card_opteration_mode_attribute[0] = 0;
				//上报退出编辑态事件
				card_evt_report(REPORT_EXIT_CARD_EDIT_EVT,&card_data_str);
				//取消卡注册回调函数
				uart_callback_unregister(CARD_OPERATION);
				#if DEBUG_IC_SENDER
				debug("unreg CARD_OPERATION callback function\r\n");
				#endif
				break;
			}
			//发卡数据在回调函数中处理
			
			//该模式下仅寻此时场内的卡
			check_one_type_card(&card_data_str);
		break;
		
		default:
		break;
	}
}
//状态机事件处理
void card_sender_state_evt_handle_real_time(void)
{
	switch(sender_state_mg.evt)
	{
		case SENDER_EVT_GET_CARD:
		{
			uint8_t *config_data;
			anticopy_t *anticopy;
			
			sender_state_mg.evt &= ~SENDER_EVT_GET_CARD;
			//判断工作模式
			config_get_data(CONFIG_ANTICOPY,&config_data);
			anticopy = (anticopy_t*)config_data;
			#if DEBUG_IC_SENDER
			debug("work_mode = %x,card type = %x\r\n",anticopy->work_mode,card_data_str.card_type);
			#endif
			if(anticopy->work_mode == 0)//非自动发卡
			{
				sender_state_mg.state = SENDER_STATUES_READY;
			}
			else if((anticopy->work_mode != 0) && (card_data_str.card_type == TYPE_IC))//自动发卡
			{
				sender_state_mg.state = SENDER_STATUES_AUTO;
			}
			timer_disable(&sender_state_mg.timer);
			//声音提示
			beep_set_mode(BEEP_OPEN_READ_MODE);
			//LED提示
			#if (!CFG_ENABLE_DEBUG)
			led_set_mode(LED_GREEN_ON);
			#endif
		}
		break;
		
		case SENDER_EVT_AUTO_SEND_FINISH://自动发卡完成
			#if DEBUG_IC_SENDER
			debug("auto sender card finish!\r\n");
		    #endif
			sender_state_mg.evt &= ~SENDER_EVT_AUTO_SEND_FINISH;
			sender_state_mg.state = SENDER_STATUES_IDLE;
			timer_set(&sender_state_mg.timer, 0);
		break;
		
		case SENDER_EVT_CARD_MOVE://卡移离
			#if DEBUG_IC_SENDER
			debug("card remove!\r\n");
		    #endif
			sender_state_mg.evt &= ~SENDER_EVT_CARD_MOVE;
			sender_state_mg.state = SENDER_STATUES_IDLE;
			timer_set(&sender_state_mg.timer, 0);
		    //取消卡注册回调函数
			uart_callback_unregister(CARD_OPERATION);
		    #if DEBUG_IC_SENDER
			debug("unreg CARD_OPERATION callback function\r\n");
		    #endif
			#if (!CFG_ENABLE_DEBUG)
			led_set_mode(LED_RED_ON);
			#endif
		break;
		
		case SENDER_EVT_ENTER_EDIT://进入卡编辑态
			#if DEBUG_IC_SENDER
			debug("enter edit...\r\n");
		    #endif
			sender_state_mg.evt &= ~SENDER_EVT_ENTER_EDIT;
			sender_state_mg.state = SENDER_STATUES_EDIT;
			timer_set(&sender_state_mg.timer, card_opteration_mode_attribute[1]*1000);
		break;
		
		case SENDER_EVT_EXIT_EDIT://退出卡编辑态,进入就绪态
			#if DEBUG_IC_SENDER
			debug("exit edit...\r\n");
			#endif
			sender_state_mg.evt &= ~SENDER_EVT_EXIT_EDIT;
			sender_state_mg.state = SENDER_STATUES_READY;
			timer_disable(&sender_state_mg.timer);
		break;
		
		default:
			sender_state_mg.evt = SENDER_EVT_NONE;
		break;
	}
}



//状态超时的处理
static void card_sender_state_real_time(void)
{
	if(timer_expired(&sender_state_mg.timer))
	{
		switch(sender_state_mg.state)
		{
			case SENDER_STATUES_IDLE:
				
			break;
			
			case SENDER_STATUES_AUTO://自动发卡超时
				#if DEBUG_IC_SENDER
				debug("sta change:SENDER_STATUES_AUTO -> SENDER_STATUES_IDLE.\r\n");
			    #endif
			    sender_state_mg.state = SENDER_STATUES_IDLE;
			break;
			
			case SENDER_STATUES_READY://就绪态超时
				#if DEBUG_IC_SENDER
				debug("sta change:SENDER_STATUES_READY -> SENDER_STATUES_IDLE.\r\n");
			    #endif
			    sender_state_mg.state = SENDER_STATUES_IDLE;
			break;
			
			case SENDER_STATUES_EDIT://编辑态超时
				#if DEBUG_IC_SENDER
				debug("sta change:SENDER_STATUES_EDIT -> SENDER_STATUES_READY.\r\n");
			    #endif
			    sender_state_mg.state = SENDER_STATUES_READY;
			    sender_state_mg.evt |= SENDER_EVT_EXIT_EDIT;
			break;
			
			default://未知状态
				timer_set(&sender_state_mg.timer, 0);
			    sender_state_mg.state = SENDER_STATUES_IDLE;
			break;
		}
	}
}

void card_sender_init(void)
{
	memset(&sender_state_mg,0,sizeof(card_sender_state_control_t));
}
void card_sender_real_time(void)
{
	card_sender_state_evt_handle_real_time();
	card_sender_state_handle_real_time();
	card_sender_state_real_time();
}

/*---------------------------------------------------------------------------*/

