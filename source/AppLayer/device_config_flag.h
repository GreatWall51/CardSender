/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 device_config_flag.h
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/23
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/23, Liuwq create this file
*
******************************************************************************/
#ifndef _DEVICE_CONFIG_FLAG_H_
#define _DEVICE_CONFIG_FLAG_H_


/*------------------------------- Includes ----------------------------------*/


/*----------------------------- Global Defines ------------------------------*/
//与config_table中的项一一对应
typedef enum
{
	CONFIG_MODIFY=0,    /*表是否被修改,必须放在第一个，id为0*/
	CONFIG_ANTICOPY,    /*防复制参数，工程号，ADF,内部认证密钥号*/
	
	CONFIG_MAX,
} CONFIG_PARAMETER_E;

/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/



#endif //_DEVICE_CONFIG_FLAG_H_
