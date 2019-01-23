/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 reg_record.c
* Desc:      flash��¼������������
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/19
* Notes:	 flash�����ݴ洢�ṹ
            -----------------------------------------------------------------
            | Byte0 | Byte1 | Byte2 |               ...                     |
            -----------------------------------------------------------------
            |  LEN  |  SUM  |          item_data(match info)...             |
            -----------------------------------------------------------------
˵��:(1)LEN��SUM�������ֶ�Ҳ������MASK(0xff,0xff��ʾ����;0x00,0x00��ʾ����)
     (2)match infoλ��item����������,����match->offset��ƫ��,match->match_lenָ������
��ֲ˵��: (1)���ⲿ�ṩ���½ӿ�
          C_FREE
          C_MALLOC
          uint32_t flash_read ( uint32_t address, uint8_t* paddr, uint32_t size );
          FLASH_ERROR_CODE_E  flash_write ( uint32_t address, const uint8_t* paddr, uint32_t size );
          FLASH_ERROR_CODE_E flash_erase_sector(uint32_t sector_from, uint16_t sector_cnt);
          (2)���ṩ���º궨��
          FLASH_PAGE_SIZE
          FLASH_USER_END_ADDR
* ----------------------------------------------------------------------------
* Histroy: v1.0   2018/07/19, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include <string.h>
#include "reg_record.h"
#include "opt.h"
#if   DEBUG_REG_RECORD
#include "debug.h"
#endif
/*------------------- Global Definitions and Declarations -------------------*/

#define reg_nor_read(address, buf, len)            flash_read(address+FLASH_BASE_ADDRESS, buf, len)
#define reg_nor_write(address, buf, len)           flash_write(address+FLASH_BASE_ADDRESS, buf, len)
#define reg_nor_erase(sector_from,sector_cnt)      flash_erase_sector(sector_from,sector_cnt)

#define CFG_FLASH_SECTOR_SIZE    FLASH_PAGE_SIZE         //ҳ�Ĵ�С1024Byte
#define CFG_FLASH_SIZE           (FLASH_USER_END_ADDR+1) //FLASH ����ַ */

#define MASK_INFO_LEN 2   /*��Ϣͷ������*/
static const uint8_t mask_info_unused[MASK_INFO_LEN] = {0xFF, 0xFF};  //��Ŀδʹ�ñ��
static const uint8_t mask_info_discard[MASK_INFO_LEN] = {0x00, 0x00}; //��Ŀ�������
/*---------------------- Constant / Macro Definitions -----------------------*/
#define ADDR_2_SECTOR(ADDR) ((ADDR)/CFG_FLASH_SECTOR_SIZE)
#define SECTOR_2_ADDR(SECTOR) ((SECTOR) * CFG_FLASH_SECTOR_SIZE)

/*----------------------- Type Declarations ---------------------------------*/
typedef struct
{
	uint8_t  len;                   /*!< ��Ŀʵ����Ч���ݳ���*/
	uint8_t  check_sum;             /*!< У���*/
	uint8_t  data;                  /*!< ������*/
} item_head_t;

/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name: 	 get_check_data_sum
*
* Desc:       	 ����У��ͣ���������������
* Param(in):  	 point:��У�����ݣ�len����У�����ݳ���
* Param(out):
* Return:     	 �����
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
static uint8_t  get_check_data_sum ( uint8_t* point, uint16_t len )
{
	uint8_t* pbuf;
	uint16_t i;
	uint8_t sum = 0;

	if ( point == NULL || len < 1 )
	{
		return 0;
	}
	pbuf = ( uint8_t* ) point;

	for ( i = 0; i < len; i++ )
	{
		sum += * ( pbuf + i );
	}
	return ( sum );
}
/******************************************************************************
* Name: 	 read_config
*
* Desc:       	 ��ȡ�������Ϣ
* Param(in):  	 pReg:����ȡ������Ϣ�ı�
* Param(out): 	 pReg:���������Ϣ�ı�
* Return:     	 0->success,>0->fail
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
static uint8_t read_config ( reg_base_t* pReg )
{
	uint16_t i, j, k;
	uint8_t now_sector;    //����ǰFLASH�����ҳ
	uint8_t mask[MASK_INFO_LEN] = {0};//�������ݣ������ж����ݵ���Ч�Լ�ʹ�����
	uint8_t* flash_match_data;
	uint8_t* flash_item_data;
	uint16_t first;

	if ( pReg == NULL )
	{
		return 1;
	}
	/*��ʼ����Ŀ��������*/
	memset ( pReg->index_effictive_array, 0xFF, ( pReg->max_index >> 3 ) + 1 );
	memset ( pReg->index_use_array, 0xFF, ( pReg->max_index >> 3 ) + 1 );

	/*��ȡFLASH��Ŀʹ��״̬�������������飬�����հ�ҳ��Ϊ����ҳ*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, mask, \
		               sizeof ( mask ) );
		//��������ҳ
		if ( memcmp ( mask, mask_info_unused,sizeof ( mask ) ) == 0 )
		{
			if ( ! ( i % pReg->cfg_per_page_item_cnt ) )
			{
				pReg->back_sector = ( i / pReg->cfg_per_page_item_cnt ) + pReg->start_sector;
			}
		}
		//������������
		else if ( memcmp ( mask, mask_info_discard,sizeof ( mask ) ) == 0 )
		{
			pReg->index_effictive_array[i >> 3] &= ~ ( 0x80 >> ( i & 0x07 ) );
			pReg->index_use_array[i >> 3] &= ~ ( 0x80 >> ( i & 0x07 ) );
		}
		else
		{
			pReg->index_use_array[i >> 3] &= ~ ( 0x80 >> ( i & 0x07 ) );
		}
	}
	/*������������ҳ */
	if ( pReg->back_sector == 0 )
	{
		/*������������ҳ����Ѱ����ǰ����FLASH�����ҳ�Լ�����ǰ�ı���ҳ��һ�������ҵ���i��С��max_index*/
		for ( i = 0; i < pReg->max_index; i++ )
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, mask, \
			               sizeof ( mask ) );
			if ( memcmp ( mask, mask_info_unused, sizeof ( mask ) ) == 0 )
			{
				break;
			}
		}
		flash_item_data = ( uint8_t* ) C_MALLOC ( pReg->cfg_item_size );
		if ( !flash_item_data )
		{
			return 1;
		}
		flash_match_data = ( uint8_t* ) C_MALLOC ( pReg->cfg_item_size );
		if ( !flash_match_data )
		{
			C_FREE ( flash_item_data );
			return 1;
		}
		reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + ( i - 1 ) *pReg->cfg_item_size, flash_item_data, \
		               pReg->cfg_item_size );
		for ( j = 0; j < pReg->max_index; j++ )
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + j * pReg->cfg_item_size, flash_match_data, \
			               pReg->cfg_item_size );

			if ( ( memcmp ( flash_item_data, flash_match_data,  pReg->cfg_item_size ) == 0 ) && ( j != ( i - 1 ) ) )
			{
				break;
			}
		}

		/*��ɵ���ǰFLASH������*/
		now_sector = j / pReg->cfg_per_page_item_cnt + pReg->start_sector;
		first = j;
		for ( k = ( j % pReg->cfg_per_page_item_cnt ) + 1; k < pReg->cfg_per_page_item_cnt; k++ )
		{
			j++;
			if ( ( pReg->index_effictive_array[j >> 3] & ( 0x80 >> ( j & 0x07 ) ) ) != 0 )
			{
				reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + j * pReg->cfg_item_size, flash_item_data, \
				               pReg->cfg_item_size );
				reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, flash_item_data, \
				                flash_item_data[0] + MASK_INFO_LEN );
				pReg->index_use_array[i >> 3] &= ~ ( 0x80 >> ( i & 0x07 ) );
				i++;
			}
		}
		reg_nor_erase ( now_sector, 1 );
		pReg->back_sector = now_sector;
		first = first - first % pReg->cfg_per_page_item_cnt;
		for ( i = 0; i < ( pReg->cfg_per_page_item_cnt >> 3 ); i++ )
		{
			pReg->index_use_array[ ( first >> 3 ) + i] = 0xFF;
			pReg->index_effictive_array[ ( first >> 3 ) + i] = 0xFF;
		}

		C_FREE ( flash_item_data );
		C_FREE ( flash_match_data );
	}
	return 0;
}

/******************************************************************************
* Name: 	 reg_record_del_all
*
* Desc:       	 ��ձ�
* Param(in):
* Param(out):
* Return:     	 0 -> success,
								 1 -> parameter error,
								 2 -> erase sector error,
* Global:
* Note:      ���flash����Ϣ
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint8_t reg_record_del_all ( reg_base_t* pReg )
{
	uint8_t page_max_num,result = 1;

	if ( pReg == NULL )
	{
		return 1;
	}
	page_max_num = pReg->max_index / pReg->cfg_per_page_item_cnt;
	result = reg_nor_erase ( pReg->start_sector, page_max_num );
	if ( result != 0 )
	{
		return 2;
	}
	pReg->items_cnt = 0;

	memset ( pReg->index_effictive_array, 0xFF, ( pReg->max_index >> 3 ) + 1 );
	memset ( pReg->index_use_array, 0xFF, ( pReg->max_index >> 3 ) + 1 );
	return 0;
}
/******************************************************************************
* Name: 	 reg_record_init
*
* Desc:       	 ��ָ��Flash��base_addrλ�ô�����һ�Ż������ñ�����ٿɴ洢:
                 cfg_max_items_cnt����������Ϊcfg_item_size ��С����Ŀ
* Param(in):  	 pReg:���ڴ洢�������������Ϣ���洢�ռ䱻Ԥ�ȷ��䣬���ڸú����ڱ���ʼ��
                 base_addr    �洢�����ʼ��ַ
                 cfg_max_items_cnt   �ñ������ܴ洢����Ŀ��
                 cfg_item_size        ������Ŀ��С
* Param(out): 	 �洢��Ľ�����ַ,0->error
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint32_t reg_record_init ( reg_base_t* pReg, uint32_t base_addr, uint32_t cfg_max_items_cnt, ITEM_SIZE_ENUM cfg_item_size )
{

	uint32_t max_size;//�ܴ洢�ռ�
	uint16_t i, page_max_num, count;
	uint32_t end_addr;
	uint8_t result = 1;

	if ( pReg == NULL || cfg_item_size > ITEM_SIZE_256 )
	{
		return 0;
	}

	/*��ʼ����ʼҳ����Ĺ̶����ò���*/
	memset ( pReg, 0, sizeof ( reg_base_t ) );

	pReg->start_sector  = ADDR_2_SECTOR ( base_addr );
	pReg->cfg_item_size = ( uint16_t ) cfg_item_size;
	max_size = cfg_item_size * cfg_max_items_cnt;
	if ( max_size % CFG_FLASH_SECTOR_SIZE )
	{
		page_max_num =  max_size / CFG_FLASH_SECTOR_SIZE + 2;
	}
	else
	{
		page_max_num =  max_size / CFG_FLASH_SECTOR_SIZE + 1;
	}
	end_addr = page_max_num * CFG_FLASH_SECTOR_SIZE + base_addr - 1;
	if ( end_addr > CFG_FLASH_SIZE )
	{
		return 0;
	}
	pReg->max_index = ( end_addr - base_addr + 1 ) / cfg_item_size;
	pReg->cfg_per_page_item_cnt = CFG_FLASH_SECTOR_SIZE / cfg_item_size;

	/*Ϊ�����������ռ�*/
	pReg->index_effictive_array = C_MALLOC ( ( pReg->max_index >> 3 ) + 1 );
	if ( pReg->index_effictive_array == NULL )
	{
		return 0;
	}
	// debug_cmd("pReg->max_index=%d\r\n",pReg->max_index);
	pReg->index_use_array = C_MALLOC ( ( pReg->max_index >> 3 ) + 1 );
	if ( pReg->index_use_array == NULL )
	{
		C_FREE ( pReg->index_effictive_array );
		return 0;
	}
	count = 0;

	/*��ʼ��FLASH��Ŀʹ��������������鼰����ҳ*/
	result = read_config ( pReg );
	if ( result != 0 )
	{
		return 0;
	}

	/*�����Ѵ洢����Ŀ��*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
		{
			count++;
		}
	}
	pReg->items_cnt = count;

	return  end_addr;
}

/******************************************************************************
* Name: 	 reg_record_put_item
*
* Desc:       	 ��ָ���Ļ����洢�������һ������Ŀ(��Ŀ��󳤶�ΪpReg->cfg_item_size)
* Param(in):  	 pReg -- ָ�򻬶��洢��������Ϣ���ѱ�reg_record_init��ʼ����
                 p_item -- ָ��Ҫ�洢����Ŀ����(��Ŀ��󳤶�ΪpReg->cfg_item_size)
* Param(out):
* Return:     	 ����洢����Ŀ��,>=success,=0puterror,=-1table full
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
static int reg_record_put_item ( reg_base_t* pReg, const uint8_t* p_item )
{
	uint16_t i, k, first;
	uint8_t* flash_item_data;
	uint16_t now_sector;

	if ( pReg == NULL || p_item == NULL )
	{
		return 0;
	}
	/*�����������飬�ҵ����ڱ���ҳ��δ��ռ�õ�FLASH��Ŀ�ռ�*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 ) && \
		        ( ( ( i / pReg->cfg_per_page_item_cnt ) + pReg->start_sector ) != pReg->back_sector ) )
		{
			break;
		}
	}
	/*�ҵ��ɴ洢�Ŀռ䣬ֱ��д�룬������pReg->index_use_array����*/
	if ( i != pReg->max_index )
	{
		uint8_t result = 1;
		result = reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, \
		                         p_item, *p_item + MASK_INFO_LEN );
		if ( result != 0 )
		{
			return 0;
		}
		pReg->index_use_array[i >> 3] &= ~ ( 0x80 >> ( i & 0x07 ) );
		pReg->items_cnt ++;
	}

	/*�Ҳ����ɴ洢�Ŀռ䣬����FLASH������д��*/
	else
	{
#if   DEBUG_REG_RECORD
		debug ( "arrange flash to put new item...!\r\n" );
#endif
		// DEBUG_TOP("flash clean\r\n");
		/*����pReg->index_effictive_array�������飬�ҵ������Ŀɱ������FLASH��Ŀ*/
		for ( i = 0; i < pReg->max_index; i++ )
		{
			if ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
			{
				break;
			}
		}

		/*�Ҳ����ɱ��������Ŀ����ȷʵ�Ѿ�ʵ����Ч�Ĵ��������ϲ�Ӧ�ý����ͷţ�����д��ʧ��*/
		if ( i == pReg->max_index )
		{
#if   DEBUG_REG_RECORD
			debug ( "flash restory effect item full,arrange flash fail!!!!\r\n" );
#endif
			return -1;
		}

		/*�ҵ��ɱ��������Ŀ������FLASH����*/
		flash_item_data = ( uint8_t* ) C_MALLOC ( pReg->cfg_item_size );
		if ( !flash_item_data )
		{
			return 0;
		}
#if   DEBUG_REG_RECORD
		debug ( "remove effect item to back sector...\r\n" );
#endif
		/*���㽫�������ҳ��*/
		now_sector = i / pReg->cfg_per_page_item_cnt + pReg->start_sector;
		i = i - i % pReg->cfg_per_page_item_cnt;//���㱻�����ҳ����ʼ��Ŀ���
		k = ( pReg->back_sector - pReg->start_sector ) * pReg->cfg_per_page_item_cnt; //���㱸��ҳ����ʼ��Ŀ���

		/*�������ҳ����Ч����Ϣ���Ƶ�����ҳ*/
		for ( first = i; first < i + pReg->cfg_per_page_item_cnt; first++ )
		{
			if ( ( pReg->index_effictive_array[first >> 3] & ( 0x80 >> ( first & 0x07 ) ) ) != 0 ) //��Ч����
			{
				uint32_t len = 0,result = 1;
				len = reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + first * pReg->cfg_item_size, flash_item_data, \
				                     pReg->cfg_item_size );
				if ( len == 0 ) //������
				{
					C_FREE ( flash_item_data );
					return 0;
				}
				result = reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + k * pReg->cfg_item_size, flash_item_data, \
				                         flash_item_data[0] + MASK_INFO_LEN );
				if ( result != 0 ) //д����
				{
					C_FREE ( flash_item_data );
#if   DEBUG_REG_RECORD
					debug ( "remove effect item to back sector fail!!!!!\r\n" );
#endif
					return 0;
				}
				pReg->index_use_array[k >> 3] &= ~ ( 0x80 >> ( k & 0x07 ) );
				k++;
				pReg->items_cnt++;
			}
		}
#if   DEBUG_REG_RECORD
		debug ( "arrange flash success!\r\n" );
#endif
		/*�ڱ���ҳ�Ŀ��ÿռ���д���д�������*/
		reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + k * pReg->cfg_item_size, \
		                p_item, *p_item + MASK_INFO_LEN );
		pReg->index_use_array[k >> 3] &= ~ ( 0x80 >> ( k & 0x07 ) );
		pReg->items_cnt++;

		/*��������ҳ���������ҳ����Ϊ�µı���ҳ����������������pReg->index_use_array��pReg->index_effictive_array*/
		reg_nor_erase ( now_sector, 1 );
		pReg->back_sector = now_sector;
		for ( first = 0; first < ( pReg->cfg_per_page_item_cnt >> 3 ); first++ )
		{
			pReg->index_use_array[ ( i >> 3 ) + first] = 0xFF;
			pReg->index_effictive_array[ ( i >> 3 ) + first] = 0xFF;
		}
		pReg->items_cnt -= pReg->cfg_per_page_item_cnt;
		C_FREE ( flash_item_data );
	}

	return pReg->items_cnt;
}
/******************************************************************************
* Name: 	 reg_record_match_write
*
* Desc:       	 �����д������
* Param(in):  	 pReg:   ָ���ı�
                 match:  ƥ�����Ϣ(NULL������ƥ��,�ҵ���λ�þ�ֱ��д�룬
                                  !=NULLɾ������ƥ����Ϣ��������д��һ��)
                 p_item: ��д�������
                 len:    ��д�����ݵ���Ч����
* Param(out):
* Return:     	 д��ĳ��ȣ�������Ϊ0��д��ʧ��
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint16_t reg_record_match_write ( reg_base_t* pReg, match_t* match, const void* p_item, uint32_t len )
{
	uint8_t* flash_item_data = NULL;
	uint8_t* start;
	item_head_t* item_head;

	if ( pReg == NULL || p_item == NULL || len <1 )
	{
		return 0;
	}
	/*����Ƿ���Ҫƥ���д�룬��Ҫ����ɾ����������ƥ����Ϣ����Ŀ���ٽ���Ŀд��(ֻдһ��)*/
	if ( match != NULL )
	{
		reg_record_match_delete_all ( pReg, match );
	}
	start = ( uint8_t* ) p_item;
	flash_item_data = ( uint8_t* ) C_MALLOC ( pReg->cfg_item_size );
	if ( !flash_item_data )
	{
		return 0;
	}

	/*����д��ĳ����Ƿ񳬹����õĵ����洢��Ŀ��С*/
	if ( len <= ( pReg->cfg_item_size - MASK_INFO_LEN ) )
	{
		memset ( flash_item_data, 0xFF, pReg->cfg_item_size );

		/*����д����Ŀ��ͷ����Ϣ*/
		item_head = ( item_head_t* ) flash_item_data;
		item_head->len =  len;
		memcpy ( &item_head->data, start, item_head->len );
		item_head->check_sum = get_check_data_sum ( &item_head->data, item_head->len );
		if ( reg_record_put_item ( pReg, flash_item_data ) <= 0 )
		{
			C_FREE ( flash_item_data );
			return 0;
		}
	}
	else
	{
		C_FREE ( flash_item_data );
		return 0;
	}

	C_FREE ( flash_item_data );
	return len;
}
/******************************************************************************
* Name: 	 reg_record_get_match_item
*
* Desc:       	 ��ȡƥ�䵽�ĵ�һ����Ŀ����(�����У�飬��У��ʧ�ܻ�ɾ��flash�е�����)
* Param(in):  	 pReg:   ָ���Ĵ洢��
                 match:  ��ȡ����Ŀ��ƥ����Ϣ
                 p_item: ȡ������Ŀ����
* Param(out):
* Return:     	 ȡ������Ŀ����Ч���ݳ���(����Ϊ0���Ҳ�����Ӧ��Ŀ��ȡ��Ŀʧ��)
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint16_t reg_record_get_match_item ( reg_base_t* pReg, match_t* match, const void* p_item )
{
	uint8_t len = 0;
	uint16_t i = 0;
	uint8_t* flash_match_data;//flash�е�ƥ������
	uint8_t* flash_item_data;//flash�е���Ŀ����
	uint8_t* start;//item�е�������(���������Ⱥͼ����)
	uint8_t* p_item_temp;
	item_head_t* item_head;

	//������Ч���ж�
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 0;
	}
	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 0;
	}
	flash_item_data = ( uint8_t* ) C_MALLOC ( pReg->cfg_item_size );
	if ( !flash_item_data )
	{
		C_FREE ( flash_match_data );
		return 0;
	}
	/*�����洢������ƥ����Ϣ����Ŀ*/
	p_item_temp = ( uint8_t* ) p_item;
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //��Ч����
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, flash_match_data, \
			               match->match_len );
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, flash_item_data, \
				               pReg->cfg_item_size );
				/*��ȡ������Ŀ����У�飬�������ɾ��ƥ����Ϣ��Ŀ*/
				item_head = ( item_head_t* ) flash_item_data;
				start = &item_head->data;
				if ( item_head->check_sum == get_check_data_sum ( start, item_head->len ) )
				{
					memcpy ( p_item_temp, start, item_head->len );
					len =  item_head->len;
					break;
				}
				else
				{
#if   DEBUG_REG_RECORD
					debug ( "reg_record_match_get >>>>> check sum error,delete item on flash!\r\n" );
#endif
					reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, mask_info_discard, \
					                sizeof ( mask_info_discard ) );
					C_FREE ( flash_item_data );
					C_FREE ( flash_match_data );
					return 0;
				}
			}
		}
	}
#if   DEBUG_REG_RECORD
	if ( i == pReg->max_index )
	{
		debug ( "reg_record_match_get >> not mach data\r\n" );
	}
#endif
	C_FREE ( flash_item_data );
	C_FREE ( flash_match_data );

	return len;
}
/******************************************************************************
* Name: 	 reg_record_get_match_item_all
*
* Desc:       	 ��ȡƥ�䵽��������Ŀ����(�����У�飬��У��ʧ�ܻ�ɾ��flash�е�����)
* Param(in):  	 pReg:   ָ���Ĵ洢��
                 match:  ��ȡ����Ŀ��ƥ����Ϣ
                 p_item: ȡ������Ŀ����
* Param(out):
* Return:     	 ȡ������Ŀ����Ч���ݳ���(����Ϊ0���Ҳ�����Ӧ��Ŀ��ȡ��Ŀʧ��)
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint16_t reg_record_get_match_item_all ( reg_base_t* pReg, match_t* match, const void* p_item )
{
	uint8_t len = 0;
	uint16_t i = 0;
	uint8_t* flash_match_data;//flash�е�ƥ������
	uint8_t* flash_item_data;//flash�е���Ŀ����
	uint8_t* start;//item�е�������(���������Ⱥͼ����)
	uint8_t* p_item_temp;
	item_head_t* item_head;

	//������Ч���ж�
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 0;
	}
	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 0;
	}
	flash_item_data = ( uint8_t* ) C_MALLOC ( pReg->cfg_item_size );
	if ( !flash_item_data )
	{
		C_FREE ( flash_match_data );
		return 0;
	}
	/*�����洢������ƥ����Ϣ����Ŀ*/
	p_item_temp = ( uint8_t* ) p_item;
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //��Ч����
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, flash_match_data, \
			               match->match_len );
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, flash_item_data, \
				               pReg->cfg_item_size );
				/*��ȡ������Ŀ����У�飬�������ɾ��ƥ����Ϣ��Ŀ*/
				item_head = ( item_head_t* ) flash_item_data;
				start = &item_head->data;
				if ( item_head->check_sum == get_check_data_sum ( start, item_head->len ) )
				{
					memcpy ( p_item_temp, start, item_head->len );
					len +=  item_head->len;
					p_item_temp += item_head->len;
				}
				else
				{
#if   DEBUG_REG_RECORD
					debug ( "reg_record_match_get >>>>> check sum error,delete item on flash!\r\n" );
#endif
					reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, mask_info_discard, \
					                sizeof ( mask_info_discard ) );
					C_FREE ( flash_item_data );
					C_FREE ( flash_match_data );
					return 0;
				}
			}
		}
	}
#if   DEBUG_REG_RECORD
	if ( i == pReg->max_index )
	{
		debug ( "reg_record_match_get >> not mach data\r\n" );
	}
#endif
	C_FREE ( flash_item_data );
	C_FREE ( flash_match_data );

	return len;
}

/******************************************************************************
* Name: 	 reg_record_match_delete
*
* Desc:       	 ɾ���������ĵ�һ������ƥ����Ϣ����Ŀ
* Param(in):  	 pReg:  �洢��
                 match: ƥ����Ϣ
* Param(out):
* Return:     	 0->success,>0->fail
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint8_t reg_record_match_delete ( reg_base_t* pReg,match_t* match )
{
	uint8_t result = 1;
	uint16_t i;
	uint8_t* flash_match_data;

	//������Ч���ж�
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 1;
	}

	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 1;
	}

	/*�����洢������ƥ����Ϣ����Ŀ*/
	result = 1;
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //��Ч����
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               flash_match_data,match->match_len );

			/*�ҵ�ƥ����Ϣ����Ŀ������Ŀ��ͷ��д������ı��*/
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, mask_info_discard, \
				                sizeof ( mask_info_discard ) );
				pReg->index_effictive_array[i >> 3] &= ~ ( 0x80 >> ( i & 0x07 ) );
				result = 0;
				break;
			}
		}
	}
	C_FREE ( flash_match_data );
	return result;
}
/******************************************************************************
* Name: 	 reg_record_match_delete_all
*
* Desc:       	 ɾ����������ƥ����Ϣ��������Ŀ
* Param(in):  	 pReg:  �洢��
                 match: ƥ����Ϣ
* Param(out):
* Return:     	 ʵ��ɾ��������
* Global:
* Note:          ֻ�Ǳ��Ϊɾ������δ��flash��ɾ��
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint8_t reg_record_match_delete_all ( reg_base_t* pReg, match_t* match )
{
	uint8_t del_cnt = 0;
	uint16_t i;
	uint8_t* flash_match_data;

	//������Ч���ж�
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 0;
	}

	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 0;
	}

	/*�����洢������ƥ����Ϣ����Ŀ*/
	del_cnt = 0;
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //��Ч����
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               flash_match_data,match->match_len );

			/*�ҵ�ƥ����Ϣ����Ŀ������Ŀ��ͷ��д������ı��*/
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, mask_info_discard, \
				                sizeof ( mask_info_discard ) );
				pReg->index_effictive_array[i >> 3] &= ~ ( 0x80 >> ( i & 0x07 ) );
				del_cnt++;
			}
		}
	}

	C_FREE ( flash_match_data );
	return del_cnt;
}

/******************************************************************************
* Name: 	 reg_record_get_effect_item_num
*
* Desc:       	 ��ȡ�洢����ʵ����Ч�洢����Ŀ��
* Param(in):  	 pReg :ָ���Ĵ洢��
* Param(out):
* Return:     	 �洢������Ч��Ŀ����
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint32_t  reg_record_get_effect_item_num ( reg_base_t* pReg )
{
	uint32_t i;
	uint32_t count = 0;

	if ( pReg == NULL )
	{
		return 0;
	}
	/*�����������飬����ʵ����Ч�洢��Ŀ��*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
		{
			count++;
		}
	}
	return count;
}
/******************************************************************************
* Name: 	 reg_record_get_match_item_data_length
*
* Desc:       	 ��ȡ��һ��ƥ����Ϣ����Ŀ��ʵ�ʳ���
* Param(in):  	 pReg:  �洢��
                 match: ƥ����Ϣ
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint16_t reg_record_get_match_item_actual_length ( reg_base_t* pReg, match_t* match )
{
	uint16_t i;
	uint16_t len = 0;
	uint8_t* flash_match_data;

	//������Ч���ж�
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 0;
	}
	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 0;
	}

	/*�����洢������ƥ����Ϣ����Ŀ*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //��Ч����
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               flash_match_data, match->match_len );

			/*�ҵ�ƥ����Ϣ����Ŀ����ȡ��Ч����*/
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, flash_match_data, \
				               1 );
				len += *flash_match_data;
				break;
			}
		}
	}
	C_FREE ( flash_match_data );
	return len;
}
/******************************************************************************
* Name: 	 reg_record_match_get_data
*
* Desc:       	 ��ȡ����������Ч��Ŀ��ָ��ƫ��λ�õ�ָ�����ȵ�����
* Param(in):  	 pReg:  �洢��
                 match: ƥ����Ϣ(ֻ��ƫ��λ�úͳ�����Ч)
* Param(out): 	 p_item:ȡ��������
* Return:     	 ȡ�����ݵĳ���
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint16_t reg_record_get_all_item_offset_len_data ( reg_base_t* pReg, match_t*  match, const void* p_item )
{
	uint16_t i;
	uint16_t len = 0;
	uint8_t* p;

	if ( ( NULL == pReg ) || ( NULL == p_item ) || ( NULL == match ) )
	{
		return 0;
	}
	p = ( uint8_t* ) p_item;
	/*�����洢��*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //��Ч����
		{
			/*��ȡ��Ч��Ŀָ��ƫ�Ƶ�����*/
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               p, match->match_len );
			p = p + match->match_len;
			len += match->match_len;
		}
	}

	return len;
}

/******************************************************************************
* Name: 	 reg_record_get_match_item_special_data
*
* Desc:       	 ��ȡ����ƥ����Ŀ��ָ��ƫ�Ƽ�ָ�����ȵ�����
* Param(in):  	 pReg:  �洢��
                 match: ƥ����Ϣ(ֻ��ƫ��λ�úͳ�����Ч)
                 match_special:ָ����ƫ�Ƽ�����
* Param(out): 	 p_item:ȡ��������
* Return:     	 ȡ�����ݵĳ���
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint16_t reg_record_get_match_item_special_data ( reg_base_t* pReg, match_t*  match, match_t*  match_special, const void* p_item )
{
	uint16_t i;
	uint16_t len = 0;
	uint8_t* p;
	uint8_t* flash_match_data;

	//������Ч���ж�
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) || ( NULL == match_special ) )
	{
		return 0;
	}
	p = ( uint8_t* ) p_item;
	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
#if   DEBUG_REG_RECORD
		debug ( "reg_record_get_match_item_special_data>>>>>param error!\r\n" );
#endif
		return 0;
	}

	/*�����洢��*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
		{

			/*��ȡ��Ч��Ŀָ��ƫ�Ƶ�����*/
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               flash_match_data, match->match_len );
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match_special->offset, \
				               p, match_special->match_len );
				p = p + match_special->match_len;
				len += match_special->match_len;
			}
		}
	}
	C_FREE ( flash_match_data );
	return len;
}
/******************************************************************************
* Name: 	 reg_record_real_time
*
* Desc:       	 ʵʱ�����Ƿ���������������FLASH����
* Param(in):  	 pReg:  �洢��
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
void  reg_record_real_time ( reg_base_t* pReg )
{
	uint16_t i, first, k;
	uint8_t* flash_item_data;
	uint16_t now_sector;

	if ( pReg == NULL )
	{
		return;
	}
	/*�����������飬���洢���Ƿ�����*/
	//�ȽϺ�ʱ�䣬�����Ż���ֻ���max_index-1���bit�Ƿ�ʹ��
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 ) && \
		        ( ( ( i / pReg->cfg_per_page_item_cnt ) + pReg->start_sector ) != pReg->back_sector ) )
		{
			break;
		}
	}
	/*�洢������������FLASH����*/
	if ( i == pReg->max_index )
	{
#if   DEBUG_REG_RECORD
		debug ( "table full,auto to arrange flash...\r\n" );
#endif
		/*����pReg->index_effictive_array�������飬�ҵ������Ŀɱ������FLASH��Ŀ*/
		for ( i = 0; i < pReg->max_index; i++ )
		{
			if ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
			{
				break;
			}
		}
		/*�Ҳ����ɱ��������Ŀ����ȷʵ�Ѿ�ʵ����Ч�Ĵ���*/
		if ( i == pReg->max_index )
		{
#if   DEBUG_REG_RECORD
			debug ( "flash restory effect item full,arrange flash fail!!!!\r\n" );
#endif
			return ;
		}
		/*�ҵ��ɱ��������Ŀ������FLASH����*/
		if ( i != pReg->max_index )
		{
#if   DEBUG_REG_RECORD
			debug ( "[%d]remove effect item to back sector...\r\n",__LINE__ );
#endif
			flash_item_data = ( uint8_t* ) C_MALLOC ( pReg->cfg_item_size );
			if ( !flash_item_data )
			{
#if   DEBUG_REG_RECORD
				debug ( "reg_record_match_real_time>>>C_MALLOC flash_item_data fail.\r\n" );
#endif
				return ;
			}
			/*���㽫�������ҳ��*/
			now_sector = i / pReg->cfg_per_page_item_cnt + pReg->start_sector;
			i = i - i % pReg->cfg_per_page_item_cnt;
			k = ( pReg->back_sector - pReg->start_sector ) * pReg->cfg_per_page_item_cnt;

			/*�������ҳ����Ч����Ϣ���Ƶ�����ҳ*/
			for ( first = i; first < i + pReg->cfg_per_page_item_cnt; first++ )
			{
				if ( ( pReg->index_effictive_array[first >> 3] & ( 0x80 >> ( first & 0x07 ) ) ) != 0 )
				{
					reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + first * pReg->cfg_item_size, flash_item_data, \
					               pReg->cfg_item_size );
					reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + k * pReg->cfg_item_size, flash_item_data, \
					                flash_item_data[0] + MASK_INFO_LEN );
					pReg->index_use_array[k >> 3] &= ~ ( 0x80 >> ( k & 0x07 ) );
					k++;
					pReg->items_cnt++;
				}
			}
#if   DEBUG_REG_RECORD
			debug ( "arrange flash success!\r\n" );
#endif
			/*��������ҳ���������ҳ����Ϊ�µı���ҳ����������������pReg->index_use_array��pReg->index_effictive_array*/
			reg_nor_erase ( now_sector, 1 );
			pReg->back_sector = now_sector;
			for ( first = 0; first < ( pReg->cfg_per_page_item_cnt >> 3 ); first++ )
			{
				pReg->index_use_array[ ( i >> 3 ) + first] = 0xFF;
				pReg->index_effictive_array[ ( i >> 3 ) + first] = 0xFF;
			}
			pReg->items_cnt -= pReg->cfg_per_page_item_cnt;
			C_FREE ( flash_item_data );
		}
	}
}
/******************************************************************************
* Name: 	 reg_record_get_match_item_cnt
*
* Desc:       	 ��ȡƥ����Ϣ��Ŀ����Ŀ����
* Param(in):  	 pReg:  �洢��
                 match: ƥ����Ϣ(ֻ��ƫ��λ�úͳ�����Ч)
* Param(out):
* Return:     	 ƥ�����Ŀ����
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint32_t reg_record_get_match_item_cnt ( reg_base_t* pReg, match_t*  match )
{
	uint32_t i;
	uint32_t count = 0;
	uint8_t* flash_match_data = NULL;

	//������Ч���ж�
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 0;
	}

	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 0;
	}

	/*�����洢��*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
		{
			/*ƥ����Ŀ���м���*/
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               flash_match_data, match->match_len );
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				count++;
			}
		}
	}
	C_FREE ( flash_match_data );
	return count;
}
/*---------------------------------------------------------------------------*/
