/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 reg_record.c
* Desc:      flash记录基本操作功能
*
*
* Author: 	 Liuwq
* Date: 	 2018/07/19
* Notes:	 flash中数据存储结构
            -----------------------------------------------------------------
            | Byte0 | Byte1 | Byte2 |               ...                     |
            -----------------------------------------------------------------
            |  LEN  |  SUM  |          item_data(match info)...             |
            -----------------------------------------------------------------
说明:(1)LEN和SUM这两个字段也用来作MASK(0xff,0xff表示可用;0x00,0x00表示废弃)
     (2)match info位于item的数据区域,可用match->offset来偏移,match->match_len指明长度
移植说明: (1)需外部提供以下接口
          C_FREE
          C_MALLOC
          uint32_t flash_read ( uint32_t address, uint8_t* paddr, uint32_t size );
          FLASH_ERROR_CODE_E  flash_write ( uint32_t address, const uint8_t* paddr, uint32_t size );
          FLASH_ERROR_CODE_E flash_erase_sector(uint32_t sector_from, uint16_t sector_cnt);
          (2)需提供以下宏定义
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

#define CFG_FLASH_SECTOR_SIZE    FLASH_PAGE_SIZE         //页的大小1024Byte
#define CFG_FLASH_SIZE           (FLASH_USER_END_ADDR+1) //FLASH 最大地址 */

#define MASK_INFO_LEN 2   /*信息头部长度*/
static const uint8_t mask_info_unused[MASK_INFO_LEN] = {0xFF, 0xFF};  //条目未使用标记
static const uint8_t mask_info_discard[MASK_INFO_LEN] = {0x00, 0x00}; //条目废弃标记
/*---------------------- Constant / Macro Definitions -----------------------*/
#define ADDR_2_SECTOR(ADDR) ((ADDR)/CFG_FLASH_SECTOR_SIZE)
#define SECTOR_2_ADDR(SECTOR) ((SECTOR) * CFG_FLASH_SECTOR_SIZE)

/*----------------------- Type Declarations ---------------------------------*/
typedef struct
{
	uint8_t  len;                   /*!< 条目实际有效数据长度*/
	uint8_t  check_sum;             /*!< 校验和*/
	uint8_t  data;                  /*!< 数据域*/
} item_head_t;

/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name: 	 get_check_data_sum
*
* Desc:       	 计算校验和，检验数据完整性
* Param(in):  	 point:待校验数据，len，待校验数据长度
* Param(out):
* Return:     	 检验和
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
* Desc:       	 读取表基本信息
* Param(in):  	 pReg:待读取基本信息的表
* Param(out): 	 pReg:读完基本信息的表
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
	uint8_t now_sector;    //掉电前FLASH整理的页
	uint8_t mask[MASK_INFO_LEN] = {0};//掩码数据，用来判断数据的有效性及使用情况
	uint8_t* flash_match_data;
	uint8_t* flash_item_data;
	uint16_t first;

	if ( pReg == NULL )
	{
		return 1;
	}
	/*初始化条目索引数组*/
	memset ( pReg->index_effictive_array, 0xFF, ( pReg->max_index >> 3 ) + 1 );
	memset ( pReg->index_use_array, 0xFF, ( pReg->max_index >> 3 ) + 1 );

	/*读取FLASH条目使用状态，设置索引数组，搜索空白页作为备用页*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, mask, \
		               sizeof ( mask ) );
		//搜索备用页
		if ( memcmp ( mask, mask_info_unused,sizeof ( mask ) ) == 0 )
		{
			if ( ! ( i % pReg->cfg_per_page_item_cnt ) )
			{
				pReg->back_sector = ( i / pReg->cfg_per_page_item_cnt ) + pReg->start_sector;
			}
		}
		//设置索引数组
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
	/*搜索不到备用页 */
	if ( pReg->back_sector == 0 )
	{
		/*搜索不到备用页，搜寻掉电前进行FLASH整理的页以及掉电前的备用页，一定可以找到，i必小于max_index*/
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

		/*完成掉电前FLASH的整理*/
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
* Desc:       	 清空表
* Param(in):
* Param(out):
* Return:     	 0 -> success,
								 1 -> parameter error,
								 2 -> erase sector error,
* Global:
* Note:      清空flash中信息
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
* Desc:       	 在指定Flash的base_addr位置处建立一张滑动表，该表的至少可存储:
                 cfg_max_items_cnt条单条容量为cfg_item_size 大小的条目
* Param(in):  	 pReg:用于存储滑动表的配置信息，存储空间被预先分配，将在该函数内被初始化
                 base_addr    存储表的起始地址
                 cfg_max_items_cnt   该表至少能存储的条目数
                 cfg_item_size        单条条目大小
* Param(out): 	 存储表的结束地址,0->error
* Return:
* Global:
* Note:
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint32_t reg_record_init ( reg_base_t* pReg, uint32_t base_addr, uint32_t cfg_max_items_cnt, ITEM_SIZE_ENUM cfg_item_size )
{

	uint32_t max_size;//总存储空间
	uint16_t i, page_max_num, count;
	uint32_t end_addr;
	uint8_t result = 1;

	if ( pReg == NULL || cfg_item_size > ITEM_SIZE_256 )
	{
		return 0;
	}

	/*初始化起始页及表的固定配置参数*/
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

	/*为索引数组分配空间*/
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

	/*初始化FLASH条目使用情况的索引数组及备用页*/
	result = read_config ( pReg );
	if ( result != 0 )
	{
		return 0;
	}

	/*计算已存储的条目数*/
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
* Desc:       	 向指定的滑动存储表里插入一个新条目(条目最大长度为pReg->cfg_item_size)
* Param(in):  	 pReg -- 指向滑动存储表配置信息，已被reg_record_init初始化过
                 p_item -- 指向要存储的条目内容(条目最大长度为pReg->cfg_item_size)
* Param(out):
* Return:     	 本表存储的条目数,>=success,=0puterror,=-1table full
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
	/*遍历索引数组，找到不在备用页的未被占用的FLASH条目空间*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 ) && \
		        ( ( ( i / pReg->cfg_per_page_item_cnt ) + pReg->start_sector ) != pReg->back_sector ) )
		{
			break;
		}
	}
	/*找到可存储的空间，直接写入，并更新pReg->index_use_array数组*/
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

	/*找不到可存储的空间，进行FLASH整理，再写入*/
	else
	{
#if   DEBUG_REG_RECORD
		debug ( "arrange flash to put new item...!\r\n" );
#endif
		// DEBUG_TOP("flash clean\r\n");
		/*遍历pReg->index_effictive_array索引数组，找到废弃的可被整理的FLASH条目*/
		for ( i = 0; i < pReg->max_index; i++ )
		{
			if ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
			{
				break;
			}
		}

		/*找不到可被整理的条目，表确实已经实际有效的存满，需上层应用进行释放，返回写入失败*/
		if ( i == pReg->max_index )
		{
#if   DEBUG_REG_RECORD
			debug ( "flash restory effect item full,arrange flash fail!!!!\r\n" );
#endif
			return -1;
		}

		/*找到可被整理的条目，进行FLASH整理*/
		flash_item_data = ( uint8_t* ) C_MALLOC ( pReg->cfg_item_size );
		if ( !flash_item_data )
		{
			return 0;
		}
#if   DEBUG_REG_RECORD
		debug ( "remove effect item to back sector...\r\n" );
#endif
		/*计算将被整理的页号*/
		now_sector = i / pReg->cfg_per_page_item_cnt + pReg->start_sector;
		i = i - i % pReg->cfg_per_page_item_cnt;//计算被整理的页的起始条目编号
		k = ( pReg->back_sector - pReg->start_sector ) * pReg->cfg_per_page_item_cnt; //计算备用页的起始条目编号

		/*将整理的页中有效的信息搬移到备用页*/
		for ( first = i; first < i + pReg->cfg_per_page_item_cnt; first++ )
		{
			if ( ( pReg->index_effictive_array[first >> 3] & ( 0x80 >> ( first & 0x07 ) ) ) != 0 ) //有效数据
			{
				uint32_t len = 0,result = 1;
				len = reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + first * pReg->cfg_item_size, flash_item_data, \
				                     pReg->cfg_item_size );
				if ( len == 0 ) //读出错
				{
					C_FREE ( flash_item_data );
					return 0;
				}
				result = reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + k * pReg->cfg_item_size, flash_item_data, \
				                         flash_item_data[0] + MASK_INFO_LEN );
				if ( result != 0 ) //写出错
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
		/*在备用页的可用空间中写入待写入的数据*/
		reg_nor_write ( SECTOR_2_ADDR ( pReg->start_sector ) + k * pReg->cfg_item_size, \
		                p_item, *p_item + MASK_INFO_LEN );
		pReg->index_use_array[k >> 3] &= ~ ( 0x80 >> ( k & 0x07 ) );
		pReg->items_cnt++;

		/*擦除备用页，将整理的页设置为新的备用页，并更新索引数组pReg->index_use_array和pReg->index_effictive_array*/
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
* Desc:       	 向表中写入数据
* Param(in):  	 pReg:   指定的表
                 match:  匹配的信息(NULL不进行匹配,找到空位置就直接写入，
                                  !=NULL删除所有匹配信息，并进行写入一条)
                 p_item: 待写入的数据
                 len:    待写入数据的有效长度
* Param(out):
* Return:     	 写入的长度，若长度为0，写入失败
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
	/*检查是否需要匹配的写入，需要则先删除表中所有匹配信息的条目，再将条目写入(只写一条)*/
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

	/*检查待写入的长度是否超过配置的单条存储条目大小*/
	if ( len <= ( pReg->cfg_item_size - MASK_INFO_LEN ) )
	{
		memset ( flash_item_data, 0xFF, pReg->cfg_item_size );

		/*填充待写入条目的头部信息*/
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
* Desc:       	 获取匹配到的第一条条目数据(会进行校验，若校验失败会删除flash中的数据)
* Param(in):  	 pReg:   指定的存储表
                 match:  待取出条目的匹配信息
                 p_item: 取出的条目数据
* Param(out):
* Return:     	 取出的条目的有效数据长度(长度为0，找不到对应条目，取条目失败)
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
	uint8_t* flash_match_data;//flash中的匹配数据
	uint8_t* flash_item_data;//flash中的条目数据
	uint8_t* start;//item中的数据域(不包括长度和检验和)
	uint8_t* p_item_temp;
	item_head_t* item_head;

	//参数有效性判断
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
	/*遍历存储表，搜索匹配信息的条目*/
	p_item_temp = ( uint8_t* ) p_item;
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //有效数据
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, flash_match_data, \
			               match->match_len );
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, flash_item_data, \
				               pReg->cfg_item_size );
				/*对取到的条目进行校验，如果出错，删除匹配信息条目*/
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
* Desc:       	 获取匹配到的所有条目数据(会进行校验，若校验失败会删除flash中的数据)
* Param(in):  	 pReg:   指定的存储表
                 match:  待取出条目的匹配信息
                 p_item: 取出的条目数据
* Param(out):
* Return:     	 取出的条目的有效数据长度(长度为0，找不到对应条目，取条目失败)
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
	uint8_t* flash_match_data;//flash中的匹配数据
	uint8_t* flash_item_data;//flash中的条目数据
	uint8_t* start;//item中的数据域(不包括长度和检验和)
	uint8_t* p_item_temp;
	item_head_t* item_head;

	//参数有效性判断
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
	/*遍历存储表，搜索匹配信息的条目*/
	p_item_temp = ( uint8_t* ) p_item;
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //有效数据
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, flash_match_data, \
			               match->match_len );
			if ( memcmp ( flash_match_data, match->match_data, match->match_len ) == 0 )
			{
				reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size, flash_item_data, \
				               pReg->cfg_item_size );
				/*对取到的条目进行校验，如果出错，删除匹配信息条目*/
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
* Desc:       	 删除搜索到的第一条满足匹配信息的条目
* Param(in):  	 pReg:  存储表
                 match: 匹配信息
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

	//参数有效性判断
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 1;
	}

	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 1;
	}

	/*遍历存储表，搜索匹配信息的条目*/
	result = 1;
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //有效数据
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               flash_match_data,match->match_len );

			/*找到匹配信息的条目，将条目的头部写入废弃的标记*/
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
* Desc:       	 删除表中满足匹配信息的所有条目
* Param(in):  	 pReg:  存储表
                 match: 匹配信息
* Param(out):
* Return:     	 实际删除的条数
* Global:
* Note:          只是标记为删除，并未在flash中删除
* Author: 	 Liuwq
* -------------------------------------
* Log: 	 2018/07/19, Create this function by Liuwq
 ******************************************************************************/
uint8_t reg_record_match_delete_all ( reg_base_t* pReg, match_t* match )
{
	uint8_t del_cnt = 0;
	uint16_t i;
	uint8_t* flash_match_data;

	//参数有效性判断
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 0;
	}

	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 0;
	}

	/*遍历存储表，搜索匹配信息的条目*/
	del_cnt = 0;
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //有效数据
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               flash_match_data,match->match_len );

			/*找到匹配信息的条目，将条目的头部写入废弃的标记*/
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
* Desc:       	 获取存储表中实际有效存储的条目数
* Param(in):  	 pReg :指定的存储表
* Param(out):
* Return:     	 存储表中有效条目数量
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
	/*遍历索引数组，计算实际有效存储条目数*/
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
* Desc:       	 获取第一条匹配信息的条目的实际长度
* Param(in):  	 pReg:  存储表
                 match: 匹配信息
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

	//参数有效性判断
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 0;
	}
	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 0;
	}

	/*遍历存储表，搜索匹配信息的条目*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //有效数据
		{
			reg_nor_read ( SECTOR_2_ADDR ( pReg->start_sector ) + i * pReg->cfg_item_size + MASK_INFO_LEN + match->offset, \
			               flash_match_data, match->match_len );

			/*找到匹配信息的条目，获取有效长度*/
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
* Desc:       	 获取表中所有有效条目的指定偏移位置的指定长度的数据
* Param(in):  	 pReg:  存储表
                 match: 匹配信息(只有偏移位置和长度有效)
* Param(out): 	 p_item:取出的数据
* Return:     	 取出数据的长度
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
	/*遍历存储表*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 ) //有效数据
		{
			/*获取有效条目指定偏移的数据*/
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
* Desc:       	 获取表中匹配条目的指定偏移及指定长度的数据
* Param(in):  	 pReg:  存储表
                 match: 匹配信息(只有偏移位置和长度有效)
                 match_special:指定的偏移及长度
* Param(out): 	 p_item:取出的数据
* Return:     	 取出数据的长度
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

	//参数有效性判断
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

	/*遍历存储表*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
		{

			/*获取有效条目指定偏移的数据*/
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
* Desc:       	 实时检测表是否存满，存满则进行FLASH整理
* Param(in):  	 pReg:  存储表
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
	/*遍历索引数组，检查存储表是否已满*/
	//比较耗时间，可以优化成只检测max_index-1这个bit是否被使用
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 ) && \
		        ( ( ( i / pReg->cfg_per_page_item_cnt ) + pReg->start_sector ) != pReg->back_sector ) )
		{
			break;
		}
	}
	/*存储表已满，进行FLASH整理*/
	if ( i == pReg->max_index )
	{
#if   DEBUG_REG_RECORD
		debug ( "table full,auto to arrange flash...\r\n" );
#endif
		/*遍历pReg->index_effictive_array索引数组，找到废弃的可被整理的FLASH条目*/
		for ( i = 0; i < pReg->max_index; i++ )
		{
			if ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
			{
				break;
			}
		}
		/*找不到可被整理的条目，表确实已经实际有效的存满*/
		if ( i == pReg->max_index )
		{
#if   DEBUG_REG_RECORD
			debug ( "flash restory effect item full,arrange flash fail!!!!\r\n" );
#endif
			return ;
		}
		/*找到可被整理的条目，进行FLASH整理*/
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
			/*计算将被整理的页号*/
			now_sector = i / pReg->cfg_per_page_item_cnt + pReg->start_sector;
			i = i - i % pReg->cfg_per_page_item_cnt;
			k = ( pReg->back_sector - pReg->start_sector ) * pReg->cfg_per_page_item_cnt;

			/*将整理的页中有效的信息搬移到备用页*/
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
			/*擦除备用页，将整理的页设置为新的备用页，并更新索引数组pReg->index_use_array和pReg->index_effictive_array*/
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
* Desc:       	 获取匹配信息条目的条目数量
* Param(in):  	 pReg:  存储表
                 match: 匹配信息(只有偏移位置和长度有效)
* Param(out):
* Return:     	 匹配的条目数量
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

	//参数有效性判断
	if ( ( NULL == pReg ) || ( NULL == match ) || ( 0 == match->match_len ) || ( NULL == match->match_data ) )
	{
		return 0;
	}

	flash_match_data = ( uint8_t* ) C_MALLOC ( match->match_len );
	if ( !flash_match_data )
	{
		return 0;
	}

	/*遍历存储表*/
	for ( i = 0; i < pReg->max_index; i++ )
	{
		if ( ( ( pReg->index_effictive_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) != 0 )
		        && ( pReg->index_use_array[i >> 3] & ( 0x80 >> ( i & 0x07 ) ) ) == 0 )
		{
			/*匹配条目进行计数*/
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
