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
//卡模块
#define CARD_READ        1  //是否使能卡功能模块
#define CARD_DEBUG       0  //卡模块调试功能
#define DEBUG_ISO14443A  0  //IC卡调试
#define DEBUG_CPU        0  //CPU卡调试
#define DEBUG_ISO14443B  0  //身份证调试
#define DEBUG_CPU_SENDER 0  //CPU发卡调试
#define DEBUG_IC_SENDER  1  //IC发卡调试
/*----------------------------- Global Typedefs -----------------------------*/
 
#define C_MALLOC  malloc
#define C_FREE    free
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 
 

#endif //_OPT_H_
