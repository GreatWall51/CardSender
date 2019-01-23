/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 reg_table.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/19
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/19, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "reg_table.h"
#include "opt.h"
#include "debug.h"
/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/
#define TABLE_START_ADDR FLASH_USER_START_ADDR

/*----------------------- Type Declarations ---------------------------------*/
__packed typedef struct
{
	reg_base_t* p_reg_base;
} reg_config_t;

/*----------------------- Variable Declarations -----------------------------*/


static const uint8_t table_info[TABLE_MAX][2] =
{
	{SYSTEM_PARAM_ITEM_MAX_CNT,SYSTEM_PARAM_ITEM_MAX_SIZE},
//	{SCENE_ITEM_MAX_CNT,SCENE_ITEM_MAX_SIZE},
};



//const table_item_match_t match_table[TABLE_MAX] =
// {
//    {0x02,0x02,{0x00,0x00,0x00,0x00,0x00,0x00,0x00}},       //系统参数配置表匹配数据
//    {0x02,0x02,{0x00,0x00,0x00,0x00,0x00,0x00,0x00}},       //情景表匹配数据
// };

static reg_base_t reg_base[TABLE_MAX]= {0};
static reg_config_t reg_config[TABLE_MAX]= {0};
/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name: 	 reg_table_init
*
* Desc:       	 表初始化
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/20, Create this function by Liuwq
 ******************************************************************************/
void reg_table_init ( void )
{
	uint8_t table_index;
	uint32_t start_addr = TABLE_START_ADDR;

	for ( table_index =0; table_index<TABLE_MAX; table_index++ )
	{
#if DEBUG_REG_TABLE
		debug ( "table_%d start addr:0x%x\r\n",table_index,start_addr );
#endif
		reg_config[table_index].p_reg_base = &reg_base[table_index];
		start_addr = reg_record_init ( reg_config[table_index].p_reg_base, start_addr, table_info[table_index][0], ( ITEM_SIZE_ENUM ) table_info[table_index][1] );
		start_addr++;
#if DEBUG_REG_TABLE
		debug ( "table_%d end addr:0x%x\r\n",table_index,start_addr-1 );
#endif
	}
}
/******************************************************************************
* Name: 	 reg_table_real_time
*
* Desc:       	 表实时函数，实时检测表是否已经满，并进行整理
* Param(in):
* Param(out):
* Return:
* Global:
* Note:       	 该函数比较耗时间
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/23, Create this function by Liuwq
 ******************************************************************************/
void reg_table_real_time ( void )
{
	uint8_t table_index;
	for ( table_index =0; table_index<TABLE_MAX; table_index++ )
	{
		reg_record_real_time ( reg_config[table_index].p_reg_base );
	}
}

/******************************************************************************
* Name: 	 get_table_match_info
*
* Desc:       	 获取条目的匹配信息
* Param(in):  	 table_id:指定的表(若为TABLE_MAX,删除所有表对应的flash)
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/23, Create this function by Liuwq
 ******************************************************************************/
//table_item_match_t *get_table_match_info(TABLE_E	table_id)
//{
//    table_item_match_t *match_item;
//
//    if(table_id >= TABLE_MAX) return NULL;
//    match_item = (table_item_match_t *)&match_table[table_id];
//    return match_item;
//}


/******************************************************************************
* Name: 	 get_table_match_info
*
* Desc:       	 获取条目的匹配信息
* Param(in):  	 table_id:    指定的表(若为TABLE_MAX,删除所有表对应的flash)
                 offset:      匹配数据偏移位置
                 match_len:   匹配数据长度
                 match_data:  匹配数据
* Param(out):
* Return:     	 0->success,>0->fail
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/23, Create this function by Liuwq
 ******************************************************************************/
//uint8_t set_table_match_info(TABLE_E	table_id,uint8_t offset,uint8_t match_len,uint8_t *match_data)
//{
//    table_item_match_t *match_item = NULL;
//
//    match_item = (table_item_match_t *)&match_table[table_id];
//    if(table_id >= TABLE_MAX || match_data == NULL || match_len > MATCH_DATA_MAX_LEN) return 1;
//    match_item->offset       = offset;
//    match_item->match_len    = match_len;
//    memcpy(match_item->match_data,match_data,match_len);
//    return 0;
//}

/******************************************************************************
* Name: 	 table_clean_flash
*
* Desc:       	 清空表所对应的flash
* Param(in):  	 table_id:指定的表(若为TABLE_MAX,删除所有表对应的flash)
* Param(out):
* Return:     	 0 -> success,
                 1 -> parameter error,
                 2 -> erase sector error,
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/20, Create this function by Liuwq
 ******************************************************************************/
uint8_t table_clean_flash ( TABLE_E	table_id )
{
	uint8_t result = 1;
	if ( table_id > TABLE_MAX )
	{
		return result;
	}
	if ( table_id == TABLE_MAX )
	{
		uint16_t table_index = 0;
		for ( table_index = 0; table_index<TABLE_MAX; table_index++ )
		{
			result = reg_record_del_all ( reg_config[table_index].p_reg_base );
			if ( result != 0 )
			{
				return result;
			}
		}
	}
	else
	{
		result = reg_record_del_all ( reg_config[table_id].p_reg_base );
	}
	return result;
}

/******************************************************************************
* Name: 	 table_get_effect_item_cnt
*
* Desc:       	 获取表中有效条目的个数
* Param(in):  	 table_id:指定的表
* Param(out):
* Return:     	 存储表中有效条目数量
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/23, Create this function by Liuwq
 ******************************************************************************/
uint32_t table_get_effect_item_cnt ( TABLE_E	table_id )
{
	uint32_t item_cnt = 0;

	if ( table_id >= TABLE_MAX )
	{
		return 0;
	}
	item_cnt = reg_record_get_effect_item_num ( reg_config[table_id].p_reg_base );
	return item_cnt;
}
/******************************************************************************
* Name: 	 table_delete_one_item
*
* Desc:       	 删除检索到的第一条匹配的条目
* Param(in):  	 table_id:指定的表
                 p_match_item: 匹配信息
* Param(out):
* Return:     	 0->success,>0->fail
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/23, Create this function by Liuwq
 ******************************************************************************/
uint8_t table_delete_item ( TABLE_E	table_id,table_item_match_t* p_match_item )
{
	uint8_t result=1;
	match_t match;

	//参数有效性判断
	if ( table_id >= TABLE_MAX || p_match_item == NULL )
	{
		return result;
	}
	match.offset = p_match_item->offset;
	match.match_len = p_match_item->match_len;
	match.match_data = p_match_item->match_data;
	result  = reg_record_match_delete ( reg_config[table_id].p_reg_base,&match );
	return result;
}
/******************************************************************************
* Name: 	 table_write_one_item
*
* Desc:       	 向表中写入一条数据
* Param(in):  	 table_id:     指定的表
                 p_match_item: 匹配信息(NULL不进行匹配,找到空位置就直接写入，
                                  !=NULL删除所有匹配信息，并进行写入一条)
                 p_item:       待写入的数据
                 len:          待写入的数据的长度
* Param(out):
* Return:     	 实际写入的字节数
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/23, Create this function by Liuwq
 ******************************************************************************/
uint8_t table_write_item ( TABLE_E	table_id,table_item_match_t* p_match_item,const void* p_item,uint8_t len )
{
	uint8_t cnt = 0;
	match_t match;

	if ( table_id >= TABLE_MAX ||  p_item == NULL || len == 0 )
	{
		return cnt;
	}
#if DEBUG_REG_TABLE
	debug ( "prepare to write data(len = %d):\r\n",len );
	debug_hex ( ( uint8_t* ) p_item,len );
#endif
	if ( p_match_item != NULL )
	{
		match.offset = p_match_item->offset;
		match.match_len = p_match_item->match_len;
		match.match_data = p_match_item->match_data;
#if DEBUG_REG_TABLE
		debug ( "match data(offect = %d,len = %d):",match.offset,match.match_len );
		debug_hex ( match.match_data,match.match_len );
#endif
		cnt = reg_record_match_write ( reg_config[table_id].p_reg_base,&match,p_item,len );
	}
	else
	{
		cnt = reg_record_match_write ( reg_config[table_id].p_reg_base,NULL,p_item,len );
	}

	return cnt;
}
/******************************************************************************
* Name: 	 table_read_one_item
*
* Desc:       	 向表中读取一条数据
* Param(in):  	 table_id:     指定的表
                 p_match_item: 匹配信息
* Param(out): 	 read_data:    读到的数据
* Return:     	 取出的条目的有效数据长度(长度为0，找不到对应条目，取条目失败)
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/23, Create this function by Liuwq
 ******************************************************************************/
uint8_t table_read_item ( TABLE_E	table_id,table_item_match_t* p_match_item,uint8_t* read_data )
{
	uint8_t read_len = 0;
	match_t match;

	if ( table_id >= TABLE_MAX || p_match_item == NULL || p_match_item->match_len == 0 || read_data == NULL )
	{
		return read_len;
	}
	match.offset = p_match_item->offset;
	match.match_len = p_match_item->match_len;
	match.match_data = p_match_item->match_data;
	read_len = reg_record_get_match_item ( reg_config[table_id].p_reg_base,&match,read_data );
	return read_len;
}
/******************************************************************************
* Name: 	 table_read_item_special_data
*
* Desc:       	 向表中读取一条数据(指定偏移位置及长度)
* Param(in):  	 table_id:     指定的表
                 p_match_item: 匹配信息
                 p_match_special:偏移位置及长度
* Param(out): 	 read_data:    读到的数据
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/23, Create this function by Liuwq
 ******************************************************************************/
uint8_t table_read_item_special_data ( TABLE_E	table_id,table_item_match_t*      p_match_item, table_item_match_t*   p_match_special,const void* p_item )
{
	uint8_t read_len = 0;
	match_t match;
	match_t match_special;

	if ( table_id >= TABLE_MAX || p_match_item == NULL || p_item == NULL || p_match_item->match_len == 0 || p_match_special == NULL )
	{
		return read_len;
	}
	match.offset = p_match_item->offset;
	match.match_len = p_match_item->match_len;
	match.match_data = p_match_item->match_data;

	match_special.offset = p_match_special->offset;
	match_special.match_len = p_match_special->match_len;

#if DEBUG_REG_TABLE
	debug ( "match data(offect = %d,len = %d):\r\n",match.offset,match.match_len );
	debug_hex ( match.match_data,match.match_len );
	debug ( "match_special(offect = %d,len = %d):\r\n",match_special.offset,match_special.match_len );
#endif
	read_len = reg_record_get_match_item_special_data ( reg_config[table_id].p_reg_base,&match,&match_special,p_item );
	return read_len;
}

/*---------------------------------------------------------------------------*/

