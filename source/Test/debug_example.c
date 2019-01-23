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
//��Ҫ����������DEBUG_CMD_DECLARE����������DEBUG_CMD_TABLE�µ�debug_cmd_tableע��

//�ڴ��ڵ�������������test parameter1 parameter2 parameter3������test����������p
//arameter1Ϊ��һ��������arameter2Ϊ�ڶ���������arameter3Ϊ����������
DEBUG_CMD_FN ( test )
{
	dbg_cmd_split_t* dbg_cmd_split;

	dbg_cmd_split = get_dbg_cmd_split();

	//��������
	debug ( "param cnt = %d\r\n",dbg_cmd_split->arg_cnt-1 );
	//������һ������
	if ( strncmp ( dbg_cmd_split->arg[1], "parameter1", strlen ( "parameter1" ) ) == 0 )
	{
		debug ( "param 1 success!\r\n" );
		//�����ڶ�������
		if ( strncmp ( dbg_cmd_split->arg[2], "parameter2", strlen ( "parameter2" ) ) == 0 )
		{
			debug ( "param 2 success!\r\n" );
			//��������������
			if ( strncmp ( dbg_cmd_split->arg[3], "parameter3", strlen ( "parameter3" ) ) == 0 )
			{
				debug ( "param 3 success!\r\n" );
			}
		}
	}
	return;
}
/*---------------------------------------------------------------------------*/

