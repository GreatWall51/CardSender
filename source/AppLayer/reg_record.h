/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 reg_record.h
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/19
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/07/19, Liuwq create this file
*
******************************************************************************/
#ifndef _REG_RECORD_H_
#define _REG_RECORD_H_


/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
#include "flash_drv.h"

/*----------------------------- Global Defines ------------------------------*/
#ifndef SBIT_16
#define SBIT_16(N) ((uint16_t)((uint16_t)(0x01)<<(N)))
#endif

/*----------------------------- Global Typedefs -----------------------------*/
__packed typedef struct
{
	uint16_t start_sector;	          /*!< ���ô洢����ʼҳ*/
	uint16_t back_sector;	          /*!<  ���ô洢��ı���ҳѭ���洢 */
	uint32_t max_index;	              /*!< �������ɴ洢����Ŀ��*/
	uint32_t items_cnt;	              /*!< �Ѵ洢����Ϣ��Ŀ�� */
	uint8_t cfg_item_size;	          /*!< ������Ϣ����,ʵ�������Ч�洢����Ϊcfg_item_size-2 */
	uint16_t cfg_per_page_item_cnt;   /*!< ÿҳ�ɴ洢����Ϣ��Ŀ*/
	uint8_t* index_use_array;	      /*!< ��Ŀд����������,��Ӧbitλ1->δʹ��,0->��ʹ��*/
	uint8_t* index_effictive_array;   /*!< ��Ŀ��Ч��������,��Ӧbitλ1->��Ч,0->��Ч */
} reg_base_t;

__packed typedef struct
{
	uint32_t offset;      /*!< ƥ����Ϣ��FLASH�е���ʼƫ��*/
	uint8_t match_len;    /*!< ƥ����Ϣ�ĳ���*/
	uint8_t* match_data;  /*!< ƥ����Ϣ����*/
} match_t;

typedef enum
{
	ITEM_SIZE_1      = SBIT_16 ( 0 ),
	ITEM_SIZE_2      = SBIT_16 ( 1 ),
	ITEM_SIZE_4      = SBIT_16 ( 2 ),
	ITEM_SIZE_8      = SBIT_16 ( 3 ),
	ITEM_SIZE_16     = SBIT_16 ( 4 ),
	ITEM_SIZE_32     = SBIT_16 ( 5 ),
	ITEM_SIZE_64     = SBIT_16 ( 6 ),
	ITEM_SIZE_128    = SBIT_16 ( 7 ),
	ITEM_SIZE_256    = SBIT_16 ( 8 ),
//	ITEM_SIZE_512    = SBIT_16 ( 9 ),
//	ITEM_SIZE_1024   = SBIT_16 ( 10 ),
} ITEM_SIZE_ENUM;
/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/

uint32_t reg_record_init ( reg_base_t* pReg, uint32_t base_addr, uint32_t cfg_max_items_cnt, ITEM_SIZE_ENUM cfg_item_size );
void reg_record_real_time ( reg_base_t* pReg );

uint8_t reg_record_del_all ( reg_base_t* pReg );
uint8_t reg_record_match_delete ( reg_base_t* pReg,match_t* match );
uint8_t reg_record_match_delete_all ( reg_base_t* pReg,match_t* match );

uint32_t  reg_record_get_effect_item_num ( reg_base_t* pReg );

uint16_t reg_record_get_match_item ( reg_base_t* pReg, match_t* match,const void* p_item );
uint16_t reg_record_get_match_item_all ( reg_base_t* pReg, match_t* match, const void* p_item );
uint32_t reg_record_get_match_item_cnt ( reg_base_t* pReg, match_t*  match );
uint16_t reg_record_get_match_item_actual_length ( reg_base_t* pReg,match_t* match );

uint16_t reg_record_match_write ( reg_base_t* pReg, match_t* match,const void* p_item,uint32_t len );



uint16_t reg_record_get_all_item_offset_len_data ( reg_base_t* pReg, match_t*  match, const void* p_item );
uint16_t reg_record_get_match_item_special_data ( reg_base_t* pReg, match_t*  match, match_t*  match_special,const void* p_item );

#endif //_REG_RECORD_H_
