/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : hal_ctrl_io.h
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
#ifndef _HAL_CTRL_IO_H_
#define _HAL_CTRL_IO_H_
/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h" 
 
/*----------------------- Constant / Macro Definitions -----------------------*/
#define TIME_FLASH_SLOW (500)//����������� ms
#define TIME_FLASH (200)//����������� ms


typedef enum {
  CTRL_LED_RED = 0,//���
  CTRL_LED_GREEN,//�̵�
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
