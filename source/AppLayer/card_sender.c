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
extern uint8_t card_opteration_mode_attribute[2];//��һ�ֽڽ�����˳�������ģʽ���ڶ��ֽڳ�ʱʱ��
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
	uint8_t key_cnt;//�ڲ���֤��Կ��
	uint8_t key_type;
	uint8_t key[16];
	uint8_t prj[16];
	uint8_t uid_len;
	uint8_t uid[4];
	uint8_t check_sum;
}init_cpu_card_t;

typedef enum
{
	SENDER_AUTO_INIT_IC_SINGLE_SECTOR = 0, //�Զ���ʼ��IC����������
	SENDER_AUTO_INIT_IC_ALL_SECTOR,        //�Զ���ʼ��IC����������
	SENDER_PARAM_INIT_IC_SINGLE_SECTOR,    //���������ʼ��IC����������
	SENDER_INIT_CPU,                       //��ʼ��CPU��
	SENDER_INIT_ID,                        //��ʼ��ID��
	SENDER_INIT_SFZ,                       //��ʼ�����֤
}SENDER_ACTION_E;

typedef enum{
	SENDER_STATUES_IDLE = 0, //����
	SENDER_STATUES_AUTO,     //�Զ�����
	SENDER_STATUES_READY,    //����̬
	SENDER_STATUES_EDIT,     //���༭̬
}SENDER_STATUES_E;

typedef enum{
	SENDER_EVT_NONE = 0,
	SENDER_EVT_GET_CARD          = SBIT_16(0),            //ˢ���¼�
	SENDER_EVT_AUTO_SEND_FINISH  = SBIT_16(1),            //�Զ��������
	SENDER_EVT_CARD_MOVE         = SBIT_16(2),            //������
	SENDER_EVT_ENTER_EDIT        = SBIT_16(3),            //����༭̬
	SENDER_EVT_EXIT_EDIT         = SBIT_16(4),            //�˳��༭̬
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
	//У���
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
	//У���
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
	//У���
	init_card_result_report->check_sum = get_check_sum(data_buf,11+card_info->card_len);
}


/*----------------------- Function Implement --------------------------------*/
//�ɹ�����1
static uint8_t sender_init_card_handle(SENDER_ACTION_E action,void *parameter)
{
	uint8_t result=0;
	switch(action)
	{
		case SENDER_AUTO_INIT_IC_SINGLE_SECTOR://�Զ���ʼ����������
		{
			init_ic_card_t *init_ic_card;
			IC_SENDER_CARD_T card_info;
			
			init_ic_card = (init_ic_card_t*)parameter;
			card_info.cmd_aux = AUTO_SEND ;
			memcpy(card_info.uid,init_ic_card->uid,4);			
			result = ic_card_sender(&card_info);
		}
		break;
		
		case SENDER_PARAM_INIT_IC_SINGLE_SECTOR://���������ʼ����������
		{
			init_ic_card_t *init_ic_card;
			IC_SENDER_CARD_T card_info;
			uint8_t mastery[4]={0x78,0x77,0x88,0x69};//��ȡ����λĬ��ֵ
			
			init_ic_card = (init_ic_card_t*)parameter;
			card_info.cmd_aux = PARAM_SEND;
			memcpy(card_info.uid,init_ic_card->uid,4);
			card_info.block = ((init_ic_card->sector - 0x80)<<2) + 3;
			card_info.admin_key_type = ISO14443A_CMD_AUTH_A;
			memset(card_info.admin_key,0xff,6);
			memcpy(card_info.authority,mastery,4);
			gen_key_ic(0,card_info.uid, (card_info.block >> 2),init_ic_card->project,card_info.key_A);//���㿨��������ԿkeyA
	        gen_key_ic(1,card_info.uid, (card_info.block >> 2),init_ic_card->project,card_info.key_B);//���㿨��������ԿkeyB
			result = ic_card_sender(&card_info);
		}
		break;
		
		case SENDER_AUTO_INIT_IC_ALL_SECTOR://��ʼ�������������Զ���
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
				if(CARD_NONE_ERR == result)//��ʼ���ɹ�����¼��ʼ���ɹ��������ܸ���
				{
					success_init_cnt++;
				}
				else//��ʼ��ʧ�ܣ�������ʼ����һ������
				{
				}
			}
			if(success_init_cnt == 16)//ȫ����ʼ���ɹ�
			{
				#if DEBUG_IC_SENDER
				debug("init all sector success!\r\n");
				#endif
			}
			else//���ֻ�ȫ����ʼ��ʧ��
			{
				#if DEBUG_IC_SENDER
				debug("init %d sector success!\r\n",success_init_cnt);
				#endif
			}
		}
		break;
		
		
		
		case SENDER_INIT_CPU://��ʼ��cpu��
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
		
		case SENDER_INIT_ID://��ʼ��ID��
		break;
		
		case SENDER_INIT_SFZ://��ʼ�����֤
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
		if(aux_cmd == 0x14)//��ʼ��IC��
		{
			init_ic_card_t *init_ic_card;
			uint8_t result = 2;
		
		    init_ic_card = (init_ic_card_t *)&recv_uart_frame->payload;

			if(memcmp(init_ic_card->uid,card_data_str.card_uid,card_data_str.card_len) != 0)
			{
				//���Ų�һ��
				ic_card_init_result_report(0x02,0x02,init_ic_card->sector,&card_data_str);
				return ;
			}
			if((init_ic_card->sector >= 0x80) && (init_ic_card->sector <= 0x8f))//��ʼ����������
			{
				result = sender_init_card_handle(SENDER_PARAM_INIT_IC_SINGLE_SECTOR,init_ic_card);
			}
			else if(init_ic_card->sector == 0xa0)//��ʼ����������
			{
				result = sender_init_card_handle(SENDER_AUTO_INIT_IC_ALL_SECTOR,init_ic_card);
			}
			else
			{
				result = 0x02;
			}
			//����ϱ�
			if(result == 0x01) ic_card_init_result_report(result,0,init_ic_card->sector,&card_data_str);
			else ic_card_init_result_report(0x02,result,init_ic_card->sector,&card_data_str);;
			
		}
		else if(aux_cmd == 0x15)//��ʼ��CPU��
		{
			init_cpu_card_t *init_cpu_card;
			uint8_t result = 2;
			
			init_cpu_card = (init_cpu_card_t *)&recv_uart_frame->payload;
			if(!memcmp(init_cpu_card->uid,card_data_str.card_uid,init_cpu_card->uid_len))
			{
				//���Ų�һ��
				cpu_card_init_result_report(0x02,0x02,&card_data_str);
				return ;
			}
			result = sender_init_card_handle(SENDER_INIT_CPU,init_cpu_card);
			//����ϱ�
			if(result == 0x01) cpu_card_init_result_report(result,0,&card_data_str);
			else cpu_card_init_result_report(0x02,result,&card_data_str);
		}
	}
}

//��ⵥ���͵Ŀ���ά��led��״̬
void check_one_type_card(CARD_INFO_T *card_info)
{
	if(!card_info) return ;
	
	if(card_data_str.card_type == TYPE_IC || card_data_str.card_type == TYPE_CPU)
	{
		scan_ISO14443A_card(card_data_str.card_uid);
		card_data_str.card_len = 4;
		//�жϳ����Ƿ��п�
		if(check_have_iso14443a_chard() == 0)
		{
			//��������
			sender_state_mg.evt |= SENDER_EVT_CARD_MOVE;
			//ȡ����ע��ص�����
			uart_callback_unregister(CARD_OPERATION);
			//�ϱ��������¼�
			card_evt_report(REPORT_CARD_MOVE_EVT,&card_data_str);
		}
	}
	else if(card_data_str.card_type == TYPE_SFZ)
	{
		scan_ISO14443B_card(card_data_str.card_uid);
		card_data_str.card_len = 8;
		//�жϳ����Ƿ��п�
		if(check_have_iso14443b_chard() == 0)
		{
			//��������
			sender_state_mg.evt |= SENDER_EVT_CARD_MOVE;
			//ȡ����ע��ص�����
			uart_callback_unregister(CARD_OPERATION);
			//�ϱ��������¼�
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
//����״̬�µ�������
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
				//��ˢ���¼�
				sender_state_mg.evt |= SENDER_EVT_GET_CARD;
				//�ϱ�ˢ���¼�
				card_evt_report(REPORT_GET_CARD_EVT,&card_data_str);
				
				#if DEBUG_IC_SENDER
				debug("get card(type = %x):",card_data_str.card_type);
				#endif
			}
		}
		break;
		
		case SENDER_STATUES_AUTO://�Զ���ʼ������ģʽ����IC��
		{
			uint8_t init_sector = 0;
			uint8_t *anti_copy;
			uint8_t result = 0;
			init_ic_card_t init_ic_card;
			
			config_get_data(CONFIG_ANTICOPY,&anti_copy);
			
			init_sector = anti_copy[0];
			if((init_sector >= 0x80) && (init_sector <= 0x8f))//�Զ���ʼ����������
			{
				init_ic_card.aux_cmd = 0x14;
				init_ic_card.sector = init_sector-0x80;
				memcpy(init_ic_card.project,anti_copy+1,16);
				memcpy(init_ic_card.uid,card_data_str.card_uid,card_data_str.card_len);
				result = sender_init_card_handle(SENDER_AUTO_INIT_IC_SINGLE_SECTOR,&init_ic_card);
			}
			else if(init_sector == 0xa0)//�Զ���ʼ����������
			{
				memcpy(init_ic_card.uid,card_data_str.card_uid,card_data_str.card_len);
				result = sender_init_card_handle(SENDER_AUTO_INIT_IC_ALL_SECTOR,&init_ic_card);
			}
			//�����ɹ���Ӧ�����˳�
			sender_state_mg.evt |= SENDER_EVT_AUTO_SEND_FINISH;//�������;
			//�ϱ��������
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
			//����pc����״̬����
			if(card_opteration_mode_attribute[0] == 1)//����༭̬
			{
				sender_state_mg.evt |= SENDER_EVT_ENTER_EDIT;
				card_opteration_mode_attribute[0] = 0;
				#if DEBUG_IC_SENDER
				debug("operation time = %d\r\n",card_opteration_mode_attribute[1]);
				#endif
				//ע�Ῠ�����ص�����
				uart_callback_register(CARD_OPERATION,pc_send_card_frame_deal);
				#if DEBUG_IC_SENDER
				debug("reg CARD_OPERATION callback function\r\n");
				#endif
				//�ϱ�����༭̬�¼�
				card_evt_report(REPORT_ENTRY_CARD_EDIT_EVT,&card_data_str);
				break;
			}
			//��ģʽ�½�Ѱ��ʱ���ڵĿ�
			check_one_type_card(&card_data_str);
		}
		break;
		
		case SENDER_STATUES_EDIT:
			//����pc����״̬����
			if(card_opteration_mode_attribute[0] == 2)//�˳��༭̬
			{
				sender_state_mg.evt |= SENDER_EVT_EXIT_EDIT;
				card_opteration_mode_attribute[0] = 0;
				//�ϱ��˳��༭̬�¼�
				card_evt_report(REPORT_EXIT_CARD_EDIT_EVT,&card_data_str);
				//ȡ����ע��ص�����
				uart_callback_unregister(CARD_OPERATION);
				#if DEBUG_IC_SENDER
				debug("unreg CARD_OPERATION callback function\r\n");
				#endif
				break;
			}
			//���������ڻص������д���
			
			//��ģʽ�½�Ѱ��ʱ���ڵĿ�
			check_one_type_card(&card_data_str);
		break;
		
		default:
		break;
	}
}
//״̬���¼�����
void card_sender_state_evt_handle_real_time(void)
{
	switch(sender_state_mg.evt)
	{
		case SENDER_EVT_GET_CARD:
		{
			uint8_t *config_data;
			anticopy_t *anticopy;
			
			sender_state_mg.evt &= ~SENDER_EVT_GET_CARD;
			//�жϹ���ģʽ
			config_get_data(CONFIG_ANTICOPY,&config_data);
			anticopy = (anticopy_t*)config_data;
			#if DEBUG_IC_SENDER
			debug("work_mode = %x,card type = %x\r\n",anticopy->work_mode,card_data_str.card_type);
			#endif
			if(anticopy->work_mode == 0)//���Զ�����
			{
				sender_state_mg.state = SENDER_STATUES_READY;
			}
			else if((anticopy->work_mode != 0) && (card_data_str.card_type == TYPE_IC))//�Զ�����
			{
				sender_state_mg.state = SENDER_STATUES_AUTO;
			}
			timer_disable(&sender_state_mg.timer);
			//������ʾ
			beep_set_mode(BEEP_OPEN_READ_MODE);
			//LED��ʾ
			#if (!CFG_ENABLE_DEBUG)
			led_set_mode(LED_GREEN_ON);
			#endif
		}
		break;
		
		case SENDER_EVT_AUTO_SEND_FINISH://�Զ��������
			#if DEBUG_IC_SENDER
			debug("auto sender card finish!\r\n");
		    #endif
			sender_state_mg.evt &= ~SENDER_EVT_AUTO_SEND_FINISH;
			sender_state_mg.state = SENDER_STATUES_IDLE;
			timer_set(&sender_state_mg.timer, 0);
		break;
		
		case SENDER_EVT_CARD_MOVE://������
			#if DEBUG_IC_SENDER
			debug("card remove!\r\n");
		    #endif
			sender_state_mg.evt &= ~SENDER_EVT_CARD_MOVE;
			sender_state_mg.state = SENDER_STATUES_IDLE;
			timer_set(&sender_state_mg.timer, 0);
		    //ȡ����ע��ص�����
			uart_callback_unregister(CARD_OPERATION);
		    #if DEBUG_IC_SENDER
			debug("unreg CARD_OPERATION callback function\r\n");
		    #endif
			#if (!CFG_ENABLE_DEBUG)
			led_set_mode(LED_RED_ON);
			#endif
		break;
		
		case SENDER_EVT_ENTER_EDIT://���뿨�༭̬
			#if DEBUG_IC_SENDER
			debug("enter edit...\r\n");
		    #endif
			sender_state_mg.evt &= ~SENDER_EVT_ENTER_EDIT;
			sender_state_mg.state = SENDER_STATUES_EDIT;
			timer_set(&sender_state_mg.timer, card_opteration_mode_attribute[1]*1000);
		break;
		
		case SENDER_EVT_EXIT_EDIT://�˳����༭̬,�������̬
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



//״̬��ʱ�Ĵ���
static void card_sender_state_real_time(void)
{
	if(timer_expired(&sender_state_mg.timer))
	{
		switch(sender_state_mg.state)
		{
			case SENDER_STATUES_IDLE:
				
			break;
			
			case SENDER_STATUES_AUTO://�Զ�������ʱ
				#if DEBUG_IC_SENDER
				debug("sta change:SENDER_STATUES_AUTO -> SENDER_STATUES_IDLE.\r\n");
			    #endif
			    sender_state_mg.state = SENDER_STATUES_IDLE;
			break;
			
			case SENDER_STATUES_READY://����̬��ʱ
				#if DEBUG_IC_SENDER
				debug("sta change:SENDER_STATUES_READY -> SENDER_STATUES_IDLE.\r\n");
			    #endif
			    sender_state_mg.state = SENDER_STATUES_IDLE;
			break;
			
			case SENDER_STATUES_EDIT://�༭̬��ʱ
				#if DEBUG_IC_SENDER
				debug("sta change:SENDER_STATUES_EDIT -> SENDER_STATUES_READY.\r\n");
			    #endif
			    sender_state_mg.state = SENDER_STATUES_READY;
			    sender_state_mg.evt |= SENDER_EVT_EXIT_EDIT;
			break;
			
			default://δ֪״̬
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

