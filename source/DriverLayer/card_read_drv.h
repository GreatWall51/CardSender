/********************************************************************************
**
** 文件名:     	 Card_read_drv.h 
** 版权所有:   (c) 	 2016 厦门立林科技有限公司
** 文件描述:   
**
*********************************************************************************
**             修改历史记录
**===============================================================================
**| 日期        | 作者     |  修改记录
**===============================================================================
**| 2016/02/24  | yeqizhi  |  创建该文件
**
*********************************************************************************/
#ifndef _CARD_READ_DRV_H_     
#define _CARD_READ_DRV_H_    
 
/*-------------------------------- Includes ----------------------------------*/
 #include <string.h>
 #include "stm32f0xx.h"
 
/*------------------------------ Global Defines ------------------------------*/
#define UID_MAX_LEN 11


#define IC_SCAN_TIMES_START  0x01       //开始扫描IC卡
#define IC_SCAN_TIMES        0x01       //IC卡扫描次数
#define SFZ_SCAN_TIMES       0x01       //身份证扫描次数
#define	NFC_SCAN_TIMES       0x01       //NFC卡扫描次数

#define IC_INTERVAL_TIME    6
#define SFZ_INTERVAL_TIME   6
#define NFC_INTERVAL_TIME   6
 
/*------------------------------ Global Typedefs -----------------------------*/
typedef struct   CARD_READER_S{
    uint8_t card_ab_type;       //卡类型  0:只读IC卡  1:支持读身份证和IC卡
    uint8_t anti_copy;         //IC防复制参数，81-8F
    uint8_t project_num[16];      //项目号
    uint8_t adf[2];         //ADF标示
    uint8_t key_inter;     //内部密钥号
}CARD_READER_CONFIG;

enum card_type{
    CARD_NONE = 0x00,
    TYPE_ID=0x01,
    TYPE_IC=0x11,
    TYPE_IC_NO_REG=0x91,
    TYPE_CPU=0x12,
    TYPE_CPU_NO_REG=0x92,
    TYPE_SFZ=0X21,
    TYPE_IC_B=0X22,  
    TYPE_NFC_NO_REG=0x93,
    TYPE_NFC = 0x31,
};

enum card_uid_len{
    CARD_IC_LEN=0x04,
    CARD_SFZ_LEN=0x08, 
    CARD_NFC_LEN=0x08, 
};

enum work_mode{
    IC_MODE = 0x00,
    SFZ_MODE =0x01, 
    NFC_MODE = 0X02,
};

enum ANTI_COPY_PARAM_E{
    ANTI_COPY_NONE = 0x00,
    ANTI_COPY_IC = 0x8F,       //IC卡防复制
    ANTI_COPY_CPU = 0xC0,      //CPU卡防复制
};


/*----------------------------- External Variables ---------------------------*/
typedef struct
{
    uint8_t  card_type;      //卡类型
    uint8_t  card_len;       //卡uid长度
    uint8_t  card_uid[UID_MAX_LEN];    //卡uid值
}CARD_INFO_T;  

typedef enum
{
	CARD_EVT_NONE = 0,
	CARD_EVT_ENTER,
	CARD_EVT_EXIT,
	
}CARD_EVT_E;

/*------------------------- Global Function Prototypes -----------------------*/
void card_read_drv_init(void);
void card_read_drv_real_time(void);
uint8_t card_get_event(CARD_INFO_T *card_data_str);
uint8_t check_have_card(void);
uint8_t chevt_have_card_evt(void);
uint8_t get_reader_mode(void);

#endif //_CARD_READ_APP_H_
