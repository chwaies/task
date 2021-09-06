#ifndef TASK_H
#define TASK_H
#include <stdint.h>
/*
1,My compiler will garble when typing Chinese, so translate it in English
2,Don't think that writing English is pretending to be force,English is good
*/

typedef unsigned char Task_Id;
typedef unsigned short Task_Event;
typedef unsigned short Task_SysEvent;

#define TASK_VOLE 	volatile 

#define IS_TASK_EVENT(REG,BIT)    	     ((REG) & (BIT))
#define is_event(arg1,arg2,con)			 (IS_TASK_EVENT(arg1,arg2) ? (con |= arg2) : (con) )
#define CLEAR_TASK_EVENT(REG,BIT) 		 ((REG) ^= (BIT))
#define task_TEST()




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

typedef struct 
{
	Task_Id task_id;
	Task_Event task_event;
	Task_SysEvent task_sysevent;
}task_uhandler_t;

typedef task_uhandler_t  task_u;
typedef void(*Task_Function )(void*);
typedef void(*Task_Event_cb )(task_u* arg);

struct task_event_li;

typedef struct taskMan
{
	Task_Id task_id;
	Task_Event task_event;
	Task_SysEvent task_sysevent;
	Task_Event task_con_event;
	struct task_event_li * task_EventBase;
	struct task_event_li* systask_EventBase;
}taskMan_t;


typedef taskMan_t* taskType;

typedef struct task_event_li
{
	Task_Event task_event;
	Task_Event_cb taskeventprocess;
	struct task_event_li* next;
}task_event_l_t;

typedef struct 
{
	Task_Event cou_event;
	taskType task;
}task_handler;

void task_info(void);

void task_time_init(void);

void task_init( void );

/* User use task system related */
/* create task */
taskType task_reg_app(Task_Id taskid);

/* Get the lowest priority of the task */
Task_Id task_get_lowid(void);

/* delete task */
TASK_ErrorStatus task_del_app(taskType* RegTask );

/*task message API */
taskMessFlag task_send_msg(taskType task,Task_Event set_event,void* data);

taskMessFlag task_get_msg(taskType task, void** res);

/* Set time and event api */
uint16_t task_get_time_size(void);
/* Generating common events */
void task_new_genEx(taskType task_type , Task_Event_cb tk_pro_cb ,Task_Event set_event );

void task_new_sysEx(taskType task_type , Task_Event_cb tk_pro_cb ,Task_Event set_event );

TASK_ErrorStatus task_set_event(taskType task_type , Task_Event taskEvent);

TASK_ErrorStatus task_cls_ordEx(taskType xtarGet, Task_Event event_flag );

TASK_ErrorStatus task_set_sysex(taskType task_type , Task_SysEvent task_sysevent);

Task_SysEvent task_get_sysEx(taskType task_type);

TASK_ErrorStatus task_cls_sysex(taskType task_type , Task_SysEvent task_sysevent);

//The time unit of parameter three is ms 
TASK_ErrorStatus task_start_timer(taskType task_Able, Task_Event event_flag, uint16_t timeout_value );

void task_stop_timer(taskType task_type,Task_Event des_event);


void task_cls_cnt_time(void);

//Task scheduling related api
void task_update_time( uint32_t updateTime );

void task_run_system(void);

void task_run(void);
#if USE_TASK_HANDLER
task_handler* task_get_handler(void* arg);

void task_des_handler(task_handler* tk_hd);

#endif	/* USE_TASK_HANDLER */
#endif  /* TASK_H */
