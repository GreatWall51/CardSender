/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 flash_drv.c
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


/*------------------------------- Includes ----------------------------------*/
#include "flash_drv.h"
#include "debug_opt.h"
#include "stm32f0xx.h"
#include "define.h"
#include <stdio.h>
#if   DEBUG_FLASH_DRV
#include "debug.h"
#endif
/*------------------- Global Definitions and Declarations -------------------*/

#ifndef BUILD_UINT32
#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3)   \
          ((uint32_t)((uint32_t)((Byte0) & 0x00FF)     \
          + ((uint32_t)((Byte1) & 0x00FF) << 8)        \
          + ((uint32_t)((Byte2) & 0x00FF) << 16)       \
          + ((uint32_t)((Byte3) & 0x00FF) << 24)))
#endif

#ifndef BUILD_UINT16
#define BUILD_UINT16(loByte, hiByte) \
          ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))
#endif
/*---------------------- Constant / Macro Definitions -----------------------*/
#define FLASH_ADDR_2_SECTOR(ADDR) ((ADDR)/FLASH_PAGE_SIZE)
#define FLASH_SECTOR_2_ADDR(SECTOR) ((SECTOR) * FLASH_PAGE_SIZE)

/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/

/******************************************************************************
* Name: 	 flash_write
*
* Desc:       	 ��ָ��FLASHλ��д������
* Param(in):  	 address  -------д��FLASH����ʼ��ַ(����Ϊ4��������)
                 paddr    -------д���ʵ������
                 size     -------д�����ݵĳ���
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/17, Create this function by Liuwq
 ******************************************************************************/
FLASH_ERROR_CODE_E  flash_write ( uint32_t address, const uint8_t* paddr, uint32_t size )
{
	uint32_t end_addr = 0;
	uint32_t start_addr;
	uint16_t page_num;
	uint32_t word_num;
	uint8_t half_word_num;
	uint8_t byte_num;
	uint32_t write_cnt = 0;
	FLASH_Status result;

	/*������Ч�Լ�� */
	if ( size < 1 || paddr == NULL )
	{
		return FLASH_ADDR_ERROR;
	}

	/*����ʹ�õ�ҳ��*/
	page_num = size / FLASH_PAGE_SIZE;
	/*��ַ��Ч�Լ�� */
//   start_addr = address+ FLASH_BASE_ADDRESS;
	start_addr = address;
	end_addr = ( start_addr + ( page_num * FLASH_PAGE_SIZE ) );
#if DEBUG_FLASH_DRV
	debug ( "flash_write addr = %x\r\n",start_addr );
#endif
	if ( start_addr < FLASH_USER_START_ADDR || end_addr > FLASH_USER_END_ADDR )
	{
		return FLASH_ADDR_ERROR;
	}

	/*�������������������ֽ���*/
	word_num = size >> 2;
	half_word_num = ( size % 4 ) >> 1;
	byte_num = size % 2;

	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag ( FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR );

	/*���ֵķ�ʽд��*/
	write_cnt = 0;
	while ( write_cnt < word_num )
	{
		result = FLASH_ProgramWord ( start_addr, BUILD_UINT32 ( * ( paddr ), * ( paddr + 1 ), * ( paddr + 2 ), * ( paddr + 3 ) ) );
		if ( result == FLASH_COMPLETE )
		{
			start_addr = start_addr + 4;
			paddr = paddr + 4;
			write_cnt++;
		}
		else
		{
#if   DEBUG_FLASH_DRV
			debug ( "flash_write>>>flash write word fail,error code:%x.\r\n", result );
#endif
			return FLASH_WRITE_WORD_ERROR;
		}
	}

	/*�����ֵķ�ʽд��*/
	write_cnt = 0;
	while ( write_cnt < half_word_num )
	{
		result = FLASH_ProgramHalfWord ( start_addr, BUILD_UINT16 ( * ( paddr ), * ( paddr + 1 ) ) );
		if ( result == FLASH_COMPLETE )
		{
			start_addr = start_addr + 2;
			paddr = paddr + 2;
			write_cnt++;
		}
		else
		{
#if   DEBUG_FLASH_DRV
			debug ( "flash_write>>>flash write half-word fail,error code:%x.\r\n", result );
#endif
			return FLASH_WRITE_HALF_WORD_ERROR;
		}
	}

	/*���ֽڵķ�ʽд��*/
	if ( byte_num )
	{
		result = FLASH_ProgramHalfWord ( start_addr, BUILD_UINT16 ( * ( paddr ), 0xFFFF ) );
		if ( result == FLASH_COMPLETE )
		{
			start_addr = start_addr + 2;
			paddr = paddr + 2;
		}
		else
		{
#if   DEBUG_FLASH_DRV
			debug ( "flash_write>>>flash write byte fail,error code:%x.\r\n", result );
#endif
			return FLASH_WRITE_BYTE_ERROR;
		}
	}

	/* Lock the Flash to disable the flash control register access (recommended
	   to protect the FLASH memory against possible unwanted operation) *********/
	FLASH_Lock();

	return FLASH_SUCCESS;
}

/******************************************************************************
* Name: 	 flash_read
*
* Desc:
* Param(in):  	 address  -------��ȡFLASH����ʼ��ַ
                 size     -------����ȡ�ĳ���(��λ:�ֽ�)
* Param(out): 	 paddr    -------��ȡ��ʵ������(paddr�Ŀռ��ɵ��������룬�ռ䲻��С��size���ֽ�)
* Return:     	 ʵ�ʶ������ֽ���
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/17, Create this function by Liuwq
 ******************************************************************************/
uint32_t flash_read ( uint32_t address, uint8_t* paddr, uint32_t size )
{
	uint32_t read_index = 0;
	uint8_t value;
	uint32_t start_addr;
	uint32_t end_addr;


	/*������Ч�Լ�� */
	if ( paddr == NULL || size < 1 )
	{
		return 0;//FLASH_PARAM_ERROR;
	}
	/*��ַ��Ч�Լ�� */
//    start_addr = address+FLASH_BASE_ADDRESS;
	start_addr = address;
	end_addr = start_addr + size;
	if ( start_addr < FLASH_USER_START_ADDR || end_addr > FLASH_USER_END_ADDR )
	{
		return 0;//FLASH_ADDR_ERROR;
	}

	/*���ֽڷ�ʽ��ȡ����*/
	read_index = 0;
	while ( read_index < size )
	{
		value = * ( __IO uint8_t* ) start_addr;
		start_addr = start_addr + 1;
		* ( paddr + read_index ) = value;
		read_index++;
	}
	return read_index;
}

/*****************************************************************************
 �� �� ��  : flash_erase_by_sector_addr
 ��������  : ����ָ��FLASHλ�õ�ҳ
 �������  : uint32_t start_addr  ��ַ
 �������  : ��
 �� �� ֵ  : uint8_t
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��21��
    ��    ��   : lichp
    �޸�����   : �����ɺ���

*****************************************************************************/
FLASH_ERROR_CODE_E flash_erase ( uint32_t address )
//FLASH_ERROR_CODE_E flash_erase( uint32_t address )
{
	FLASH_Status result = FLASH_BUSY;
	uint32_t start_addr;

//		start_addr = address+FLASH_BASE_ADDRESS;
	start_addr = address;
	if ( start_addr < FLASH_USER_START_ADDR || start_addr > FLASH_USER_END_ADDR )
	{
		return FLASH_ADDR_ERROR;
	}

	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag ( FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR );
	result = FLASH_ErasePage ( start_addr );
	if ( result != FLASH_COMPLETE )
	{
#if   DEBUG_FLASH_DRV
		debug ( "flash_erase>>>flash erase fail,error code:%x.\r\n", result );
#endif
		return FLASH_ERASE_ERROR;
	}
	FLASH_Lock();

	return FLASH_SUCCESS;
}
FLASH_ERROR_CODE_E flash_erase_sector ( uint32_t sector_from, uint16_t sector_cnt )
{
	uint32_t addr;
	uint32_t sector_to;

	if ( sector_cnt == 0 )
	{
		return FLASH_PARAM_ERROR;
	}

	sector_to = sector_from + sector_cnt;
	while ( sector_from < sector_to )
	{
		FLASH_ERROR_CODE_E result = FLASH_PARAM_ERROR;
		addr = FLASH_SECTOR_2_ADDR ( sector_from );
		result = flash_erase ( addr + FLASH_BASE_ADDRESS );
		if ( result != FLASH_SUCCESS )
		{
			return FLASH_ERASE_ERROR;
		}
		sector_from++;
	}
	return FLASH_SUCCESS;
}

//���������
void flash_read_protect(void)
{
   if(SET != FLASH_OB_GetRDP())
   {
      FLASH_Unlock();
      FLASH_OB_Unlock();
      FLASH_OB_RDPConfig(OB_RDP_Level_1);
      FLASH_OB_Launch();
      FLASH_OB_Lock();    
      FLASH_Lock();
   }
}
/*---------------------------------------------------------------------------*/
