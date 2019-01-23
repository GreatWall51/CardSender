/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 parameter_mg.c
* Desc:
*
*
* Author: 	 LiuWeiQiang
* Date: 	 2018/11/09
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/11/09, LiuWeiQiang create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "parameter_mg.h"
#include "uart_mode.h"
#include "uart_protocol_func.h"
#include "device_config_flag.h"
#include "device_config.h"
#include <string.h>
#include "beep_drv.h"
#include "led_drv.h"
#include "debug.h"


uint8_t protocol_version = 0x01;
uint8_t software_version[2]={0x02,0x00};
uint8_t led_buzzer_attribute[2] = {0}; 
uint8_t card_opteration_mode_attribute[2] = {0,60};
uint8_t card_sta[10] = {10};
/*------------------- Global Definitions and Declarations -------------------*/
typedef enum
{
	PROTOCOL_VERSION      = 0X10,  //Э��汾
	LED_BUZZER_ATTRIBUTE  = 0x21,  //LED����������
	CARD_OPTERATION_MODE_ATTRIBUTE = 0X23,//������ģʽ
	SOFTWARE_VERSION_ATTRIBUTE = 0xb4,    //����汾
	SWIPE_CARD_AREA_STA = 0XD5,           //ˢ����״̬
	
	WORK_MODE_ATTRIBUTE   = 0xe2,  //����ģʽ�����̺�ADF��Կ������
}OBJECT_ATTRIBUTE_E;
typedef struct
{
	uint8_t object_cnt;//�������
	uint8_t attribute;
}read_obj_attribute_t;
typedef struct
{
//	uint8_t result;
	uint8_t object_cnt;//�������
	uint8_t attribute;
}ask_read_obj_attribute_t;
typedef struct
{
	uint8_t object_cnt;//�������
	uint8_t payload;
}write_obj_attribute_t;
typedef struct
{
	uint8_t result;
	uint8_t result_expand;
	uint8_t check_sum;
}ask_write_obj_attribute_t;
/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/
//��ȡ�������Եĳ��ȣ��������Զ������
uint8_t get_object_attribute_len(uint8_t attribute)
{
	uint8_t attribute_len = 0;
	switch(attribute)
	{
		case PROTOCOL_VERSION://Э��汾
			attribute_len = 1;
		break;
		
		case SOFTWARE_VERSION_ATTRIBUTE://����汾
			attribute_len = 2;
		break;
		
		case LED_BUZZER_ATTRIBUTE://LED����������
			attribute_len = 2;
		break;
		
		case WORK_MODE_ATTRIBUTE://����ģʽ�����̺�ADF��Կ������
			attribute_len = 20;
		break;
		
		case CARD_OPTERATION_MODE_ATTRIBUTE://������ģʽ
			attribute_len = 2;
		break;
		
		case SWIPE_CARD_AREA_STA://ˢ����״̬
			attribute_len = 10;
		break;
		
		
		default:
			attribute_len = 0;
		break;
			
	}
	return attribute_len;
}
//��flash�еĲ��������
uint8_t get_attribute_index(uint8_t attribute)
{
	uint8_t index=0;
	switch(attribute)
	{
		case WORK_MODE_ATTRIBUTE:
			index = CONFIG_ANTICOPY;
		break;
		
		default:
			index = 0;
		break;
			
	}
	return index;
}

/*----------------------- Function Implement --------------------------------*/
static void pc_read_attribute_frame_deal(uint8_t *p_context)
{
	uart_frame_head_t* recv_uart_frame;
	
	if(p_context == NULL) return ;
	recv_uart_frame = (uart_frame_head_t *)p_context;
//	debug("pc_read_attribute_frame_deal:\r\n");
//	debug_hex(p_context,recv_uart_frame->len+1);
	
    if(recv_uart_frame->cmd == READ_OBJECT_ATTRIBUTE)
    {
		#if 1
		uint8_t index;
		uint8_t *data_buf;
		uint8_t offset;    
		read_obj_attribute_t *read_obj_attribute; //����������֡
		ask_read_obj_attribute_t *ask_obj_attribute;  //������Ӧ��֡
		uint8_t attribute;          //����ֵ
		uint8_t *attribute_data;    //��������
		uint8_t attribute_data_len; //�������ݳ���
		uint8_t attribute_total_len = 0;//��������Ե��ܳ���

		read_obj_attribute = (read_obj_attribute_t *)&recv_uart_frame->payload;
//		debug("attribute(cnt = %d):\r\n",read_obj_attribute->object_cnt);
//		debug_hex(&read_obj_attribute->attribute,read_obj_attribute->object_cnt);
		//����Ӧ������ܴ�С
		attribute_total_len = 0;
		attribute_total_len += read_obj_attribute->object_cnt;//���Ը�����ÿ������ռһ�ֽ�
		for(index = 0;index <read_obj_attribute->object_cnt;index++)//��������
		{
			uint8_t attribute_len = 0;
			attribute_len = get_object_attribute_len(*((&read_obj_attribute->attribute)+index));
			attribute_total_len+= attribute_len;
//			ask_read_object_attribute(PORT_CARD_SENDER,*((&read_obj_attribute->attribute)+index));
		}
		data_buf = set_send_packet(apply_send_packet(PORT_CARD_SENDER),attribute_total_len+8,0,1);
		ask_obj_attribute = (ask_read_obj_attribute_t *)set_frame_head(data_buf,recv_uart_frame->cmd | 0x80,attribute_total_len+7,recv_uart_frame->src_addr);
//		ask_obj_attribute->result = 0x01;
		ask_obj_attribute->object_cnt = read_obj_attribute->object_cnt;
		offset = 0;
		for(index = 0;index <read_obj_attribute->object_cnt;index++)//��������
		{
			attribute = *((&read_obj_attribute->attribute)+index);//����ֵ
//			debug("attribute = %x\r\n",attribute);
			//���Բ�����...TODO
			
			
			attribute_data_len = get_object_attribute_len(attribute);//�������ݳ���
			
//			debug("attribute_data_len = %x\r\n",attribute_data_len);
			//���¸�ֵӦ������
			*((&ask_obj_attribute->attribute)+offset) = attribute;//��ֵ����ֵ
			offset++;
			//��ֵ��������
			if(attribute == WORK_MODE_ATTRIBUTE)//�洢��flash�е�����
			{
			   config_get_data((CONFIG_PARAMETER_E)get_attribute_index(attribute),&attribute_data);
			   memcpy((&ask_obj_attribute->attribute)+offset,attribute_data,attribute_data_len);
			}
			else//�洢��ram�е�����
			{
				if(attribute == PROTOCOL_VERSION)
				{
					memcpy((&ask_obj_attribute->attribute)+offset,&protocol_version,attribute_data_len);
				}
				else if(attribute == SOFTWARE_VERSION_ATTRIBUTE)
				{
					memcpy((&ask_obj_attribute->attribute)+offset,software_version,attribute_data_len);
				}
				else if(attribute == LED_BUZZER_ATTRIBUTE)
				{
					memcpy((&ask_obj_attribute->attribute)+offset,led_buzzer_attribute,attribute_data_len);
				}
				else if(attribute == CARD_OPTERATION_MODE_ATTRIBUTE)
				{
					memcpy((&ask_obj_attribute->attribute)+offset,card_opteration_mode_attribute,attribute_data_len);
				}
				else if(attribute == SWIPE_CARD_AREA_STA)
				{
					memcpy((&ask_obj_attribute->attribute)+offset,card_sta,attribute_data_len);
				}
			}
			offset += attribute_data_len;
		}
		//У���
		*((&ask_obj_attribute->attribute)+offset) = get_check_sum(data_buf,attribute_total_len+8);
//		debug("ask data:");
//		debug_hex(data_buf,attribute_total_len+8);
//		uart_send_data(PORT_CARD_SENDER,data_buf,5,1,1);
		#endif
	}
}

static void pc_write_attribute_frame_deal(uint8_t *p_context)
{
	uart_frame_head_t* recv_uart_frame;
	
	if(p_context == NULL) return ;
	recv_uart_frame = (uart_frame_head_t *)p_context;
//	debug("pc_write_attribute_frame_deal:\r\n");
//	debug_hex(p_context,recv_uart_frame->len+1);
	
    if(recv_uart_frame->cmd == WRITE_OBJECT_ATTRIBUTE)
    {
		uint8_t index;
		uint8_t offect;
		uint8_t attribute;
		uint8_t attribute_data_len = 0;
		uint8_t *data_buf;
		write_obj_attribute_t *write_obj_attribute; //����������֡
		ask_write_obj_attribute_t *ask_write_obj_attribute;
		
		write_obj_attribute = (write_obj_attribute_t *)&recv_uart_frame->payload;
		offect = 0;
		for(index = 0;index <write_obj_attribute->object_cnt;index++)
		{
			attribute = *((&write_obj_attribute->payload)+offect);
			//���Բ�����...TODO
			
			
			attribute_data_len = get_object_attribute_len(attribute);
			if(attribute == WORK_MODE_ATTRIBUTE)//�洢��flash�е�����
			{
				config_set_data(CONFIG_ANTICOPY,(&write_obj_attribute->payload)+offect+1);
			}
			else
			{
				if(attribute == PROTOCOL_VERSION)
				{
					memcpy(&protocol_version,(&write_obj_attribute->payload)+offect+1,attribute_data_len);
				}
				else if(attribute == SOFTWARE_VERSION_ATTRIBUTE)
				{
					memcpy(software_version,(&write_obj_attribute->payload)+offect+1,attribute_data_len);
				}
				else if(attribute == LED_BUZZER_ATTRIBUTE)
				{
					memcpy(led_buzzer_attribute,(&write_obj_attribute->payload)+offect+1,attribute_data_len);
					//������ʾ
					if(led_buzzer_attribute[0] == 0x01)
					{
				        beep_set_mode(BEEP_OPEN_READ_MODE);
					}
					else if(led_buzzer_attribute[0] == 0x02)
					{
						beep_set_mode(BEEP_CARD_1_MODE);
					}
					else if(led_buzzer_attribute[0] == 0x03)
					{
						beep_set_mode(BEEP_CARD_2_MODE);
					}
					
					//LED��ʾ
					#if (!CFG_ENABLE_DEBUG)
					led_set_mode((LED_MODE_E)led_buzzer_attribute[1]);
					#endif
				}
				else if(attribute == CARD_OPTERATION_MODE_ATTRIBUTE)
				{
					memcpy(card_opteration_mode_attribute,(&write_obj_attribute->payload)+offect+1,attribute_data_len);
				}
				else if(attribute == SWIPE_CARD_AREA_STA)
				{
					memcpy(card_sta,(&write_obj_attribute->payload)+offect+1,attribute_data_len);
				}
				else
				{
					
				}
				
		    }
			offect += attribute_data_len;
		}
		//�ɹ�Ӧ��
		data_buf = set_send_packet(apply_send_packet(PORT_CARD_SENDER),8,0,1);
		ask_write_obj_attribute = (ask_write_obj_attribute_t *)set_frame_head(data_buf,recv_uart_frame->cmd | 0x80,7,recv_uart_frame->src_addr);
		ask_write_obj_attribute->result = 0x01;
		*(&ask_write_obj_attribute->result_expand) = get_check_sum(data_buf,8);
		//ʧ��Ӧ��
//		data_buf = set_send_packet(apply_send_packet(PORT_CARD_SENDER),9,0,1);
//		ask_write_obj_attribute = (ask_write_obj_attribute_t *)set_frame_head(data_buf,recv_uart_frame->cmd | 0x80,8,recv_uart_frame->src_addr);
//		ask_write_obj_attribute->result = 0x01;
//		ask_write_obj_attribute->result_expand = 0x01;
		*(&ask_write_obj_attribute->check_sum) = get_check_sum(data_buf,9);
	}
}

void parameter_mg_init(void)
{
	uart_callback_register(READ_OBJECT_ATTRIBUTE,pc_read_attribute_frame_deal);
	uart_callback_register(WRITE_OBJECT_ATTRIBUTE,pc_write_attribute_frame_deal);
}
/*---------------------------------------------------------------------------*/

