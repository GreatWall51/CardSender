/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : hal_ctrl_io.c
  �� �� ��   : ����
  ��    ��   : wzh
  ��������   : 2016��7��4��
  ����޸�   :
  ��������   : 
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��7��4��
    ��    ��   : wzh
    �޸�����   : �����ļ�

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
/* ���ñ������LED������ţ�ע����ͬ���޸�LED_INDEXö��ֵ...added by Kimson, 2014-03-19 */
const CTRL_CONFIG_TABLE c_ctrlioConfigTable[CTRL_NUM] = 
{   
    {GPIOA, GPIO_Pin_3},     //led0 ��
	{GPIOA, GPIO_Pin_2}, //��
   
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


