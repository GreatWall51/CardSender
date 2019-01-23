/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 debug_card_drv.c
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/05/24
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/05/24, Liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "opt.h"
#include "debug.h"
#include "des.h"
#include "card_encrypt.h"
#include "lib_iso14443Apcd.h"
#include "lib_iso14443Bpcd.h"
#include "timer_drv.h"
/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
uint8_t card_uid[8] = {0};
uint8_t prj_num[16] = {0};
uint8_t mf[2]={0x3f,0x00};
uint8_t adf[2]={0x10,0x05};
uint8_t adf_size[2]={0x08,0x00};
uint8_t keyfile_id[2]={0x00,0x01};
uint8_t keyfile_size[2]={0x01,0x40};
uint8_t ex_key_cnt=0x03;//外部认证密钥
uint8_t in_key_cnt=0x05;//内部认证密钥
uint8_t ef_read_key_cnt=0x01;//读ef密钥
uint8_t ef_write_key_cnt=0x02;//写ef密钥
uint8_t ef_file_id[2]={0x00,0x02};

uint8_t ex_key_type=0x39;
uint8_t in_key_type=0x30;
uint8_t ef_read_key_type=0x39;
uint8_t ef_write_key_type=0x39;
uint8_t key[16]={0};
uint8_t key_len = 16;
/*----------------------- Function Prototype --------------------------------*/



/*----------------------- Function Implement --------------------------------*/
DEBUG_CMD_FN(card_sfz)
{
	dbg_cmd_split_t *dbg_cmd_split;

	dbg_cmd_split = get_dbg_cmd_split();
	if(CARD_READ == 0)
	{
		debug("this device not support card read!\r\n");
		return;
	}
	
	if (strncmp(dbg_cmd_split->arg[1], "scan_sfz", strlen("scan_sfz")) == 0)
	{
		if(scan_ISO14443B_card ( card_uid ) == ISO14443B_SUCCESS)
		{
			debug("scan SFZ card get uid:\r\n");
			debug_hex ( card_uid,8 );
		}
		else
		{
			debug("scan SFZ card fail!\r\n");
		}
	}
}

DEBUG_CMD_FN(card_ic)
{
	dbg_cmd_split_t *dbg_cmd_split;

	dbg_cmd_split = get_dbg_cmd_split();
	if(CARD_READ == 0)
	{
		debug("this device not support card read!\r\n"); 
		return;
	}
	if (strncmp(dbg_cmd_split->arg[1], "scan_ic", strlen("scan_ic")) == 0)
	{
		memset(card_uid,0,sizeof(card_uid));
		if(scan_ISO14443A_card ( card_uid ) == ISO14443A_SUCCESS)
		{
			debug("scan card get uid:\r\n");
			debug_hex ( card_uid,4);
		}
		else
		{
			debug("scan IC card fail!\r\n");
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "get_card_type", strlen("get_card_type")) == 0)
	{
		if(is_support_rats() == 1)
		{
			debug("card type is cpu.\r\n");
		}
		else
		{
			debug("card type is ic.\r\n");
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ic_authen", strlen("ic_authen")) == 0)//密钥认证失败时，必须重新寻卡，不能直接换密钥再认证
	{
		uint8_t result=1;
		uint8_t key_a[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
		uint8_t key_b[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
		
		if (strncmp(dbg_cmd_split->arg[2], "key_a", strlen("key_a")) == 0)
		{
			result = ISO14443A_Authen(card_uid,1,key_a,ISO14443A_CMD_AUTH_A);
			if(result==ISO14443A_SUCCESS)
			{
				debug("ISO14443A_Authen key_a success!\r\n");
			}
			else
			{
				debug("ISO14443A_Authen key_a error!\r\n");
			}
		}
		else if(strncmp(dbg_cmd_split->arg[2], "key_b", strlen("key_b")) == 0)
		{
			result = ISO14443A_Authen(card_uid,1,key_b,ISO14443A_CMD_AUTH_B);
			if(result==ISO14443A_SUCCESS)
			{
				debug("ISO14443A_Authen key_b success!\r\n");
			}
			else
			{
				debug("ISO14443A_Authen key_b error!\r\n");
			}
		}
		else
		{
			debug("param error!\r\n");
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ic_write", strlen("ic_write")) == 0)
	{
		uint8_t result;
		uint8_t write_data[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};

		result = ISO14443A_Write(0x20,write_data);
		if(result==ISO14443A_SUCCESS)
		{
			debug("ISO14443A_Write success!\r\n");
		}
		else
		{
			debug("ISO14443A_Write error!\r\n");
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ic_read", strlen("ic_read")) == 0)
	{
		uint8_t result;
		uint8_t read_data[16] = {0};
		result = ISO14443A_Read(0x20,read_data);
		if(result==ISO14443A_SUCCESS)
		{
			debug("ISO14443A_Read data:\r\n");
			debug_hex(read_data,16);
		}
		else
		{
			debug("ISO14443A_Read error!\r\n");
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ic_halt", strlen("ic_halt")) == 0)
	{
		ISO14443A_Halt();
	}
	
}





DEBUG_CMD_FN(card_cpu)
{
	dbg_cmd_split_t *dbg_cmd_split;

	dbg_cmd_split = get_dbg_cmd_split();
	if(CARD_READ == 0)
	{
		debug("this device not support card read!\r\n");
		return;
	} 
	
	if (strncmp(dbg_cmd_split->arg[1], "Rats", strlen("Rats")) == 0)
	{
		Rats();
	}
	else if (strncmp(dbg_cmd_split->arg[1], "select_mf", strlen("select_mf")) == 0)
	{
		SelectFile(mf);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ExAuthenticate_default", strlen("ExAuthenticate_default")) == 0)
	{
		//空卡默认的外部认证密钥号为0，密钥为16个0xFF
		uint8_t key_cnt = 0;
		
		memset(key,0xff,16);
		ExAuthenticate(key_cnt,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ExAuthenticate_mf", strlen("ExAuthenticate_mf")) == 0)//主目录验证用户密钥
	{
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,mf,ex_key_cnt,prj_num,key);//获取密钥
		ExAuthenticate(ex_key_cnt,key);
	}
	
	
	else if (strncmp(dbg_cmd_split->arg[1], "InAuthenticate_mf", strlen("InAuthenticate_mf")) == 0)
	{
		uint8_t key[16];
		uint8_t des_in[8],des_out[8],des_temp[8],rece_len;


		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,mf,in_key_cnt,prj_num,key);
		GetRandom ( &rece_len, des_temp );
		memcpy(des_in,des_temp,rece_len);
		TDES(des_temp,des_out,key,ENCRY);
		InAuthenticate(des_in,in_key_cnt,des_out);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "DeleteADF", strlen("DeleteADF")) == 0)
	{
		DeleteADF();
	}
	else if (strncmp(dbg_cmd_split->arg[1], "CreatKeyFile_mf", strlen("CreatKeyFile_mf")) == 0)
	{
		//leelen默认密钥文件名:0x00,0x01,大小:10*16=160字节(可存10条密钥，每条16字节)
		CreatKeyFile(keyfile_id,keyfile_size);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "AddKey_mf_ex", strlen("AddKey_mf_ex")) == 0)
	{
		//leelen默认4个密钥:内部认证密钥05,外部认证03，写ef密钥02，读EF密钥01

		memset(prj_num,0xff,key_len);
		gen_key_cpu(card_uid,mf,ex_key_cnt,prj_num,key);
		AddKey(ex_key_type,ex_key_cnt,key_len,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "AddKey_mf_in", strlen("AddKey_mf_in")) == 0)
	{
		//leelen默认4个密钥:内部认证密钥05,外部认证03，写ef密钥02，读EF密钥01

		memset(prj_num,0xff,key_len);
		gen_key_cpu(card_uid,mf,in_key_cnt,prj_num,key);
		AddKey(in_key_type,in_key_cnt,key_len,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "CreatADF", strlen("CreatADF")) == 0)
	{
		CreatADF(adf,adf_size);
	}
	
	else if (strncmp(dbg_cmd_split->arg[1], "select_adf", strlen("select_adf")) == 0)
	{
		//leelen默认adf:0x10,0x05
		SelectFile(adf);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "CreatKeyFile_adf", strlen("CreatKeyFile_adf")) == 0)
	{
		//leelen默认密钥文件名:0x00,0x01,大小:10*16=160字节(可存10条密钥，每条16字节)
		CreatKeyFile(keyfile_id,keyfile_size);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "AddKey_adf_ex", strlen("AddKey_adf_ex")) == 0)
	{
		//leelen默认4个密钥:内部认证密钥05,外部认证03，写ef密钥02，读EF密钥01
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,ex_key_cnt,prj_num,key);
		AddKey(ex_key_type,ex_key_cnt,key_len,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "AddKey_adf_in", strlen("AddKey_adf_in")) == 0)
	{
		//leelen默认4个密钥:内部认证密钥05,外部认证03，写ef密钥02，读EF密钥01		
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,in_key_cnt,prj_num,key);
		AddKey(in_key_type,in_key_cnt,key_len,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "AddKey_ef_read", strlen("AddKey_ef_read")) == 0)
	{
		//leelen默认4个密钥:内部认证密钥05,外部认证03，写ef密钥02，读EF密钥01
		uint8_t key[16];
		uint8_t key_len = 16;
		
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,ef_read_key_cnt,prj_num,key);
		AddKey(ef_read_key_type,ef_read_key_cnt,key_len,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "AddKey_ef_write", strlen("AddKey_ef_write")) == 0)
	{
		//leelen默认4个密钥:内部认证密钥05,外部认证03，写ef密钥02，读EF密钥01
		uint8_t key[16];
		uint8_t key_len = 16;
		
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,ef_write_key_cnt,prj_num,key);
		AddKey(ef_write_key_type,ef_write_key_cnt,key_len,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ExAuthenticate_adf", strlen("ExAuthenticate_adf")) == 0)
	{
		//leelen默认的外部认证密钥号为3,密钥通过gen_key_cpu计算得出
		memset(key,0xff,16);
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,ex_key_cnt,prj_num,key);//获取密钥
		ExAuthenticate(ex_key_cnt,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "CreatEFFile", strlen("CreatEFFile")) == 0)
	{
		uint8_t item_cnt = 10;
		uint8_t item_len = 32;
		uint8_t rw_key=0x91;
		
		CreatEFFile(ef_file_id,item_cnt,item_len,rw_key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ExAuthenticate_read_ef", strlen("ExAuthenticate_read_ef")) == 0)
	{
		//leelen默认的外部认证密钥号为3,密钥通过gen_key_cpu计算得出
		memset(key,0xff,16);
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,ef_read_key_cnt,prj_num,key);//获取密钥
		ExAuthenticate(ef_read_key_cnt,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ExAuthenticate_write_ef", strlen("ExAuthenticate_write_ef")) == 0)
	{
		//leelen默认的外部认证密钥号为3,密钥通过gen_key_cpu计算得出
		memset(key,0xff,16);
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,ef_write_key_cnt,prj_num,key);//获取密钥
		ExAuthenticate(ef_write_key_cnt,key);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "select_ef", strlen("select_ef")) == 0)
	{
		//leelen默认adf:0x00,0x02
		SelectFile(ef_file_id);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "WriteEF", strlen("WriteEF")) == 0)
	{
		uint8_t item_id = 0x01;
		uint8_t item_len = 32;
		uint8_t data[32] ={0},i;
		
		for(i=0;i<32;i++)
		{
			data[i] = i;
		}
		//写密钥认证
		memset(key,0xff,16);
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,ef_write_key_cnt,prj_num,key);//获取密钥
		ExAuthenticate(ef_write_key_cnt,key);
		//写ef数据
		for(i=1;i<10;i++)
		{
			item_id = i;
		WriteEF(item_id,item_len,data);
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "ReadEF", strlen("ReadEF")) == 0)
	{
		uint8_t item_id = 0x01,i;
		uint8_t item_len = 32;
		uint8_t data[32]={0};
		//读密钥认证
		memset(key,0xff,16);
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,ef_read_key_cnt,prj_num,key);//获取密钥
		ExAuthenticate(ef_read_key_cnt,key);
		//读ef
		for(i=1;i<10;i++)
		{
			item_id= i;
		ReadEF(item_id,item_len,data);
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "GetRandom", strlen("GetRandom")) == 0)
	{
		uint8_t len,data[20];
		GetRandom(&len,data);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "InAuthenticate_adf", strlen("InAuthenticate_adf")) == 0)
	{
		//leelen默认adf：0x10 0x00,内部认证密钥号0x05
			uint8_t key[16];
			uint8_t des_in[8],des_out[8],des_temp[8],rece_len;
			
			
			memset(prj_num,0xff,16);
			gen_key_cpu(card_uid,adf,in_key_cnt,prj_num,key);
			GetRandom ( &rece_len, des_temp );
			memcpy(des_in,des_temp,rece_len);
			TDES(des_temp,des_out,key,ENCRY);
			InAuthenticate(des_in,in_key_cnt,des_out);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "mf_ExAuthen_admin", strlen("mf_ExAuthen_admin")) == 0)//主目录认证管理员密钥
	{
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,mf,4,prj_num,key);//获取密钥
		ExAuthenticate(4,key);
	}
//	else if (strncmp(dbg_cmd_split->arg[1], "mf_Addkey_user_ex", strlen("mf_Addkey_user_ex")) == 0)//主目录增加用户密钥
//	{
//		CPU_CREATE_KEY_DATA_T key_cmd;
//		
//		memset(prj_num,0xff,16);
//		gen_key_cpu(card_uid,mf,ex_key_cnt,prj_num,key);
//		key_cmd.key_cnt = ex_key_cnt;
//		key_cmd.key_type = ex_key_type;
//		key_cmd.use_authority = MF_KEY_USE_AUTHORITY;
//		key_cmd.modify_authority = MF_USER_KEY_MODIFY_AUTHORITY;
//		key_cmd.authority = MF_USER_AUTHEH_AFTER;
//		key_cmd.err_cnt = 0xff;
//		key_cmd.key_len = 16;
//		memcpy(key_cmd.key,key,key_cmd.key_len);
//		ISO14443A_Addkey(&key_cmd);
//	}
//	else if (strncmp(dbg_cmd_split->arg[1], "mf_Addkey_admin_ex", strlen("mf_Addkey_admin_ex")) == 0)//主目录增加管理员密钥
//	{
//		CPU_CREATE_KEY_DATA_T key_cmd;
//		
//		memset(prj_num,0xff,16);
//		gen_key_cpu(card_uid,mf,4,prj_num,key);
//		key_cmd.key_cnt = 4;
//		key_cmd.key_type = ex_key_type;
//		key_cmd.use_authority = MF_KEY_USE_AUTHORITY;
//		key_cmd.modify_authority = MF_ADMIN_KEY_MODIFY_AUTHORITY;
//		key_cmd.authority = MF_ADMIN_AUTHEH_AFTER;
//		key_cmd.err_cnt = 0xff;
//		key_cmd.key_len = 16;
//		memcpy(key_cmd.key,key,key_cmd.key_len);
//		ISO14443A_Addkey(&key_cmd);
//	}
//	else if (strncmp(dbg_cmd_split->arg[1], "new_CreatDF", strlen("new_CreatDF")) == 0) //创建DF应用目录
//	{
//		CPU_CREATE_DF_DATA_T cmd;
//		
//		cmd.file_id[0] = adf[0];
//		cmd.file_id[1] = adf[1];
//		cmd.file_type = 0x38;
//		cmd.file_size[0]=adf_size[0];
//		cmd.file_size[1]=adf_size[1];
//		cmd.build_authority = MF_CREATE_AUTHORITY;
//		cmd.delete_authority = MF_DELETE_AUTHORITY;
//		cmd.app_id = 0x95;
//		cmd.REF[0] = 0xff;
//		cmd.REF[1] = 0xff;
//		cmd.file_name[0]=0x4c;
//		cmd.file_name[1]=0x45;
//		cmd.file_name[2]=0x45;
//		cmd.file_name[3]=0x4c;
//		cmd.file_name[4]=0x45;
//		cmd.file_name[5]=0x4e;
//		cmd.file_name_len = 6;
//		ISO14443A_CreatDF(&cmd);
//	}
//	else if (strncmp(dbg_cmd_split->arg[1], "new_Creatkeyfile", strlen("new_Creatkeyfile")) == 0)//创建密钥文件
//	{
//		CPU_CREATE_KEY_FILE_DATA_T cmd;
//		
//		cmd.file_id[0] = keyfile_id[0];
//		cmd.file_id[1] = keyfile_id[1];
//		cmd.file_type = 0x3f;
//		cmd.file_size[0]=keyfile_size[0];
//		cmd.file_size[1]=keyfile_size[1];
//		cmd.app_id = 0x95;
//		cmd.add_authority = ADD_KEY_AUTHORITY;
//		cmd.REF[0] = 0xff;
//		cmd.REF[1] = 0xff;
//		ISO14443A_CreateKeyFile(&cmd);
//	}
//	else if (strncmp(dbg_cmd_split->arg[1], "df_Addkey_user_ex", strlen("df_Addkey_user_ex")) == 0)//DF增加用户密钥
//	{
//		CPU_CREATE_KEY_DATA_T key_cmd;
//		
//		memset(prj_num,0xff,16);
//		gen_key_cpu(card_uid,adf,ex_key_cnt,prj_num,key);
//		key_cmd.key_cnt = ex_key_cnt;
//		key_cmd.key_type = ex_key_type;
//		key_cmd.use_authority = DF_KEY_USE_AUTHORITY;
//		key_cmd.modify_authority = DF_KEY_MODIFY_AUTHORITY;
//		key_cmd.authority = DF_USER_AUTHEH_AFTER;
//		key_cmd.err_cnt = 0xff;
//		key_cmd.key_len = 16;
//		memcpy(key_cmd.key,key,key_cmd.key_len);
//		ISO14443A_Addkey(&key_cmd);
//	}
//	else if (strncmp(dbg_cmd_split->arg[1], "df_Addkey_admin_ex", strlen("df_Addkey_admin_ex")) == 0)//DF增加管理员密钥
//	{
//		CPU_CREATE_KEY_DATA_T key_cmd;
//		
//		memset(prj_num,0xff,16);
//		gen_key_cpu(card_uid,adf,4,prj_num,key);
//		key_cmd.key_cnt = 4;
//		key_cmd.key_type = ex_key_type;
//		key_cmd.use_authority = DF_KEY_USE_AUTHORITY;
//		key_cmd.modify_authority =DF_KEY_MODIFY_AUTHORITY;
//		key_cmd.authority = DF_ADMIN_AUTHEH_AFTER;
//		key_cmd.err_cnt = 0xff;
//		key_cmd.key_len = 16;
//		memcpy(key_cmd.key,key,key_cmd.key_len);
//		ISO14443A_Addkey(&key_cmd);
//	}
	else if (strncmp(dbg_cmd_split->arg[1], "df_ExAuthen_admin", strlen("df_ExAuthen_admin")) == 0) //用管理员密钥认证
	{
		//leelen默认的外部认证密钥号为3,密钥通过gen_key_cpu计算得出
		memset(key,0xff,16);
		memset(prj_num,0xff,16);
		gen_key_cpu(card_uid,adf,4,prj_num,key);//获取密钥
//		debug("cauculate key:");
//		debug_hex(key,16);
		ExAuthenticate(4,key);
	}
	else
	{
		debug("cmd param error!\r\n");
	}
}

#include "card_rw_drv.h"
DEBUG_CMD_FN(card_app)
{
	dbg_cmd_split_t *dbg_cmd_split;

	dbg_cmd_split = get_dbg_cmd_split();
	if(CARD_READ == 0)
	{
		debug("this device not support card function!\r\n");
		return;
	}
	if (strncmp(dbg_cmd_split->arg[1], "card_ic_rw_block", strlen("card_ic_rw_block")) == 0)
	{
		IC_BLOCK_RW_STR ic_block_rw;
		CARD_DATA_TYPE date_type = DATA_USER;
		uint8_t ic_write_data[16]={0xea,0xb9,0x25,0x60,0x42,0xfe,0x81,0xc8,0xfb,0xa3,0xd1,0x7f,0xe7,0x24,0x63,0x74};
		
		if (strncmp(dbg_cmd_split->arg[2], "read", strlen("read")) == 0)
		{
			//读，使用指定密钥及密钥号
			#if 0
			ic_block_rw.cmd_aux = IC_BLOCK_READ;//IC_BLOCK_WRITE;
			scan_ISO14443A_card ( ic_block_rw.uid );
			ic_block_rw.block_cn  = 0x20;
			ic_block_rw.key_cn = 0x60;
			memset(prj_num,0xff,16);
			gen_key_ic(ic_block_rw.key_cn-0x60,ic_block_rw.uid,ic_block_rw.block_cn>>2,prj_num,ic_block_rw.key);
//			debug("key:");
//			debug_hex(ic_block_rw.key,6);
			#endif

			//读,自动使用leelen默认参数
			#if 1
			ic_block_rw.cmd_aux = IC_BLOCK_READ;//IC_BLOCK_WRITE;
			scan_ISO14443A_card ( ic_block_rw.uid );
			ic_block_rw.block_cn  = 0x20;
			ic_block_rw.key_cn = 0x0;
			#endif
		}
		
		else if (strncmp(dbg_cmd_split->arg[2], "write", strlen("write")) == 0)
		{
			//写，使用指定密钥及密钥号
			#if 0
			ic_block_rw.cmd_aux = IC_BLOCK_WRITE;
			scan_ISO14443A_card ( ic_block_rw.uid );
			ic_block_rw.block_cn  = 0x20;
			ic_block_rw.key_cn = 0x61;
			memset(prj_num,0xff,16);
			gen_key_ic(ic_block_rw.key_cn-0x60,ic_block_rw.uid,ic_block_rw.block_cn>>2,prj_num,ic_block_rw.key);
			memcpy(ic_block_rw.block_buffer,ic_write_data,16);
			#endif
			
			//写,自动使用leelen默认参数
			#if 1
			ic_block_rw.cmd_aux = IC_BLOCK_WRITE;
			scan_ISO14443A_card ( ic_block_rw.uid );
			ic_block_rw.block_cn  = 0x20;
			ic_block_rw.key_cn = 0x0;
			memcpy(ic_block_rw.block_buffer,ic_write_data,16);
			#endif
		}
		else
		{
			debug("param error!\r\n");
			return ;
		}
		
		
		card_ic_rw_block(&ic_block_rw,date_type);
		
		if(ic_block_rw.result == IC_SUCCESS)
		{
			if(ic_block_rw.cmd_aux == 0x12)//读
			{
				debug("read data:\r\n");
				debug_hex(ic_block_rw.block_buffer,16);
			}
			else if(ic_block_rw.cmd_aux == 0x11)//写
			{
				debug("write data success!\r\n");
			}
		}
		else
		{
			debug("write/read reeor,error code:%x\r\n",ic_block_rw.result_expand);
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "cpu_internal_auth", strlen("cpu_internal_auth")) == 0)
	{
		CPU_IN_AUTH_T cpu_in_auth;
		
		//指定adf，密钥，
		#if 0
		scan_ISO14443A_card ( cpu_in_auth.uid );
		cpu_in_auth.adf[0] = 0x10;
		cpu_in_auth.adf[1] = 0x05;
		cpu_in_auth.key_cn = 0x05;
		cpu_in_auth.key_len = 16;
		memset(prj_num,0xff,16);
		gen_key_cpu(cpu_in_auth.uid,cpu_in_auth.adf,cpu_in_auth.key_cn,prj_num,cpu_in_auth.key);
		#endif
		
		//自动用leelen参数进行验证
		#if 1
		scan_ISO14443A_card ( cpu_in_auth.uid );
		cpu_in_auth.adf[0] = 0x00;
    cpu_in_auth.adf[1] = 0x00;
		#endif
		cpu_internal_auth(&cpu_in_auth);
		debug("cpu_internal_auth result = %x %x\r\n",cpu_in_auth.result,cpu_in_auth.result_expand);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "card_cpu_w_ef", strlen("card_cpu_w_ef")) == 0)
	{
		uint16_t i;
		CPU_RW_T cpu_rw;
		
		scan_ISO14443A_card ( cpu_rw.uid );
		cpu_rw.cmd_aux = 0x21;
		cpu_rw.key_cn = 0x02;
		
		cpu_rw.adf[0] = 0x10;
		cpu_rw.adf[1] = 0x05;
		cpu_rw.ef[0] = 0x00;
		cpu_rw.ef[1] = 0x02;
		cpu_rw.ef_type[0] = 0x2a;
		cpu_rw.ef_type[1] = 0x20;
		cpu_rw.addr_start = 1;
		cpu_rw.key_len = 16;
		memset(prj_num,0xff,16);
		gen_key_cpu(cpu_rw.uid,cpu_rw.adf,cpu_rw.key_cn,prj_num,cpu_rw.key);
		cpu_rw.len = CPU_RWBUF_MAX;
		for(i=0;i<CPU_RWBUF_MAX;i++)
		{
			cpu_rw.data[i]=i;
		}
		card_cpu_rw_ef(&cpu_rw);
		debug("card_cpu_w_ef result = %x %x\r\n",cpu_rw.result,cpu_rw.result_expand);
	}
	else if (strncmp(dbg_cmd_split->arg[1], "card_cpu_r_ef", strlen("card_cpu_r_ef")) == 0)
	{
		CPU_RW_T cpu_rw;
		
		scan_ISO14443A_card ( cpu_rw.uid );
		cpu_rw.cmd_aux = 0x22;
		cpu_rw.key_cn = 0x01;
		
		cpu_rw.adf[0] = 0x10;
		cpu_rw.adf[1] = 0x05;
		cpu_rw.ef[0] = 0x00;
		cpu_rw.ef[1] = 0x02;
		cpu_rw.ef_type[0] = 0x2a;
		cpu_rw.ef_type[1] = 0x20;
		cpu_rw.addr_start = 1;
		cpu_rw.key_len = 16;
		memset(prj_num,0xff,16);
		gen_key_cpu(cpu_rw.uid,cpu_rw.adf,cpu_rw.key_cn,prj_num,cpu_rw.key);
		cpu_rw.len = CPU_RWBUF_MAX;

		card_cpu_rw_ef(&cpu_rw);
		debug("card_cpu_r_ef result = %x %x\r\n",cpu_rw.result,cpu_rw.result_expand);
		if(cpu_rw.result == 1)
		{
			debug_hex(cpu_rw.data,cpu_rw.len);
		}
		else
		{
			debug("card_cpu_r_ef error!\r\n");
		}
	}
	else if (strncmp(dbg_cmd_split->arg[1], "cpu_card_sender", strlen("cpu_card_sender")) == 0)
	{
		CPU_SENDER_CARD_T cpu_sender_card;
		scan_ISO14443A_card ( cpu_sender_card.uid );
		cpu_sender_card.adf[0] = 0x0;
		cpu_sender_card.adf[1] = 0x0;
		cpu_card_sender(&cpu_sender_card);
		debug("cpu_card_sender result: %x\r\n",cpu_sender_card.result);
	}
	else
	{
		debug("cmd param error!\r\n");
	}
}

DEBUG_CMD_FN(card_ic_sender)
{
	dbg_cmd_split_t *dbg_cmd_split;

	dbg_cmd_split = get_dbg_cmd_split();
	if(CARD_READ == 0)
	{
		debug("this device not support card function!\r\n");
		return;
	}
	if (strncmp(dbg_cmd_split->arg[1], "ic_sender", strlen("ic_sender")) == 0)
	{
//		uint8_t admin[6]={0xff,0xff,0xff,0xff,0xff,0xff};
		IC_SENDER_CARD_T card_info;
		
//		scan_ISO14443A_card(card_info.uid);
//		card_info.admin_key_type = 0x60;
//		memcpy(card_info.admin_key,admin,6);
//		card_info.block = 7;
		
		card_info.cmd_aux = AUTO_SEND;
		scan_ISO14443A_card(card_info.uid);
		ic_card_sender ( &card_info );
	}
	if (strncmp(dbg_cmd_split->arg[1], "ic_authen", strlen("ic_authen")) == 0)
	{
		uint8_t result;
		uint8_t admin1[6]={0xa1,0xa2,0xa3,0xa4,0xa5,0xa6};
		uint8_t admin[6]={0xff,0xff,0xff,0xff,0xff,0xff};

		scan_ISO14443A_card(card_uid);
		result = ISO14443A_Authen(card_uid,1,admin1,0x60);
		if(result)
		{
			debug("ISO14443A_Authen error code = %x!\r\n",result);
			scan_ISO14443A_card(card_uid);
			result = ISO14443A_Authen(card_uid,1,admin,0x60);
			if(result)
		    {
				debug("ISO14443A_Authen error code = %x!\r\n",result);
			}
			else
			{
				debug("ISO14443A_Authen success..!\r\n");
			}
		}
		else
		{
			debug("ISO14443A_Authen success...!\r\n");
		}
	}
}



/*---------------------------------------------------------------------------*/

