/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : ctrl_io_drv.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年6月15日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年6月15日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "timer_drv.h"
#include "define.h" 
#include <string.h>
#include <stdlib.h>
#include "ctrl_io_drv.h" 
#include "hal_ctrl_io.h"  
#include "debug.h" 
/*----------------------- Constant / Macro Definitions -----------------------*/

/*---------------------- Constant / Macro Definitions -----------------------*/
enum CTRL_STATUS{
  STATUS_ON = 0,
  STATUS_OFF,
  STATUS_FLASH,
};
/*----------------------- Type Declarations ---------------------------------*/
typedef struct {
    uint32_t timeout;
    uint8_t status; //see LED_STATUS
    uint8_t period; // just use in status: STATUS_FLASH
    uint16_t timer;
}CTRL_CONTROL_SINGLE;

/*----------------------- Variable Declarations -----------------------------*/
  

static CTRL_CONTROL_SINGLE ctrl_io[CTRL_NUM];

/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/

/******************************************************************************
* Name: 	 LEDInit 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/06, Create this function by Kimson
 ******************************************************************************/
void ctrl_io_init()
{
    memset(ctrl_io,0,sizeof(ctrl_io));
    /* 硬件相关初始化 */
    ctrl_io_hw_init();
}

/******************************************************************************
* Name: 	 LedSet 
*
* Desc: 	设置指示灯状态 
* Param: 	 
            index: see LED_INDEX
            mode: see LED_MODE
            timeout(*10ms): 
               >0: will turn OFF after *timeout* time
               0: stay in the mode, and never timeout
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/12, Create this function by Kimson
 ******************************************************************************/
void ctrl_io_set(CTRL_INDEX index, CTRL_MODE mode, uint32_t timeout)
{
    if(index >= CTRL_NUM) return; //error...
    if(mode >= CTRL_MODE_NUM) return;
    
    ctrl_io[index].timeout = timeout/TIME_EVT_8/TIME_BASE;
    switch(mode)
    {
        case CTRL_MODE_FLASH_SLOW: //慢闪
        ctrl_io[index].status = STATUS_FLASH;
        ctrl_io[index].period = (TIME_FLASH_SLOW/TIME_EVT_8/TIME_BASE);
        ctrl_io[index].timer = 0;
        break;
        case CTRL_MODE_FLASH: 
        ctrl_io[index].status = STATUS_FLASH;
        ctrl_io[index].period = (TIME_FLASH/TIME_EVT_8/TIME_BASE);
        ctrl_io[index].timer = 0;
        break;
        case CTRL_MODE_OFF:
        CTRL_OFF(index);
        ctrl_io[index].status = STATUS_OFF;

        break;
        case CTRL_MODE_ON:
        CTRL_ON(index);
        ctrl_io[index].status = STATUS_ON;

        break;
        case CTRL_MODE_TOGGLE:
        if(IS_CTRL_ON(index))
        {
            CTRL_OFF(index);
            ctrl_io[index].status = STATUS_OFF;
        }
        else
        {
            CTRL_ON(index);
            ctrl_io[index].status = STATUS_ON;
        }
        break;
        default:break;
    }
}

uint8_t ctrl_io_get(uint8_t index)
{
    return ctrl_io[index].status;
}
/******************************************************************************
* Name: 	 LedRealTime 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/12, Create this function by Kimson
 ******************************************************************************/
void ctrl_io_real_time(void)
{
    uint32_t i;
    
    if(!IS_TIMER_EVT(TIME_EVT_8)) return;//1*TIME_BASE ms

    //run every TIME_EVT_8*TIME_BASE
    for(i = 0; i < CTRL_NUM; i++)
    {
        if(ctrl_io[i].status == STATUS_FLASH)
        {
            if(ctrl_io[i].timer) ctrl_io[i].timer--;
            if(ctrl_io[i].timer == 0)
            {
                if(IS_CTRL_ON(i))
                {
                    CTRL_OFF(i);
                }
                else
                {
                    CTRL_ON(i);
                }
                ctrl_io[i].timer = ctrl_io[i].period;                    
            }
        }
        
        if(ctrl_io[i].timeout)
        {
            ctrl_io[i].timeout--;
            if(ctrl_io[i].timeout == 0)
            {
               CTRL_OFF(i);
               ctrl_io[i].status = STATUS_OFF;   
            }
        }
    }
}
/*---------------------------------------------------------------------------*/

