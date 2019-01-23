/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 debug_device_config.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/25
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/25, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "debug.h"
#include "device_config.h"
/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
DEBUG_CMD_FN ( dev_config )
{
	dbg_cmd_split_t* dbg_cmd_split;

	dbg_cmd_split = get_dbg_cmd_split();
//    if (strncmp(dbg_cmd_split->arg[1], "get_config_change_evt", strlen("get_config_change_evt")) == 0)
//    {
//        uint8_t evt = 2;
//        uint32_t index;
//        const config_item_info_t* config_item_info=NULL;
//
//
//        if(dbg_cmd_split->arg_cnt != 3)
//        {
//            debug("param cnt error!\r\n");
//            return;
//        }
//        index = *dbg_cmd_split->arg[2] - '0';
//        evt = get_config_change_evt((CONFIG_PARAMETER_E)index);
//        config_item_info = get_config_list_item_info((CONFIG_PARAMETER_E)index);
//        if(config_item_info)
//        {
//        debug("get %s change evt = %d\r\n",config_item_info->config_name,evt);
//        }
//        else
//        {
//            debug("index error!\r\n");
//        }
//    }
//    else if (strncmp(dbg_cmd_split->arg[1], "set_config_change_evt", strlen("set_config_change_evt")) == 0)
//    {
//        uint8_t evt = 2;
//        uint32_t index;
//        const config_item_info_t* config_item_info=NULL;
//
//        if(dbg_cmd_split->arg_cnt != 4)
//        {
//            debug("param cnt error!\r\n");
//            return;
//        }
//        index = *dbg_cmd_split->arg[2] - '0';
//        evt = *dbg_cmd_split->arg[3] - '0';
//        evt = set_config_change_evt((CONFIG_PARAMETER_E)index,evt);
//        config_item_info = get_config_list_item_info((CONFIG_PARAMETER_E)index);
//        if(config_item_info)
//        {
//        debug("set %s change evt = %d\r\n",config_item_info->config_name,evt);
//        }
//        else
//        {
//            debug("index error!\r\n");
//        }
//    }
	if ( strncmp ( dbg_cmd_split->arg[1], "init_default", strlen ( "init_default" ) ) == 0 )
	{
		device_param_default_init();
		debug ( "init all parametet to default data!\r\n" );
	}
	else if ( strncmp ( dbg_cmd_split->arg[1], "set_param_sync_to_flash", strlen ( "set_param_sync_to_flash" ) ) == 0 )
	{
		set_param_sync_to_flash();
		debug ( "Synchronize the parameters to  flash!\r\n" );
	}
	else if ( strncmp ( dbg_cmd_split->arg[1], "config_get_data", strlen ( "config_get_data" ) ) == 0 )
	{
		uint8_t len = 0;
		uint16_t index = 0;
		uint8_t* read_data;
		if ( dbg_cmd_split->arg_cnt != 3 )
		{
			debug ( "param cnt error!\r\n" );
			return;
		}
		index = *dbg_cmd_split->arg[2] - '0';
		len = config_get_data ( ( CONFIG_PARAMETER_E ) index,&read_data );
		if ( len )
		{
			const config_item_info_t* config_item_info=NULL;

			config_item_info = get_config_list_item_info ( ( CONFIG_PARAMETER_E ) index );
			debug ( "config_get_data(addr = %p,index = %d,len = %d):\r\n",read_data,index,len );
			if ( config_item_info->data_type == DATA_TYPE_CHAR ||\
			        config_item_info->data_type == DATA_TYPE_NUMBER )
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_char ( read_data,len );
			}
			else if ( config_item_info->data_type == DATA_TYPE_IP || \
			          config_item_info->data_type == DATA_TYPE_BCD )
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_dec ( read_data,len );
			}
			else if (config_item_info->data_type == DATA_TYPE_HEX)
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_hex ( read_data,len );
			}
			else
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_hex ( read_data,len );
			}
		}
		else
		{
			debug ( "config_get_data fail!\r\n" );
		}
	}
	else if ( strncmp ( dbg_cmd_split->arg[1], "config_set_data", strlen ( "config_set_data" ) ) == 0 )
	{
		uint8_t len = 0;
		uint16_t index = 0;
		uint8_t write_data[32]= {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,05,05};
		if ( dbg_cmd_split->arg_cnt != 3 )
		{
			debug ( "param error!\r\n" );
			return;
		}
		index = *dbg_cmd_split->arg[2] - '0';
		len = config_set_data ( ( CONFIG_PARAMETER_E ) index,write_data );
		if ( len>2 )
		{
			const config_item_info_t* config_item_info=NULL;

			config_item_info = get_config_list_item_info ( ( CONFIG_PARAMETER_E ) index );
			len -= 2;
			debug ( "config_set_data(index = %d,len = %d):\r\n",index,len );
			if ( config_item_info->data_type == DATA_TYPE_CHAR )
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_char ( write_data,len );
			}
			else if ( config_item_info->data_type == DATA_TYPE_IP || \
			          config_item_info->data_type == DATA_TYPE_BCD || \
			          config_item_info->data_type == DATA_TYPE_NUMBER )
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_dec ( write_data,len );
			}
			else
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_hex ( write_data,len );
			}
		}
		else
		{
			debug ( "config_set_data fail!\r\n" );
		}
	}
	else if ( strncmp ( dbg_cmd_split->arg[1], "config_refresh_data", strlen ( "config_refresh_data" ) ) == 0 )
	{
		uint8_t len = 0;
		uint16_t index = 0;
		uint8_t write_data[32]= {172,16,1,166};
		if ( dbg_cmd_split->arg_cnt != 3 )
		{
			debug ( "param error!\r\n" );
			return;
		}
		index = *dbg_cmd_split->arg[2] - '0';
		len = config_refresh_data ( ( CONFIG_PARAMETER_E ) index,write_data );
		if ( len )
		{
			const config_item_info_t* config_item_info=NULL;

			config_item_info = get_config_list_item_info ( ( CONFIG_PARAMETER_E ) index );
			debug ( "config_refresh_data(index = %d,len = %d):\r\n",index,len );
			if ( config_item_info->data_type == DATA_TYPE_CHAR )
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_char ( write_data,len );
			}
			else if ( config_item_info->data_type == DATA_TYPE_IP || \
			          config_item_info->data_type == DATA_TYPE_BCD || \
			          config_item_info->data_type == DATA_TYPE_NUMBER )
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_dec ( write_data,len );
			}
			else
			{
				debug ( "%s = ",config_item_info->config_name );
				debug_hex ( write_data,len );
			}
		}
		else
		{
			debug ( "config_refresh_data fail!\r\n" );
		}
	}
	else
	{
		debug ( "unsupport function!\r\n" );
	}
}

/*---------------------------------------------------------------------------*/

