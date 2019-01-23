/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 reg_table.h
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/18
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/18, Liuwq create this file
*
******************************************************************************/
#ifndef _REG_TABLE_H_
#define _REG_TABLE_H_


/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
#include "reg_record.h"
/*----------------------------- Global Defines ------------------------------*/
#define MATCH_DATA_MAX_LEN 7

/*----------------------------- Global Typedefs -----------------------------*/
typedef enum
{
	TABLE_SYSTEM_PARAM = 0,    //系统参数配置表
	TABLE_SCENE,               //情景表

	TABLE_MAX,
} TABLE_E;

__packed typedef struct
{
	uint8_t offset;          /*!< 匹配信息在FLASH中的起始偏移*/
	uint8_t match_len;       /*!< 匹配信息的长度*/
	uint8_t match_data[MATCH_DATA_MAX_LEN];   /*!< 匹配信息数据*/
} table_item_match_t;
/*----------------------------- External Variables --------------------------*/
/* 系统参数配置表存储空间定义 */
#define SYSTEM_PARAM_ITEM_MAX_CNT       20             //系统参数配置表最大条目数
#define SYSTEM_PARAM_ITEM_MAX_SIZE      ITEM_SIZE_32   //系统参数配置表条目最大字节

///* 情景表存储空间定义 */
//#define SCENE_ITEM_MAX_CNT       64             //情景表最大条目数
//#define SCENE_ITEM_MAX_SIZE      ITEM_SIZE_32   //情景表条目最大字节

/*------------------------ Global Function Prototypes -----------------------*/
void reg_table_init ( void );
void reg_table_real_time ( void );



//table_item_match_t *get_table_match_info(TABLE_E	table_id);
//uint8_t set_table_match_info(TABLE_E	table_id,uint8_t offset,uint8_t match_len,uint8_t *match_data);

uint8_t table_clean_flash ( TABLE_E	table_id );
uint32_t table_get_effect_item_cnt ( TABLE_E	table_id );
uint8_t table_delete_item ( TABLE_E	table_id,table_item_match_t* p_match_item );
uint8_t table_write_item ( TABLE_E	table_id,table_item_match_t* p_match_item,const void* p_item,uint8_t len );
uint8_t table_read_item ( TABLE_E	table_id,table_item_match_t* p_match_item,uint8_t* read_data );
uint8_t table_read_item_special_data ( TABLE_E	table_id,table_item_match_t*      p_match_item, table_item_match_t*   p_match_special,const void* p_item );
#endif //_REG_TABLE_H_
