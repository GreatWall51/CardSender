/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 device_config.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/25
* Notes:	�����������ûд��flash��������д�������Ƿ�������ñ���Ŀ�ĳ���
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/25, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include <string.h>
#include "define.h"
#include "device_config.h"
#include "device_config_flag.h"
#include "reg_table.h"
#include "opt.h"
#include "debug.h"

/*------------------- Global Definitions and Declarations -------------------*/
static config_sys_param_t sys_param;
/*
note:
(1)�����ÿһ���ڵ��Ӧflash�е�һ��item�������ӳ�Աʱ��ֻ�������һ������ĺ���ӣ�������ƻ�flash�еĶ�Ӧ��ϵ
(2)����config_list������CONFIG_PARAMETER_Eһһ��Ӧ
(3)���ӳ�Ա�����������֮ǰ�����ݣ�ɾ����Ա�����Ὣ�������ݻָ�Ĭ��ֵ
(4)�����ĳ�Ա���ں���sys_param_new_member_init�н��г�ʼ��
(5)��device_param_default_init���ûָ�����Ĭ��ֵ
*/
static config_item_info_t config_list[CONFIG_MAX] =
{
	{ "modify_flag",&sys_param.modify_flag, DATA_TYPE_BCD, sizeof ( sys_param.modify_flag ) },
	{ "anty_copy",sys_param.anticopy, DATA_TYPE_HEX, sizeof ( sys_param.anticopy )/sizeof ( sys_param.anticopy[0]) },
};

/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/
typedef struct
{
	uint16_t item_id;    //������Ŀ���
	uint8_t data[SYSTEM_PARAM_ITEM_MAX_SIZE];
} flash_item_info_t;

typedef struct
{
	uint16_t len;//buf�ĳ���(��λ:�ֽ�)
	uint8_t* buf;
} bit_index_info_t;
bit_index_info_t bit_index_info;
static uint8_t need_sync_to_flash = 0;
/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
void set_param_sync_to_flash ( void )
{
	need_sync_to_flash = 1;
}
/******************************************************************************
* Name: 	 get_index_bit_valure
*
* Desc:       	 ��ȡ����λ��ֵ
* Param(in):  	 index:����λ
                 info: ���Խṹ����Ϣ
* Param(out):
* Return:     	 ����λ��ֵ
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/28, Create this function by Liuwq
 ******************************************************************************/
static uint8_t get_index_bit_valure ( uint32_t index,bit_index_info_t* info )
{
	uint8_t bit_val=2;
	if ( ( info->buf == NULL ) || ( info->len == 0 ) || ( index > ( ( info->len<<3 )-1 ) ) )
	{
		return bit_val;
	}

	if ( info->buf[index >> 3] & ( 0x80 >> ( index & 0x07 ) ) )
	{
		bit_val = 1;
	}
	else
	{
		bit_val = 0;
	}
	return bit_val;
}
static uint8_t get_config_change_evt ( CONFIG_PARAMETER_E index )
{
	uint8_t evt =  0;
	evt = get_index_bit_valure ( index,&bit_index_info );
	return evt;
}
/******************************************************************************
* Name: 	 set_index_bit_valure
*
* Desc:       	 ��������λ��ֵ
* Param(in):  	 index:����λ
                 val:  ����λ��ֵ
                 info: ���Խṹ����Ϣ
* Param(out):
* Return:     	 ���õ�ֵ
* Global:
* Note:       	 ��indexΪ((info->len<<3)-1))����ʾ������������������ֵ
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/28, Create this function by Liuwq
 ******************************************************************************/
static uint8_t set_index_bit_valure ( uint32_t index,uint8_t val,bit_index_info_t* info )
{
	if ( info->buf == NULL || info->len == 0 || ( index > ( ( info->len<<3 )-1 ) ) )
	{
		return 2;
	}
	if ( index == ( ( info->len<<3 )-1 ) )
	{
		if ( val )
		{
			memset ( info->buf,0xff,info->len );
		}
		else
		{
			memset ( info->buf,0,info->len );
		}
	}
	else
	{
		if ( val )
		{
			info->buf[index >> 3] |=  ( 0x80 >> ( index & 0x07 ) );
		}
		else
		{
			info->buf[index >> 3] &= ~ ( 0x80 >> ( index & 0x07 ) );
		}

	}
	return val;
}
static uint8_t set_config_change_evt ( CONFIG_PARAMETER_E index,uint8_t val )
{
	uint8_t evt = 2;
	evt = set_index_bit_valure ( index,val,&bit_index_info );
	return evt;
}

/******************************************************************************
* Name: 	 update_item
*
* Desc:       	 ���±���ָ������Ŀ��flash
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
static uint8_t update_item ( uint16_t index,TABLE_E table,uint8_t* data )
{
	uint8_t len = 0;
	flash_item_info_t flash_item_info;
	table_item_match_t item_match;

	if ( ( table >= TABLE_MAX ) || ( index >= CONFIG_MAX ) || ( data == NULL ) )
	{
		return len;
	}
	flash_item_info.item_id = ( uint16_t ) ( index>>8 ) + ( uint16_t ) ( index<<8 );;
	memcpy ( flash_item_info.data,data,MIN ( config_list[index].data_len,SYSTEM_PARAM_ITEM_MAX_SIZE ) );
	item_match.offset = 0;
	item_match.match_len = sizeof ( flash_item_info.item_id );
	item_match.match_data[0] = ( uint8_t ) ( flash_item_info.item_id&0xff );
	item_match.match_data[1] = ( uint8_t ) ( ( flash_item_info.item_id>>8 ) &0xff );
	len = table_write_item ( table,&item_match,&flash_item_info,config_list[index].data_len+item_match.match_len );
	return len;
}
/******************************************************************************
* Name: 	 sys_param_new_member_init
*
* Desc:       	 ��ʼ��sys_param�����ӵĳ�Ա������ram
* Param(in):
* Param(out):
* Return:
* Global:
* Note:       	 �ú����ڳ�Ա�����޸�ʱ��ÿ�ζ������±���
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
void sys_param_new_member_init ( void )
{
	uint8_t antycopy_param[20] = {0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x10,0x05,0x05};
	//ֻ�������ʼ�������ӵĳ�Ա��������ʼ����������Ա��������Ч�ġ�
	sys_param.work_mode = 1;
	memcpy ( sys_param.anticopy,antycopy_param,sizeof ( sys_param.anticopy ) );

}
/******************************************************************************
* Name: 	 device_param_init
*
* Desc:       	 ��ʼ����������(��flash��ȡ���������еĸ�������)
* Param(in):  	 item_list:����ʼ���Ĳ�������
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
static void device_param_init ( config_item_info_t* item_list )
{
	uint16_t index = 0;
	flash_item_info_t flash_item_info;
	table_item_match_t item_match;

	if ( item_list == NULL )
	{
		return ;
	}

	//��ձ��Ӧ��flash�洢�ռ�
	table_clean_flash ( TABLE_SYSTEM_PARAM );
	//�������ò�����flash
	for ( index = 0; index<CONFIG_MAX; index++ )
	{
		flash_item_info.item_id = ( uint16_t ) ( index>>8 ) + ( uint16_t ) ( index<<8 );
		memcpy ( flash_item_info.data, ( uint8_t* ) config_list[index].config_adder,\
		         MIN ( config_list[index].data_len,SYSTEM_PARAM_ITEM_MAX_SIZE ) );

		item_match.offset = 0;
		item_match.match_len = sizeof ( flash_item_info.item_id );
		item_match.match_data[0] = ( uint8_t ) ( ( flash_item_info.item_id>>8 ) &0xff );
		item_match.match_data[1] = ( uint8_t ) ( flash_item_info.item_id&0xff );
		table_write_item ( TABLE_SYSTEM_PARAM,&item_match,&flash_item_info,\
		                   config_list[index].data_len+item_match.match_len );
	}

}

/******************************************************************************
* Name: 	 device_param_default_init
*
* Desc:       	 �ָ���������
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
void device_param_default_init ( void )
{
	uint8_t antycopy_param[20] = {0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x10,0x05,0x05};
		
	memset ( &sys_param,0,sizeof ( config_sys_param_t ) );
	sys_param.modify_flag = CONFIG_MAX;//sizeof(config_sys_param_t);
	sys_param.work_mode = 1;
	memcpy ( sys_param.anticopy,antycopy_param,sizeof ( sys_param.anticopy ) );
	
	
	
	device_param_init ( config_list );
}

/******************************************************************************
* Name: 	 device_config_init
*
* Desc:       	 �豸���ó�ʼ��
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
void device_config_init ( void )
{
	uint16_t index = 0;
	table_item_match_t table_item_match;
	table_item_match_t item_special_match;

	//���ٲ���������������ռ�
	bit_index_info.len = ( CONFIG_MAX&0x07 ) ? ( ( CONFIG_MAX>>3 ) + 1 ) : ( CONFIG_MAX>>3 );
	bit_index_info.buf = ( uint8_t* ) C_MALLOC ( bit_index_info.len );
	if ( bit_index_info.buf == NULL )
	{
		debug ( "C_MALLOC for bit_index_info.buf error!\r\n" );
	}
	else
	{
		//��������¼�
		set_index_bit_valure ( ( bit_index_info.len<<3 )-1,0,&bit_index_info );
	}
	//��flash�ж�ȡ���õ�ram
	for ( index = 0; index<CONFIG_MAX; index++ )
	{
		table_item_match.offset = 0;
		table_item_match.match_len = 2;
		table_item_match.match_data[0] = ( uint8_t ) ( ( index>>8 ) &0xff );
		table_item_match.match_data[1] = ( uint8_t ) ( index&0xff );

		item_special_match.offset = 2;
		item_special_match.match_len = config_list[index].data_len;

		table_read_item_special_data ( TABLE_SYSTEM_PARAM,&table_item_match,&item_special_match,\
		                               ( uint8_t* ) config_list[index].config_adder );
		//�жϱ��Ƿ��޸�
		if ( index == CONFIG_MODIFY )
		{
			debug ( "flash modify flag = %d,except len = %d\r\n",* ( ( uint32_t* ) config_list[index].config_adder ),CONFIG_MAX );
			if(* ( ( uint32_t* ) config_list[index].config_adder ) == 0)//���豸��flashΪ0x00
			{
#if DEBUG_DEVICE_CONFIG
				debug ( "new drivers,init table to default val now!\r\n" );
#endif
				device_param_default_init();
//				break;
			}
			else if ( * ( ( uint32_t* ) config_list[index].config_adder ) < CONFIG_MAX ) //�������������ʼ���¼ӵ�������
			{
				uint8_t new_item;
#if DEBUG_DEVICE_CONFIG
				debug ( "table struct member increase,init new increase member!\r\n" );
#endif
				//��ʼ�������ӵ���Ŀ
				sys_param_new_member_init();
				for ( new_item = CONFIG_MAX; new_item> ( * ( ( uint32_t* ) config_list[index].config_adder ) ); new_item-- )
				{
					update_item ( new_item-1,TABLE_SYSTEM_PARAM, ( uint8_t* ) config_list[new_item-1].config_adder );
				}
				//���±���޸ı�־
				sys_param.modify_flag = CONFIG_MAX;
				update_item ( CONFIG_MODIFY,TABLE_SYSTEM_PARAM, ( uint8_t* ) config_list[CONFIG_MODIFY].config_adder );
			}
			else if ( * ( ( uint32_t* ) config_list[index].config_adder )   > CONFIG_MAX ) //��������٣��ָ���������
			{
#if DEBUG_DEVICE_CONFIG
				debug ( "table struct member reduction,init table to default val now!\r\n" );
#endif
				device_param_default_init();
//				break;
			}
		}
#if DEBUG_DEVICE_CONFIG
		debug ( "%s addr = %p,%s = ",config_list[index].config_name,\
		        config_list[index].config_adder,config_list[index].config_name );
		if ( config_list[index].data_type == DATA_TYPE_CHAR )
		{
			debug_char ( ( uint8_t* ) config_list[index].config_adder,config_list[index].data_len );
		}
		else if ( config_list[index].data_type == DATA_TYPE_IP || \
		          config_list[index].data_type == DATA_TYPE_BCD || \
		          config_list[index].data_type == DATA_TYPE_NUMBER )
		{
			debug_dec ( ( uint8_t* ) config_list[index].config_adder,config_list[index].data_len );
		}
		else if(config_list[index].data_type == DATA_TYPE_HEX)
		{
			debug_hex ( ( uint8_t* ) config_list[index].config_adder,config_list[index].data_len );
		}
#endif
	}
}
void device_config_real_time ( void )
{
	//�������ͬ��
	if ( need_sync_to_flash )
	{
		uint32_t index = 0;
		for ( index = 0; index< CONFIG_MAX; index++ )
		{
			//�ж���Ҫ���µĲ���
			if ( get_config_change_evt ( ( CONFIG_PARAMETER_E ) index ) )
			{
				debug ( "refresh parameter %s to flash!\r\n",config_list[index].config_name );
				update_item ( index,TABLE_SYSTEM_PARAM, ( uint8_t* ) config_list[index].config_adder );
//                //�����Ӧ�¼������·�ͳһ���
//                set_config_change_evt((CONFIG_PARAMETER_E)index,0);
			}
		}
		need_sync_to_flash = 0;
		//��������¼�
		set_index_bit_valure ( ( bit_index_info.len<<3 )-1,0,&bit_index_info );
	}
}
/******************************************************************************
* Name: 	 config_get_data
*
* Desc:       	 ��ȡ���ò���
* Param(in):  	 index:������
* Param(out): 	 data :��������������
* Return:     	 ��ȡ�����������ݵĳ���
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
uint8_t config_get_data ( CONFIG_PARAMETER_E index,uint8_t** data )
{
	uint8_t len=0;
	if ( index >= CONFIG_MAX || data == NULL )
	{
		return len;
	}
	*data = ( uint8_t* ) config_list[index].config_adder;
	len = config_list[index].data_len;
	return len;
}

/******************************************************************************
* Name: 	 config_set_data
*
* Desc:       	 �������ò���
* Param(in):  	 index:������
                 data :д�����������
* Param(out):
* Return:     	 д������ݳ���
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
uint8_t config_set_data ( CONFIG_PARAMETER_E index,uint8_t* data )
{
	uint8_t len=0;

	if ( index >= CONFIG_MAX || data == NULL )
	{
		return len;
	}

	//���±���ram
	memcpy ( ( uint8_t* ) config_list[index].config_adder,data,config_list[index].data_len );
	//д��flash
	len = update_item ( index,TABLE_SYSTEM_PARAM,data );
	//�����������ֱ����Ч��ͬ����flash������Ҫ�������������¼�
//    //������Ӧ�Ĳ��������¼�
//    set_config_change_evt(index,1);
	return len;
}

/******************************************************************************
* Name: 	 config_refresh_data
*
* Desc:       	 ˢ�����ò���(����ram�еĲ�������δ���浽flash)
* Param(in):  	 index:������
                 data :д�����������
* Param(out):
* Return:
* Global:
* Note:       	 ���ֶ�����set_param_sync_to_flash������ͬ��flash
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
uint8_t config_refresh_data ( CONFIG_PARAMETER_E index,uint8_t* data )
{
	uint8_t len=0;
	if ( index >= CONFIG_MAX || data == NULL )
	{
		return len;
	}
	//���±���ram
	memcpy ( ( uint8_t* ) config_list[index].config_adder,data,config_list[index].data_len );
	len = config_list[index].data_len;
	//������Ӧ�Ĳ��������¼�
	set_config_change_evt ( index,1 );
	//��ϣ������ͬ����flash��ʱ���ֶ�����set_param_sync_to_flash������ͬ��flash
	return len;
}
/******************************************************************************
* Name: 	 get_config_list_item_info
*
* Desc:       	 ��ȡ������ʵʱ��Ϣ
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
const config_item_info_t* get_config_list_item_info ( CONFIG_PARAMETER_E index )
{
	if ( index >= CONFIG_MAX )
	{
		return NULL;
	}

	return &config_list[index];
}
/*---------------------------------------------------------------------------*/

