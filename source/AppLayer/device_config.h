/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 device_config.h
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
#ifndef _DEVICE_CONFIG_H_
#define _DEVICE_CONFIG_H_


/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
#include "device_config_flag.h"
/*----------------------------- Global Defines ------------------------------*/
typedef enum
{
	DATA_TYPE_CHAR = 0,    /*字符串*/
	DATA_TYPE_IP,          /*IP*/
	DATA_TYPE_NUMBER,      /*号码*/
	DATA_TYPE_BCD,         /*数值 十进制*/
	DATA_TYPE_HEX,         /*数值 十六进制*/
	
	DATA_TYPE_UNABLE =   0xFF, /*不使能*/
} DATA_TYPE_E;

/*----------------------------- Global Typedefs -----------------------------*/


typedef struct _CONFIG_SET_STR
{
	char* config_name;       //预设变量名
	void* config_adder;      //变量地址
	DATA_TYPE_E data_type;   //数据类型
	uint8_t data_len;        //数据长度
} config_item_info_t;
//typedef struct _USER_INFO_T
//{
//	char name[12];
//	char phone[12];
////    char id[18];//配置的条目大小只有16，先不存
//	char home_id[12];
//	char pwd[12];
//} user_info_t;



//typedef struct _CONFIG_NET_T
//{
//	uint8_t ip[4];    /*IP地址*/
//	uint8_t mask[4]; /*子网掩码*/
//	uint8_t gw[4];    /*网关*/
//} CONFIG_NET_T;
/*
note:
(1)新加的配置项可在成员变量中间加入，但一定要在config_list中最后一个数组后添加。
(2)config_sys_param_t中增加的配置项要与config_list和CONFIG_PARAMETER_E同步，同时将新增加的配置项在
   sys_param_new_member_init中进行初始化。
*/
typedef struct _CONFIG_SYS_PARAM_STR
{
	uint32_t modify_flag;
	uint8_t work_mode;
	uint8_t anticopy[20];
//	CONFIG_NET_T net;
//	user_info_t user;//先配置一个人
} config_sys_param_t;
/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
void device_config_init ( void );
void device_config_real_time ( void );

const config_item_info_t* get_config_list_item_info ( CONFIG_PARAMETER_E index );
//uint8_t get_config_change_evt ( CONFIG_PARAMETER_E index );
//uint8_t set_config_change_evt(CONFIG_PARAMETER_E index,uint8_t val);

void device_param_default_init ( void );
uint8_t config_get_data ( CONFIG_PARAMETER_E index,uint8_t** data );
uint8_t config_set_data ( CONFIG_PARAMETER_E index,uint8_t* data );
uint8_t config_refresh_data ( CONFIG_PARAMETER_E index,uint8_t* data );
void set_param_sync_to_flash ( void );

#endif //_DEVICE_CONFIG_H_
