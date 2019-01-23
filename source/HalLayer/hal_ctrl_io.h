/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : hal_ctrl_io.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年7月4日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年7月4日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/
#ifndef _HAL_CTRL_IO_H_
#define _HAL_CTRL_IO_H_
/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h" 
 
/*----------------------- Constant / Macro Definitions -----------------------*/
#define TIME_FLASH_SLOW (500)//慢闪间隔配置 ms
#define TIME_FLASH (200)//快闪间隔配置 ms


typedef enum {
  CTRL_LED_RED = 0,//红灯
  CTRL_LED_GREEN,//绿灯
  CTRL_NUM //this must <= 32
}CTRL_INDEX; 

/*------------------------ Variable Define/Declarations ----------------------*/
typedef struct {
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
}CTRL_CONFIG_TABLE;  

extern const CTRL_CONFIG_TABLE c_ctrlioConfigTable[CTRL_NUM];

#define CTRL_ON(ctrl_io_N) GPIO_ResetBits(c_ctrlioConfigTable[ctrl_io_N].GPIOx, c_ctrlioConfigTable[ctrl_io_N].GPIO_Pin)   //Led oN
#define CTRL_OFF(ctrl_io_N) GPIO_SetBits(c_ctrlioConfigTable[ctrl_io_N].GPIOx, c_ctrlioConfigTable[ctrl_io_N].GPIO_Pin)     //Led off 
#define IS_CTRL_ON(ctrl_io_N) (GPIO_ReadOutputDataBit(c_ctrlioConfigTable[ctrl_io_N].GPIOx, c_ctrlioConfigTable[ctrl_io_N].GPIO_Pin) == Bit_RESET)

/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void ctrl_io_hw_init(void);
#endif
