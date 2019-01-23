/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : hal_ctrl_io.c
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

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "timer_drv.h"
#include "define.h" 
#include <string.h>
#include <stdlib.h>
#include "hal_ctrl_io.h"  
#include "ctrl_io_drv.h"  
 
/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Variable Define/Declarations ----------------------*/
/* 配置本程序的LED相关引脚，注意需同步修改LED_INDEX枚举值...added by Kimson, 2014-03-19 */
const CTRL_CONFIG_TABLE c_ctrlioConfigTable[CTRL_NUM] = 
{   
    {GPIOA, GPIO_Pin_3},     //led0 红
	{GPIOA, GPIO_Pin_2}, //绿
   
};  
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void ctrl_io_hw_init(void)
{
    uint32_t i;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* GPIOB Periph clock enable */
     RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | \
    RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD, ENABLE);	  

    /* Configure ALL PIN in output pushpull mode */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
    for(i = 0; i < sizeof(c_ctrlioConfigTable)/sizeof(c_ctrlioConfigTable[0]); i++)
    {
        GPIO_InitStructure.GPIO_Pin = c_ctrlioConfigTable[i].GPIO_Pin;
        GPIO_Init(c_ctrlioConfigTable[i].GPIOx, &GPIO_InitStructure);
    }
}


