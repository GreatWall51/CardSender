/******************************************************************************
* Copyright 2018-2023 Renyucjs@163.com
* FileName: 	 debug_cmd_io.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/04/02
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/04/02, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "debug.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
//需要将此命令在DEBUG_CMD_DECLARE下声明，并DEBUG_CMD_TABLE下的debug_cmd_table注册

//在串口调试助手中输入test parameter1 parameter2 parameter3，其中test当命令解析
//arameter1为第一个参数，arameter2为第二个参数，arameter3为第三个参数
DEBUG_CMD_FN ( test )
{
	dbg_cmd_split_t* dbg_cmd_split;

	dbg_cmd_split = get_dbg_cmd_split();

	//参数个数
	debug ( "param cnt = %d\r\n",dbg_cmd_split->arg_cnt-1 );
	//解析第一个参数
	if ( strncmp ( dbg_cmd_split->arg[1], "parameter1", strlen ( "parameter1" ) ) == 0 )
	{
		debug ( "param 1 success!\r\n" );
		//解析第二个参数
		if ( strncmp ( dbg_cmd_split->arg[2], "parameter2", strlen ( "parameter2" ) ) == 0 )
		{
			debug ( "param 2 success!\r\n" );
			//解析第三个参数
			if ( strncmp ( dbg_cmd_split->arg[3], "parameter3", strlen ( "parameter3" ) ) == 0 )
			{
				debug ( "param 3 success!\r\n" );
			}
		}
	}
	return;
}
/*---------------------------------------------------------------------------*/

