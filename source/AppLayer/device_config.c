/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 device_config.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/25
* Notes:	如果发现数据没写到flash，考虑所写的数据是否大于配置表条目的长度
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
(1)本表的每一个节点对应flash中的一条item，新增加成员时，只能往最后一个数组的后面加，否则会破坏flash中的对应关系
(2)本表config_list必须与CONFIG_PARAMETER_E一一对应
(3)增加成员变量不会擦除之前的数据，删除成员变量会将所有数据恢复默认值
(4)新增的成员需在函数sys_param_new_member_init中进行初始化
(5)在device_param_default_init设置恢复出厂默认值
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
	uint16_t item_id;    //配置条目编号
	uint8_t data[SYSTEM_PARAM_ITEM_MAX_SIZE];
} flash_item_info_t;

typedef struct
{
	uint16_t len;//buf的长度(单位:字节)
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
* Desc:       	 获取索引位的值
* Param(in):  	 index:索引位
                 info: 所以结构体信息
* Param(out):
* Return:     	 索引位的值
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
* Desc:       	 设置索引位的值
* Param(in):  	 index:索引位
                 val:  索引位的值
                 info: 所以结构体信息
* Param(out):
* Return:     	 设置的值
* Global:
* Note:       	 当index为((info->len<<3)-1))，表示设置整个索引组数的值
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
* Desc:       	 更新表中指定的条目到flash
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
* Desc:       	 初始化sys_param新增加的成员到本地ram
* Param(in):
* Param(out):
* Return:
* Global:
* Note:       	 该函数在成员变量修改时，每次都得重新编码
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/25, Create this function by Liuwq
 ******************************************************************************/
void sys_param_new_member_init ( void )
{
	uint8_t antycopy_param[20] = {0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x10,0x05,0x05};
	//只能在这初始化新增加的成员变量，初始化非新增成员变量是无效的。
	sys_param.work_mode = 1;
	memcpy ( sys_param.anticopy,antycopy_param,sizeof ( sys_param.anticopy ) );

}
/******************************************************************************
* Name: 	 device_param_init
*
* Desc:       	 初始化参数链表(从flash读取参数到表中的各个变量)
* Param(in):  	 item_list:待初始化的参数链表
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

	//清空表对应的flash存储空间
	table_clean_flash ( TABLE_SYSTEM_PARAM );
	//设置配置参数到flash
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
* Desc:       	 恢复出厂设置
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
* Desc:       	 设备配置初始化
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

	//开辟参数更新索引数组空间
	bit_index_info.len = ( CONFIG_MAX&0x07 ) ? ( ( CONFIG_MAX>>3 ) + 1 ) : ( CONFIG_MAX>>3 );
	bit_index_info.buf = ( uint8_t* ) C_MALLOC ( bit_index_info.len );
	if ( bit_index_info.buf == NULL )
	{
		debug ( "C_MALLOC for bit_index_info.buf error!\r\n" );
	}
	else
	{
		//清空所有事件
		set_index_bit_valure ( ( bit_index_info.len<<3 )-1,0,&bit_index_info );
	}
	//从flash中读取配置到ram
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
		//判断表是否被修改
		if ( index == CONFIG_MODIFY )
		{
			debug ( "flash modify flag = %d,except len = %d\r\n",* ( ( uint32_t* ) config_list[index].config_adder ),CONFIG_MAX );
			if(* ( ( uint32_t* ) config_list[index].config_adder ) == 0)//新设备，flash为0x00
			{
#if DEBUG_DEVICE_CONFIG
				debug ( "new drivers,init table to default val now!\r\n" );
#endif
				device_param_default_init();
//				break;
			}
			else if ( * ( ( uint32_t* ) config_list[index].config_adder ) < CONFIG_MAX ) //增加了配置项，初始化新加的配置项
			{
				uint8_t new_item;
#if DEBUG_DEVICE_CONFIG
				debug ( "table struct member increase,init new increase member!\r\n" );
#endif
				//初始化新增加的条目
				sys_param_new_member_init();
				for ( new_item = CONFIG_MAX; new_item> ( * ( ( uint32_t* ) config_list[index].config_adder ) ); new_item-- )
				{
					update_item ( new_item-1,TABLE_SYSTEM_PARAM, ( uint8_t* ) config_list[new_item-1].config_adder );
				}
				//更新表的修改标志
				sys_param.modify_flag = CONFIG_MAX;
				update_item ( CONFIG_MODIFY,TABLE_SYSTEM_PARAM, ( uint8_t* ) config_list[CONFIG_MODIFY].config_adder );
			}
			else if ( * ( ( uint32_t* ) config_list[index].config_adder )   > CONFIG_MAX ) //配置项变少，恢复出厂设置
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
	//处理参数同步
	if ( need_sync_to_flash )
	{
		uint32_t index = 0;
		for ( index = 0; index< CONFIG_MAX; index++ )
		{
			//判断需要更新的参数
			if ( get_config_change_evt ( ( CONFIG_PARAMETER_E ) index ) )
			{
				debug ( "refresh parameter %s to flash!\r\n",config_list[index].config_name );
				update_item ( index,TABLE_SYSTEM_PARAM, ( uint8_t* ) config_list[index].config_adder );
//                //清除相应事件，在下方统一清空
//                set_config_change_evt((CONFIG_PARAMETER_E)index,0);
			}
		}
		need_sync_to_flash = 0;
		//清空所有事件
		set_index_bit_valure ( ( bit_index_info.len<<3 )-1,0,&bit_index_info );
	}
}
/******************************************************************************
* Name: 	 config_get_data
*
* Desc:       	 获取配置参数
* Param(in):  	 index:配置项
* Param(out): 	 data :读到的配置数据
* Return:     	 读取到的配置数据的长度
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
* Desc:       	 设置配置参数
* Param(in):  	 index:配置项
                 data :写入的配置数据
* Param(out):
* Return:     	 写入的数据长度
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

	//更新本地ram
	memcpy ( ( uint8_t* ) config_list[index].config_adder,data,config_list[index].data_len );
	//写入flash
	len = update_item ( index,TABLE_SYSTEM_PARAM,data );
	//这里参数设置直接生效且同步到flash，不需要产生参数更新事件
//    //产生对应的参数更新事件
//    set_config_change_evt(index,1);
	return len;
}

/******************************************************************************
* Name: 	 config_refresh_data
*
* Desc:       	 刷新配置参数(更新ram中的参数，并未保存到flash)
* Param(in):  	 index:配置项
                 data :写入的配置数据
* Param(out):
* Return:
* Global:
* Note:       	 需手动调用set_param_sync_to_flash将参数同步flash
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
	//更新本地ram
	memcpy ( ( uint8_t* ) config_list[index].config_adder,data,config_list[index].data_len );
	len = config_list[index].data_len;
	//产生对应的参数更新事件
	set_config_change_evt ( index,1 );
	//在希望参数同步到flash的时候，手动调用set_param_sync_to_flash将参数同步flash
	return len;
}
/******************************************************************************
* Name: 	 get_config_list_item_info
*
* Desc:       	 获取配置项实时信息
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

