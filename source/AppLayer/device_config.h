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
	DATA_TYPE_CHAR = 0,    /*�ַ���*/
	DATA_TYPE_IP,          /*IP*/
	DATA_TYPE_NUMBER,      /*����*/
	DATA_TYPE_BCD,         /*��ֵ ʮ����*/
	DATA_TYPE_HEX,         /*��ֵ ʮ������*/
	
	DATA_TYPE_UNABLE =   0xFF, /*��ʹ��*/
} DATA_TYPE_E;

/*----------------------------- Global Typedefs -----------------------------*/


typedef struct _CONFIG_SET_STR
{
	char* config_name;       //Ԥ�������
	void* config_adder;      //������ַ
	DATA_TYPE_E data_type;   //��������
	uint8_t data_len;        //���ݳ���
} config_item_info_t;
//typedef struct _USER_INFO_T
//{
//	char name[12];
//	char phone[12];
////    char id[18];//���õ���Ŀ��Сֻ��16���Ȳ���
//	char home_id[12];
//	char pwd[12];
//} user_info_t;



//typedef struct _CONFIG_NET_T
//{
//	uint8_t ip[4];    /*IP��ַ*/
//	uint8_t mask[4]; /*��������*/
//	uint8_t gw[4];    /*����*/
//} CONFIG_NET_T;
/*
note:
(1)�¼ӵ���������ڳ�Ա�����м���룬��һ��Ҫ��config_list�����һ���������ӡ�
(2)config_sys_param_t�����ӵ�������Ҫ��config_list��CONFIG_PARAMETER_Eͬ����ͬʱ�������ӵ���������
   sys_param_new_member_init�н��г�ʼ����
*/
typedef struct _CONFIG_SYS_PARAM_STR
{
	uint32_t modify_flag;
	uint8_t work_mode;
	uint8_t anticopy[20];
//	CONFIG_NET_T net;
//	user_info_t user;//������һ����
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
