/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 flash_drv.h
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/17
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/17, Liuwq create this file
*
******************************************************************************/
#ifndef _FLASH_DRV_H_
#define _FLASH_DRV_H_


/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>

/*----------------------------- Global Defines ------------------------------*/
#define FLASH_BASE_ADDRESS        0x08000000

#define FLASH_PAGE_SIZE        ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_USER_START_ADDR  ((uint32_t)0x0800a000)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR    ((uint32_t)0x0800FFFF)   /* End @ of user Flash area */

/*----------------------------- Global Typedefs -----------------------------*/
typedef enum
{
	FLASH_SUCCESS = 0,
	FLASH_PARAM_ERROR,
	FLASH_ADDR_ERROR,
	FLASH_WRITE_WORD_ERROR,
	FLASH_WRITE_HALF_WORD_ERROR,
	FLASH_WRITE_BYTE_ERROR,
	FLASH_READ_ERROR,
	FLASH_ERASE_ERROR,
} FLASH_ERROR_CODE_E;

/*----------------------------- External Variables --------------------------*/

void flash_read_protect(void);
/*------------------------ Global Function Prototypes -----------------------*/
FLASH_ERROR_CODE_E flash_write ( uint32_t address, const uint8_t* paddr, uint32_t size );
uint32_t flash_read ( uint32_t address, uint8_t* paddr, uint32_t size );
FLASH_ERROR_CODE_E flash_erase ( uint32_t address );
FLASH_ERROR_CODE_E flash_erase_sector ( uint32_t sector_from, uint16_t sector_cnt );
#endif //_FLASH_DRV_H_
