/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 debug_flash.c 
* Desc:
* 
* 
* Author: 	 Liuwq
* Date: 	 2018/07/16
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/16, Liuwq create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include "debug.h"
#include "flash_drv.h"
#include <string.h>
/*------------------- Global Definitions and Declarations -------------------*/
#ifndef ADDR_2_SECTOR
#define ADDR_2_SECTOR(ADDR) ((ADDR)/FLASH_PAGE_SIZE)
#endif
 
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/
 
 
/*----------------------- Variable Declarations -----------------------------*/

 
/*----------------------- Function Prototype --------------------------------*/

 
/*----------------------- Function Implement --------------------------------*/
DEBUG_CMD_FN(flash_drv)
{
    dbg_cmd_split_t *dbg_cmd_split;

    dbg_cmd_split = get_dbg_cmd_split();
		if (strncmp(dbg_cmd_split->arg[1], "flash_read", strlen("flash_read")) == 0)
		{
			uint32_t read_byte;
			uint8_t read_data[7] ={0};
			memset(read_data,0,sizeof(read_data));
			read_byte = flash_read(FLASH_USER_START_ADDR,read_data,7);
			if(read_byte >0)
			{
			debug("flash_read data:\r\n");
			debug_hex(read_data,read_byte);
			}
			else
			{
				debug("flash_read error!\r\n");
			}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "flash_write", strlen("flash_write")) == 0)
		{
				FLASH_ERROR_CODE_E result;
				uint8_t write_data[7]={8,1,2,3,4,5,6};
				
				result = flash_write(FLASH_USER_START_ADDR,write_data,7);
				if(result == FLASH_SUCCESS)
				{
					debug("flash_write success!\r\n");
				}
				else
				{
					debug("flash_write error!\r\n");
				}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "flash_erase", strlen("flash_erase")) == 0)
		{
			FLASH_ERROR_CODE_E result;
			result = flash_erase(FLASH_USER_START_ADDR);
			if(result == FLASH_SUCCESS)
			{
				debug("flash_erase success!\r\n");
			}
			else
			{
				debug("flash_erase fail!\r\n");
			}
		}
		else if (strncmp(dbg_cmd_split->arg[1], "erase_sector", strlen("erase_sector")) == 0)
		{
			uint8_t sec_cnt = 0;
			FLASH_ERROR_CODE_E result;
			if(dbg_cmd_split->arg_cnt != 3)
			{
				debug("param cnt error!\r\n");
				return ;
			}
			sec_cnt = *dbg_cmd_split->arg[2] - '0';
			result = flash_erase_sector(ADDR_2_SECTOR(FLASH_USER_START_ADDR),sec_cnt);
			if(result == FLASH_SUCCESS)
			{
				debug("erase %d sector success!(from %d to sector %d)\r\n",sec_cnt,ADDR_2_SECTOR(FLASH_USER_START_ADDR),ADDR_2_SECTOR(FLASH_USER_START_ADDR)+sec_cnt-1);
			}
			else
			{
					debug("erase sector error!\r\n");
			}
		}
}
 
/*---------------------------------------------------------------------------*/

