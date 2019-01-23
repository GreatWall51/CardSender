/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 debug_reg_table.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/24
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/24, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "debug.h"
#include <stdlib.h>
#include "reg_table.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/

uint16_t item_id = 0x0003;
uint8_t write_data[10]={0,3,192,168,1,1,0x07,0x08,0x09,0x10};
uint8_t read_data[20]={0};
/*----------------------- Function Prototype --------------------------------*/

//需在主函数调用reg_table_init() 和 reg_table_real_time();
/*----------------------- Function Implement --------------------------------*/
DEBUG_CMD_FN(reg_table)
{
    dbg_cmd_split_t *dbg_cmd_split;

    dbg_cmd_split = get_dbg_cmd_split();
    if(strncmp(dbg_cmd_split->arg[1], "table_get_effect_item_cnt", strlen("table_get_effect_item_cnt")) == 0)
	{
	    uint32_t cnt =0;
	    cnt = table_get_effect_item_cnt(TABLE_SYSTEM_PARAM);
	    debug("TABLE_SYSTEM_PARAM effect cnt = %d\r\n",cnt);
	}
	else if(strncmp(dbg_cmd_split->arg[1], "table_write_item", strlen("table_write_item")) == 0)
	{
	    table_item_match_t table_item_match;
	    uint8_t cnt;
	    
	    table_item_match.offset = 0;
	    table_item_match.match_len = 2;
	    table_item_match.match_data[0] = (uint8_t)((item_id>>8)&0xff);
	    table_item_match.match_data[1] = (uint8_t)(item_id&0xff);
	    cnt = table_write_item(TABLE_SYSTEM_PARAM,&table_item_match,write_data,sizeof(write_data));
	    debug("write %d byte to flash!\r\n",cnt);
	}
	else if(strncmp(dbg_cmd_split->arg[1], "table_read_item", strlen("table_read_item")) == 0)
	{
	    table_item_match_t table_item_match;
	    uint8_t cnt;
	    
	    table_item_match.offset = 0;
	    table_item_match.match_len = 2;
	    table_item_match.match_data[0] = (uint8_t)((item_id>>8)&0xff);
	    table_item_match.match_data[1] = (uint8_t)(item_id&0xff);
	    memset(read_data,0,20);
	    cnt = table_read_item(TABLE_SYSTEM_PARAM,&table_item_match,read_data);
	    if((cnt>0) && (cnt<=20))
	    {
	        debug("read data(len = %d):\r\n",cnt);
	        debug_hex(read_data,cnt);
	    }
	    else
	    {
	        debug("table_read_item fail(len = %d)!\r\n",cnt);
	    }
	}
	else if(strncmp(dbg_cmd_split->arg[1], "table_delete_item", strlen("table_delete_item")) == 0)
	{
	    table_item_match_t table_item_match;
	    uint8_t result = 1;
	    table_item_match.offset = 0;
	    table_item_match.match_len = 2;
	    table_item_match.match_data[0] = (uint8_t)((item_id>>8)&0xff);
	    table_item_match.match_data[1] = (uint8_t)(item_id&0xff);
	    
	    result = table_delete_item(TABLE_SYSTEM_PARAM,&table_item_match);
	    if(result == 0)
	    {
	        debug("table_delete_item success!\r\n");
	    }
	    else
	    {
	        debug("table_delete_item fail!\r\n");
	    }
	}
	else if(strncmp(dbg_cmd_split->arg[1], "table_clean_flash", strlen("table_clean_flash")) == 0)
	{
	    uint8_t result = 1;
	    result = table_clean_flash(TABLE_SYSTEM_PARAM);
	    if(result == 0)
	    {
	        debug("table_clean_flash success!\r\n");
	    }
	    else
	    {
	        debug("table_clean_flash fail!\r\n");
	    }
	}
	else if(strncmp(dbg_cmd_split->arg[1], "table_read_special", strlen("table_read_special")) == 0)
	{
	    table_item_match_t table_item_match;
	    table_item_match_t item_special_match;
	    uint8_t cnt;
	    
	    table_item_match.offset = 0;
	    table_item_match.match_len = 2;
	    table_item_match.match_data[0] = (uint8_t)((item_id>>8)&0xff);
	    table_item_match.match_data[1] = (uint8_t)(item_id&0xff);

        item_special_match.offset = 2;
	    item_special_match.match_len = 8;
   
	    memset(read_data,0,20);
	    cnt = table_read_item_special_data(TABLE_SYSTEM_PARAM,&table_item_match,&item_special_match,read_data);
	    if((cnt>0) && (cnt<=20))
	    {
	        debug("read data(len = %d):\r\n",cnt);
	        debug_hex(read_data,cnt);
	    }
	    else
	    {
	        debug("table_read_item fail(len = %d)!\r\n",cnt);
	    }
	}
}

/*---------------------------------------------------------------------------*/

