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
//��config_table�е���һһ��Ӧ
typedef enum
{
	CONFIG_MODIFY=0,    /*���Ƿ��޸�,������ڵ�һ����idΪ0*/
	CONFIG_ANTICOPY,    /*�����Ʋ��������̺ţ�ADF,�ڲ���֤��Կ��*/
	
	CONFIG_MAX,
} CONFIG_PARAMETER_E;

/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/



#endif //_DEVICE_CONFIG_FLAG_H_
