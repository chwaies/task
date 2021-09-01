#ifndef TASK_H
#define TASK_H
#include "main.h"

/*
1,My compiler will garble when typing Chinese, so translate it in English
2,Don't think that writing English is pretending to be force,English is good
*/

typedef uint8_t Task_Id;
typedef unsigned short Task_Event;
typedef unsigned short Task_SysEvent;


#define IS_TASK_EVENT(REG,BIT)    	     ((REG) & (BIT))
#define CLEAR_TASK_EVENT(REG,BIT) 		 ((REG) ^= (BIT))
#define TASK_SYSEVENTMAX                 0XFFFF
#define task_TEST()




/* task message event */
#define TASK_SYSTEMEVENT							0x8000

typedef enum timestatus
{
	timerror = 0,
	timeradd,
	timerDelect
}timestatus_t ;

typedef enum
{
	mes_sendErr = 0,
	mes_sendSucc,
	mes_getErr,
	mes_getSucc,
	mes_initErr,
	mes_initSucc,
	mes_resError,
	mes_resSucc,
	mes_null
}taskMessFlag;

typedef enum
{
	TASK_ERROR = 0U,
	TASK_SUCCESS = !TASK_ERROR
} TASK_ErrorStatus;



typedef void(*Task_Function )(void*);

typedef struct taskMan
{
	Task_Id task_id;
	Task_Event task_event;
	Task_SysEvent task_sysevent;
	Task_Function taskProcess;
}taskMan_t;


typedef taskMan_t* taskAble;

void init_TimeRec(void);

void init_Tasks( void );

/* User use task system related */
taskAble task_RegisterTaskApp(Task_Function RegTask, Task_Id taskid);

Task_Id get_TaskLowId(void);

TASK_ErrorStatus delect_ResTaskApp(taskAble* RegTask );

//task message API
taskMessFlag send_TaskMessage(taskAble task, void* data);

taskMessFlag get_TaskMessage(taskAble task, void** res);

uint16_t get_Timersize(void);

TASK_ErrorStatus task_SetEvent(taskAble task_able , Task_Event taskEvent);

TASK_ErrorStatus task_ClearEvent(taskAble xtarGet, Task_Event event_flag );

TASK_ErrorStatus task_SysSetEvent(taskAble task_able , Task_SysEvent task_sysevent);

Task_SysEvent task_GetSysEvent(taskAble task_able);

TASK_ErrorStatus task_SysClearEvent(taskAble task_able , Task_SysEvent task_sysevent);

void timer_Update( uint32_t updateTime );

void task_RunSystem(void);

void run_Task(void);

TASK_ErrorStatus task_StartTimerEx(taskAble task_Able, Task_Event event_flag, uint16_t timeout_value );

TASK_ErrorStatus task_StopTimerEx( Task_Id taskID, Task_Event event_flag);

/* Guarantee will be deleted , Please use it in the system message!  */
void task_StopTimerGuara( Task_Id taskID,Task_Event delect_event_id, Task_Event* event_flag);

//uint8_t task_StopTimerConfirm(uint8_t taskID, uint16_t event_id , uint8_t messid);
uint32_t get_KernalCount(void);

void set_KernalCountClear(void);


#define task_TimedEnding(arg1,arg2,arg3) 		do {							\
											task_ClearEvent(arg1,arg2);			\
											task_StartTimerEx(arg1,arg2,arg3);	\
											return;								\
											}while(0U)




/* In order to reflect the priority, please add return at the end to end the function operation */
#define task_Ending(arg1,arg2) 				do {							    \
											task_ClearEvent(arg1,arg2);			\
											return;								\
											}while(0U)
											
#endif
