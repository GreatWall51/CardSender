/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 debug_reg_record.c
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


/*------------------------------- Includes ----------------------------------*/
#include "debug.h"
#include "reg_record.h"
#include <stdlib.h>
/*------------------- Global Definitions and Declarations -------------------*/
static reg_base_t test_reg_base;
#define TEST_REG_RECORD_ADDR 0X0800B000
/*---------------------- Constant / Macro Definitions -----------------------*/
uint8_t mach_data[2] = {0x11,0x22};
uint8_t reg_write_data[9] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99};
uint8_t reg_read_data[200] = {0};
/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
void test_reg_record_init(void)
{
	  reg_record_init(&test_reg_base,TEST_REG_RECORD_ADDR,30,ITEM_SIZE_32);
}
void test_reg_record_real_time(void)
{
	reg_record_real_time(&test_reg_base);
}
/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
DEBUG_CMD_FN(reg_record)
{
    dbg_cmd_split_t *dbg_cmd_split;

    dbg_cmd_split = get_dbg_cmd_split();
		if(strncmp(dbg_cmd_split->arg[1], "get_test_reg_base_info", strlen("get_test_reg_base_info")) == 0)
		{
			debug("test_reg_base info:\r\n");
			debug("start_sector = %d\r\n",test_reg_base.start_sector);
			debug("back_sector = %d\r\n",test_reg_base.back_sector);
			debug("max_index = %d\r\n",test_reg_base.max_index);
			debug("total_items_cnt = %d\r\n",test_reg_base.items_cnt);
			debug("per_page_item_cnt = %d\r\n",test_reg_base.cfg_per_page_item_cnt);
			debug("per_item_size = %d\r\n",test_reg_base.cfg_item_size);
			debug("index_use_array addr = %p\r\n",test_reg_base.index_use_array);
			debug("index_effictive_array addr = %p\r\n",test_reg_base.index_effictive_array);
			
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_match_write", strlen("reg_record_match_write")) == 0)
		{
				uint32_t result;
				match_t match_item;
			
				debug("get reg_record_match_write cmd success!\r\n");
			  match_item.match_len = 0x02;
			  match_item.offset = 0;
				match_item.match_data = malloc(2);
			  memcpy(match_item.match_data,mach_data,2);
			  result = reg_record_match_write(&test_reg_base,&match_item,reg_write_data,9);
				if(result)
				{
					debug("reg_record_match_write %d bytes success!\r\n",result);
				}
				else
				{
						debug("reg_record_match_write fail!\r\n");
				}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_no_match_write", strlen("reg_record_no_match_write")) == 0)
		{
				uint32_t result;

			  result = reg_record_match_write(&test_reg_base,NULL,reg_write_data,9);
				if(result)
				{
					debug("reg_record_match_write %d bytes success!\r\n",result);
				}
				else
				{
						debug("reg_record_match_write fail!\r\n");
				}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_match_get", strlen("reg_record_match_get")) == 0)
		{
				match_t match_item;
			uint32_t len;
			
			match_item.match_len = 0x02;
			  match_item.offset = 0;
				match_item.match_data = malloc(2);
			  memcpy(match_item.match_data,mach_data,2);
			
			len = reg_record_get_match_item(&test_reg_base,&match_item,reg_read_data);
			if(len)
			{
				debug("reg_record_match_get data(len = %d):\r\n",len);
				debug_hex(reg_read_data,len);
			}
				else
				{
					debug("reg_record_match_get fail!\r\n");
				}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_match_del_all", strlen("reg_record_match_del_all")) == 0)
		{
				match_t match_item;
			  uint8_t result;
			
			  match_item.match_len = 0x02;
			  match_item.offset = 0;
				match_item.match_data = malloc(2);
			  memcpy(match_item.match_data,mach_data,2);
				result = reg_record_match_delete_all(&test_reg_base,&match_item);
				if(result == 1)
				{
						debug("reg_record_match_del_all success!\r\n");
				}
				else if(result == 2)
				{
						debug("no fin mach item!\r\n");
				}
				else
				{
						debug("reg_record_match_del_all fial!\r\n");
				}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_get_effect_item_num", strlen("reg_record_get_effect_item_num")) == 0)
		{
			uint16_t cnt = 0;
			cnt = reg_record_get_effect_item_num(&test_reg_base);
			debug("table effect item total num is :%d\r\n",cnt);
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_del_all", strlen("reg_record_del_all")) == 0)
		{
			reg_record_del_all(&test_reg_base);
			debug("reg_record_del_all success!\r\n");
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_get_match_item_cnt", strlen("reg_record_get_match_item_cnt")) == 0)
		{
			uint16_t cnt=0;
			match_t match_item;

			match_item.match_len = 0x02;
			match_item.offset = 0;
			match_item.match_data = malloc(2);
			memcpy(match_item.match_data,mach_data,2);
			cnt = reg_record_get_match_item_cnt(&test_reg_base,&match_item);
			debug("match item total cnt = %d\r\n",cnt);
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_match_del_first", strlen("reg_record_match_del_first")) == 0)
		{
			uint8_t result = 0;
			match_t match_item;

			match_item.match_len = 0x02;
			match_item.offset = 0;
			match_item.match_data = malloc(2);
			memcpy(match_item.match_data,mach_data,2);
			result = reg_record_match_delete(&test_reg_base,&match_item);
			if(result == 1)
			{
					debug("reg_record_match_del_first success!\r\n");
			}
			else if(result == 2)
			{
					debug("no find mach item!\r\n");
			}
			else
			{
					debug("reg_record_match_del_first fial!\r\n");
			}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "match_get_data", strlen("match_get_data")) == 0)
		{
			uint16_t len = 0;
			uint8_t data[200]={0};
			match_t match_item;

			match_item.match_len = 0x02;
			match_item.offset = 0;
			match_item.match_data = malloc(2);
			memcpy(match_item.match_data,mach_data,2);
			len = reg_record_get_all_item_offset_len_data(&test_reg_base,&match_item,data);
			if(len)
			{
				debug("get match data:\r\n");
				debug_hex(data,len);
			}
			else
			{
				debug("reg_record_match_get_data fail!\r\n");
			}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "reg_record_get_match_item_data_length", strlen("reg_record_get_match_item_data_length")) == 0)
		{
			uint16_t len = 0;
			match_t match_item;

			match_item.match_len = 0x02;
			match_item.offset = 0;
			match_item.match_data = mach_data;
			len = reg_record_get_match_item_actual_length(&test_reg_base,&match_item);
			debug("reg_record_get_match_item_data_length = %d",len);
		}
		else if (strncmp(dbg_cmd_split->arg[1], "get_match_special", strlen("get_match_special")) == 0)
		{
			uint16_t len = 0;
			match_t match_item;
			match_t match_special;
			uint8_t data[20]={0};

			match_item.match_len = 0x02;
			match_item.offset = 0;
			match_item.match_data = malloc(2);
			memcpy(match_item.match_data,mach_data,2);
			
			match_special.match_len = 0x05;
			match_special.offset = 3;
			match_special.match_data = data;
			len = reg_record_get_match_item_special_data(&test_reg_base,&match_item,&match_special,match_special.match_data);
			if(len)
			{
				debug("get match data:\r\n");
				debug_hex(match_special.match_data,len);
			}
			else
			{
				debug("reg_record_match_get_data fail!\r\n");
			}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "get_all_match", strlen("get_all_match")) == 0)
		{
				match_t match_item;
			  uint32_t len;
			
			  match_item.match_len = 0x02;
			  match_item.offset = 0;
				match_item.match_data = mach_data;
			
			len = reg_record_get_match_item_all(&test_reg_base,&match_item,reg_read_data);
			if(len)
			{
				debug("get_all_match data(len = %d):\r\n",len);
				debug_hex(reg_read_data,len);
			}
				else
				{
					debug("get_all_match data fail!\r\n");
				}
		}
}

/*---------------------------------------------------------------------------*/

