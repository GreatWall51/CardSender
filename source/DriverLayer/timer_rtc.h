/******************************************************************************
* Copyright 2017-2022 545122859@qq.com
* FileName: 	 calendar_drv.h 
* Desc:
* 
* 
* Author: 	 LiuWeiQiang
* Date: 	 2017/10/14
* Notes: 
* 
* -----------------------------------------------------------------------------
* Histroy: v1.0   2017/10/14, LiuWeiQiang create this file
* 
******************************************************************************/
#ifndef _TIMER_RTC_H_     
#define _TIMER_RTC_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
/*---------------------- Constant / Macro Definitions -----------------------*/
#define TIME_ZONE 8
#ifndef time_t
#define time_t uint32_t
#endif


struct tm
{
    int tm_sec;   /* �� �C ȡֵ����Ϊ[0,59] */
    int tm_min;   /* �� - ȡֵ����Ϊ[0,59] */
    int tm_hour; /* ʱ - ȡֵ����Ϊ[0,23] */
    int tm_mday; /* һ�����е����� - ȡֵ����Ϊ[1,31] */
    int tm_mon;   /* �·� - ȡֵ����Ϊ[1,12] */
    int tm_year; /* ��ݣ���ֵ����ʵ����ݼ�ȥ1900 */
    int tm_wday; /* ���� �C ȡֵ����Ϊ[0,6]������0���������죬1��������һ���Դ����� */
    int tm_yday; /* ��ÿ���1��1�տ�ʼ������ �C ȡֵ����Ϊ[0,365]������0����1��1�գ�1����1��2��*/
    int tm_isdst;/* ����ʱ��ʶ����ʵ������ʱ��ʱ��tm_isdstΪ������ʵ������ʱ��ʱ��tm_isdstΪ0�����˽����ʱ��tm_isdstΪ����*/
};
typedef struct tm timer_rtc_t;
/*----------------------------- Global Typedefs -----------------------------*/
 
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 void second_irq_call_back(void);
 
 void set_timestamp(time_t time_cnt);
 time_t get_timestamp(void);
 uint8_t set_time_by_tm(timer_rtc_t *date_time);
 timer_rtc_t *get_time_by_tm(void);
 

#endif //_TIMER_RTC_H_
