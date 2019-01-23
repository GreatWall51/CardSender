/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 uart_drv.c 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/05/16
* Notes: 
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/05/16, LiuWeiQiang create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "uart_drv.h"
#include "hal_com.h"


USART_TypeDef* COM_USART[USART_NUM] = {USART1}; 
GPIO_TypeDef*  COM_TX_PORT[USART_NUM] = {GPIOB};
GPIO_TypeDef*  COM_RX_PORT[USART_NUM] = {GPIOB};

const uint32_t COM_USART_CLK[USART_NUM] = {RCC_APB2Periph_USART1};

const uint32_t COM_TX_PORT_CLK[USART_NUM] = {RCC_AHBPeriph_GPIOB};
const uint32_t COM_RX_PORT_CLK[USART_NUM] = {RCC_AHBPeriph_GPIOB};

const uint16_t COM_TX_PIN[USART_NUM] = {GPIO_Pin_6};
const uint16_t COM_RX_PIN[USART_NUM] = {GPIO_Pin_7};

const uint8_t  COM_TX_PIN_SOURCE[USART_NUM] = {GPIO_PinSource6};
const uint8_t  COM_RX_PIN_SOURCE[USART_NUM] = {GPIO_PinSource7};

const uint8_t  COM_TX_AF[USART_NUM] = {GPIO_AF_0};
const uint8_t  COM_RX_AF[USART_NUM] = {GPIO_AF_0};


/* 波特率 */
const uint32_t COM_BAUD[USART_NUM] = {19200};
/* 使能发送中断 */
const FunctionalState  COM_S_IRQ_ENABLE[USART_NUM] = {ENABLE};
/* 发送中断类型 */
const uint32_t COM_S_IRQ_TYPE[USART_NUM] = {USART_IT_TC};

/* 使能接收中断 */
const FunctionalState  COM_R_IRQ_ENABLE[USART_NUM] = {ENABLE};
/* 接收中断类型 */
const uint32_t COM_R_IRQ_TYPE[USART_NUM] = {USART_IT_RXNE};
/* 中断通道 */
const uint8_t  COM_IRQ_Channel[USART_NUM] = {USART1_IRQn};
/* 中断优先级 */
const uint8_t  COM_IRQ_PRIO[USART_NUM] = {0};


/*------------------------ Variable Define/Declarations ----------------------*/


/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void usart_init(USART_TYPE COM, USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM], ENABLE);

  /* Enable USART clock */
  if(COM == USART_1)
  {
    RCC_APB2PeriphClockCmd(COM_USART_CLK[USART_1] , ENABLE); 
  }
  else if(COM == USART_2)
  {
    RCC_APB1PeriphClockCmd(COM_USART_CLK[USART_2], ENABLE);
  }
  /* Connect PXx to USARTx_Tx */
  GPIO_PinAFConfig(COM_TX_PORT[COM], COM_TX_PIN_SOURCE[COM], COM_TX_AF[COM]);

  /* Connect PXx to USARTx_Rx */
  GPIO_PinAFConfig(COM_RX_PORT[COM], COM_RX_PIN_SOURCE[COM], COM_RX_AF[COM]);
  
  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);
    
  /* Configure USART Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(COM_USART[COM], USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(COM_USART[COM], ENABLE);
}


void uart_drv_init(USART_TYPE com_num)
{
    USART_InitTypeDef USART_InitStructure;		
	NVIC_InitTypeDef 	NVIC_InitStructure;
   
	NVIC_InitStructure.NVIC_IRQChannel = COM_IRQ_Channel[com_num];
	NVIC_InitStructure.NVIC_IRQChannelPriority = COM_IRQ_PRIO[com_num];
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_InitStructure.USART_BaudRate = COM_BAUD[com_num];
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	usart_init(com_num, &USART_InitStructure);

	USART_ITConfig(COM_USART[com_num],COM_R_IRQ_TYPE[com_num],COM_R_IRQ_ENABLE[com_num]);		
    USART_ClearITPendingBit(COM_USART[com_num], COM_S_IRQ_TYPE[com_num]);
    USART_ITConfig(COM_USART[com_num],COM_S_IRQ_TYPE[com_num],COM_S_IRQ_ENABLE[com_num]);

}

void USART1_IRQHandler(void)
{
    //注意！不能使用if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)来判断
     if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)    
     {
          USART_ClearITPendingBit(USART1, USART_IT_ORE);//清除中断标志
         //USART_ReceiveData(USART1);
     }   
     if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
     {
         USART_ClearITPendingBit(USART1, USART_IT_TC);//清除中断标志 
         hal_com_send_irq_callback(PORT_CARD_SENDER);
//        USART_SendData(USART1, 0x12);  
     }
     /* 接收中断 */
     if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
     {   
         hal_com_receive_irq_callback(PORT_CARD_SENDER, USART_ReceiveData(USART1));
     }
}



/*---------------------------------------------------------------------------*/

