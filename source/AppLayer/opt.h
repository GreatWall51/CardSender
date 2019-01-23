/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 opt.h 
* Desc:
* 
* 
* Author: 	 Liuwq
* Date: 	 2018/05/24
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2018/05/24, Liuwq create this file
* 
******************************************************************************/
#ifndef _OPT_H_     
#define _OPT_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include "debug_opt.h"
#include <stdlib.h>
 
/*----------------------------- Global Defines ------------------------------*/
//��ģ��
#define CARD_READ        1  //�Ƿ�ʹ�ܿ�����ģ��
#define CARD_DEBUG       0  //��ģ����Թ���
#define DEBUG_ISO14443A  0  //IC������
#define DEBUG_CPU        0  //CPU������
#define DEBUG_ISO14443B  0  //���֤����
#define DEBUG_CPU_SENDER 0  //CPU��������
#define DEBUG_IC_SENDER  1  //IC��������
/*----------------------------- Global Typedefs -----------------------------*/
 
#define C_MALLOC  malloc
#define C_FREE    free
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 
 

#endif //_OPT_H_
