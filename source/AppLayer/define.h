/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 define.h
* Desc:
*
*
* Author: 	 liuwq
* Date: 	 2017/09/23
* Notes:
*
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/09/23, liuwq create this file
*
******************************************************************************/
#ifndef _DEFINE_H_
#define _DEFINE_H_


/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"

/*----------------------------- Global Typedefs -----------------------------*/
#define ERROR		1
#define	OK			0
#define TRUE        1
#define FALSE       0

#define sbi(ADDRESS,BIT) 	((ADDRESS->BSRR = BIT))	// Set bit
#define cbi(ADDRESS,BIT) 	((ADDRESS->BRR = BIT))	// Clear bit
#define	bis(ADDRESS,BIT)	(GPIO_ReadInputDataBit(ADDRESS,BIT))	// Is bit set?
#define	bic(ADDRESS,BIT)	(!GPIO_ReadInputDataBit(ADDRESS,BIT))	// Is bit clear
#define	toggle(ADDRESS,BIT)	(GPIO_WriteBit(ADDRESS,BIT,(BitAction)((1-GPIO_ReadOutputDataBit(ADDRESS,BIT)))))	// Toggle bit

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

/* takes a byte out of a uint32 : var - uint32,  ByteNum - byte to take out (0 - 3) */
#define BREAK_UINT32( var, ByteNum ) \
          (uint8_t)((uint32_t)(((var) >>((ByteNum) * 8)) & 0x00FF))

#define BUILD_UINT16(loByte, hiByte) \
          ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define UINT16_TO_UINT32(loByte, hiByte) \
        ((uint32_t)(((loByte) & 0x0000FFFF) + (((hiByte) & 0x0000FFFF) << 16)))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
          ((uint32_t)((uint32_t)((Byte0) & 0x00FF) \
          + ((uint32_t)((Byte1) & 0x00FF) << 8) \
          + ((uint32_t)((Byte2) & 0x00FF) << 16) \
          + ((uint32_t)((Byte3) & 0x00FF) << 24)))

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define MASK_8  (8-1)
#define MASK_16 (16-1)
#define MASK_32 (32-1)
#define SBIT_8(N) ((uint8_t)((uint8_t)(0x01)<<(N&MASK_8)))
#define SBIT_16(N) ((uint16_t)((uint16_t)(0x01)<<(N&MASK_16)))
#define SBIT_32(N) ((uint32_t)((uint32_t)(0x01)<<(N&MASK_32)))

typedef enum
{
	ERROR_NONE = 0,
	ERROR_DEFAULT,
	ERROR_PARAM,
} ERROR_CODE_E;
/*----------------------------- Global Defines ------------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/



#endif //_DEFINE_H_
