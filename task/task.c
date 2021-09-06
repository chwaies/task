
/**************************************************************************
  * @brief : task system 
  * @author : chWaies
  * @copyright :  
  * @version :v1.0
  * @note : 
  * @history : 
*/

#include "taskconfig.h"
#include "task.h"
#include "task_mem.h"
/* time */
#define timeRe_t 		 int8_t
#define TASK_EVENT_BIT   1U

typedef struct taskLink
{
	Task_Id task_id;
	Task_Event task_event;
	Task_SysEvent task_sysevent;
	
	Task_Event task_con_event;
	task_event_l_t* task_EventBase;
	task_event_l_t* systask_EventBase;
	struct taskLink* taskPrev;
	struct taskLink* taskNext;
}taskLink_t;

typedef struct taskTime
{
	TASK_TIMER_SAVE_TYPE tasktime;
	TASK_TIMER_SAVE_TYPE prevtime;
	//TASK_TIMER_SAVE_TYPE errortime;
	Task_Function task;
}taskTime_t;

typedef struct taskTimerRec
{
	Task_Id task_id;
	Task_Event task_event;
	uint16_t timeout;
	int16_t cur;
	taskType task_type;
//  uint16_t reloadTimeout;
} taskTimerRec_t;


typedef struct taskMessage
{
	Task_Event msgEvent;
	taskType task;
	void* data;
	struct taskMessage* nextMess;
}taskMessage_t;


typedef taskTimerRec_t TimerMan[TASK_TIME_SIZEO];

/* Message queue */
static taskMessage_t* taskMesTail = NULL;
static taskMessage_t* taskMesfront = NULL;

/* task time management */
static TimerMan timerman;

/* task count calculate */
static uint16_t timerCount = 0;

static Task_Id taskLowId = 0xFF;
static taskLink_t taskHight =		 \
{									 \
.task_id = 0xff,					 \
.task_event = 0x00U,                 \
.task_con_event = 0x00U,             \
.task_sysevent = 0,					 \
.task_EventBase = NULL,              \
.systask_EventBase = NULL, 			 \
.taskNext = NULL,					 \
.taskPrev = NULL,					 \
};

/* task memory management base address */
uint8_t taskRegheap[TASK_REG_HEAP_SIZE];

/* task memory management handle */
hpSeBind tkhpHandler;

/* System heart */
static TASK_VOLE uint32_t taskCnt;

/* stm32 timer description handle */
extern TIM_HandleTypeDef htim6;

/* The time used before the task runs */
static	TASK_VOLE uint32_t ticknow = 0;

/* The time elapsed after the task has run */
static	TASK_VOLE uint32_t tickprev = 0;


//Application time node
static timeRe_t task_new_timeSLx(void);
static timeRe_t task_find_timer( uint8_t task_id, uint16_t event_flag );
static timeRe_t task_find_timer_prev( uint8_t task_id, uint16_t event_flag );
static timestatus_t task_add_timer( taskType task_Able, uint16_t event_flag, uint16_t timeout );
static timestatus_t task_del_timer( uint8_t task_id, uint16_t event_flag);
TASK_ErrorStatus task_remove_timer( Task_Id taskID, Task_Event event_flag);
static void task_stop_timer_ax( Task_Id taskID,Task_Event delect_event_id, Task_Event* event_flag);


static uint32_t task_get_cnt(void);
static void task_scheduling_process(void * arg);
static Task_Event task_event_proc(taskType task , Task_Event proevent);
static void task_clr_event(taskType task_type ,task_event_l_t* event_ll, Task_Event_cb tk_pro_cb ,Task_Event set_event );
/* message */
static taskMessFlag task_init_msg(void);
static void task_msg_exe(void);
 /**
 * Application time node
 * @param void
 * @return timeRe_t
 */
static timeRe_t task_new_timeSLx(void)
{
	int index = timerman[1].cur;
	if (index != 0)
		timerman[1].cur = timerman[index].cur;
	return index;
}


/**
 * Initialize the event management table
 * @param void
 * @return  none
 */
void task_time_init(void)
{
	for(int i = 1; i < TASK_TIME_SIZEO-1;i++){
		timerman[i].cur = i+1;
	}
	timerman[0].cur = -1;
	timerman[TASK_TIME_SIZEO-1].cur = 0;
}




/**
 * Initialize the message queue
 * @param void
 * @return  none
 */
taskMessFlag task_init_msg(void)
{
	taskMesTail = taskMesfront = (taskMessage_t*)task_new_m(&tkhpHandler,sizeof(taskMessage_t));
	if (taskMesTail != NULL){
		taskMesTail->msgEvent = 0;
		taskMesTail->data = NULL;
		taskMesTail->task = NULL;
		taskMesTail->nextMess = NULL;
		return mes_initSucc;
	}
	return mes_initErr;
}

 


/**
 * task Register a Task
 * @param RegTask - Registered function pointer address , 
 * @param taskid -  Priority of registration,Can also be called task id
 * @return  taskType - Returns the address of the task handle,
 *			this parameter is used to facilitate management
 */
taskType task_reg_app(Task_Id taskid)
{
	/* For searching */
	taskLink_t* pTaskHight = &taskHight;
	
	/* Determine whether to initialize the registry */
	if (taskid != 0){
		/* Application node is used to save liao */
		taskLink_t* newtask = (taskLink_t*)task_new_m(&tkhpHandler,sizeof(taskLink_t));
		if (newtask != NULL){
			taskLink_t* taskIterat;
			taskLink_t* taskIteratNext = NULL;
			newtask->task_id = taskid;	// taskid;
			newtask->task_event = 0;
			newtask->task_sysevent = 0;
			newtask->task_con_event = 0;
			newtask->task_EventBase = NULL;
			newtask->systask_EventBase = NULL;
			newtask->taskNext = NULL;
			newtask->taskPrev = NULL;
			
			/* Get small priority tasks. */
			taskLowId > newtask->task_id ? taskLowId = newtask->task_id : taskLowId;
			
			/* Iteratively find the right position to insert into the linked list. */
			for (taskIterat = pTaskHight;															\
				taskIterat->taskNext != NULL && newtask->task_id <= taskIterat->taskNext->task_id;  \
				taskIterat = taskIterat->taskNext)
			{

			}
			/* Save the next node of the iteration */
			taskIteratNext = taskIterat->taskNext;
			
			/* Make a link. */
			if (taskIteratNext == NULL)
			{
				taskIterat->taskNext = newtask;
				newtask->taskPrev = taskIterat;
			}
			else
			{
				newtask->taskNext = taskIterat->taskNext;
				newtask->taskPrev = taskIterat;
				taskIteratNext->taskPrev = newtask;
				taskIterat->taskNext = newtask;
			}
			return (taskType)newtask;
		}
		else
		{
			task_TEST();
		}
	}
	else
	{
		task_TEST();
	}
	return NULL;
}


/**
 * Get the lowest priority of the task
 * @param void
 * @return Task_Id
 */
Task_Id task_get_lowid(void)
{
	return taskLowId;
}




/**
 * Delete a registered task , Usually refers to the layer
 * This function has not realized the release of memory, please do not use it!!
 * @param RegTask - The address of the incoming task handle
 * @return TASK_ErrorStatus - Can it be deleted?
 */
TASK_ErrorStatus task_del_app(taskType* RegTask )
{
	if (RegTask != NULL){
		/* Used to traverse the entire task system. */
		taskLink_t* pxPrevDelect = &taskHight;
		
		/* Iteratively find the task to be deleted. */
		for (; pxPrevDelect != NULL && pxPrevDelect->taskNext->task_id != (*RegTask)->task_id; pxPrevDelect = pxPrevDelect->taskNext)
		{
		}
		/* Whether to find the task. */
		if (pxPrevDelect == NULL){
			/* No task to delete was found. */
			task_TEST();
		}
		else{
			/* Start deleting task. */
			taskLink_t* pxdelect = pxPrevDelect->taskNext;
			if (pxdelect->taskNext != NULL){
				pxPrevDelect->taskNext = pxdelect->taskNext;
				pxdelect->taskNext->taskPrev = pxPrevDelect;
			}
			else{
				pxPrevDelect->taskNext = NULL;
			}
			/* Delete tasks in the system. */
			task_del_m(&tkhpHandler,pxdelect);
			
			/* Prevent wild pointers */
			*RegTask = NULL;
			
			/* Return success after deletion. */
			return TASK_SUCCESS;
		}
	}
	else{
		task_TEST();
	}
	/* failed to delete. */
	return TASK_ERROR;
}



/**
 * Get the reference of the task system message
 * @param task
 * @param data - The data to be sent, the life cycle of the function cannot be partial
 * @return TASK_ErrorStatus
 */
taskMessFlag task_send_msg(taskType task,Task_Event set_event,void* data)
{
	if (task != NULL && set_event & TASK_SYSEVENTMAX)
	{
#if USE_RUN_INIT
		if (taskMesTail == NULL && taskMesfront == NULL){
			task_init_msg();
		}
#endif
		if (taskMesTail != NULL && taskMesfront != NULL){
			taskMessage_t* newMess = (taskMessage_t*)task_new_m( &tkhpHandler ,sizeof(taskMessage_t));
			newMess->msgEvent = set_event;
			newMess->data = data;
			newMess->task = task;
			newMess->nextMess = NULL;
			taskMesTail->nextMess = newMess;
			taskMesTail = newMess;
			return mes_sendSucc;
		}

	}
	return mes_sendErr;
}





/**
 * Get the reference of the task system message
 * @param task_id
 * @return taskMessage_t*
 */
taskMessFlag task_get_msg(taskType task, void** res)
{
	(*res) = NULL;

	if (task != NULL){
		if (taskMesTail  == NULL && taskMesfront == NULL)
			return mes_resError;
		else{
			if (taskMesTail != taskMesfront){
				taskMessage_t* pxMess = taskMesfront->nextMess;
				if (pxMess->task == task){
					(*res) = pxMess->data;
					taskMesfront->nextMess = pxMess->nextMess;
					if (pxMess == taskMesTail){
						taskMesTail = taskMesfront;
						taskMesTail->nextMess = NULL;
					}
					task_del_m(&tkhpHandler,pxMess);
					return mes_getSucc;
				}
			}
		}
	}
	return mes_getErr;
}



static void task_msg_exe(void)
{
	if (taskMesTail != taskMesfront){
		taskMessage_t* p_msg_h = taskMesfront->nextMess;
		task_set_event(p_msg_h->task,p_msg_h->msgEvent);
	}
}




/**
 * Find the previous node of a timed task
 * @param task_id
 * @param event_flag- find task event
 * @return timeRe_t
 */
timeRe_t task_find_timer_prev( uint8_t task_id, uint16_t event_flag )
{
	timeRe_t searchTime = timerman[0].cur;

	if (timerman[searchTime].task_id == task_id && timerman[searchTime].task_event == event_flag){
		searchTime = 0;
		return searchTime;
	}

	timeRe_t searchPrevtime = 0;
	while (searchTime != -1 ){
		if (timerman[searchTime].task_id == task_id && timerman[searchTime].task_event == event_flag){
			break;
		}
		searchPrevtime = searchTime;
		searchTime = timerman[searchTime].cur;

	}
	if (searchTime == -1){
		searchPrevtime = -1; 
	}
	return searchPrevtime;

}





/**
 * Find a scheduled task
 * @param task_id
 * @param event_flag- find task event
 * @return timeRe_t
 */
timeRe_t task_find_timer( uint8_t task_id, uint16_t event_flag )
{
	
	timeRe_t srchTimer = timerman[0].cur;
	
	if(timerman[1].cur == 0){
		return 0;
	}
	
	while(srchTimer != -1){
		if(task_id == timerman[srchTimer].task_id &&
			event_flag == timerman[srchTimer].task_event ){
			break;
		}
		srchTimer = timerman[srchTimer].cur;
	}
	
	return srchTimer;
}






/**
 * add a timer event
 * @param task_id
 * @param event_flag - add task event
 * @param timeout - Time of event
 * @return timestatus_t
 */
timestatus_t task_add_timer( taskType task_Able, uint16_t event_flag, uint16_t timeout )
{
	/*Search whether there are exactly the same nodes.*/
	timeRe_t srchTimer = task_find_timer( task_Able->task_id,event_flag );
	
	if(srchTimer == 0){
		//mem full
		return timerror;
	}
	
	/* If there is no exactly the same node, then create a new one. */
	if(srchTimer == -1){
			/* Apply for a node. If it is zero, return an error code. */
			timeRe_t newtime = task_new_timeSLx();
			if (newtime == 0)
			{
				/* Oh, something went wrong! */
				return timerror;
			}
			timerman[newtime].task_event = event_flag;
			timerman[newtime].task_id = task_Able->task_id;
			timerman[newtime].timeout = timeout;
			timerman[newtime].task_type = task_Able;
			/*If it is the first node.*/
			if (timerman[0].cur == -1)
			{
				timerman[newtime].cur = -1;
			}
			else
			{
				timerman[newtime].cur = timerman[0].cur;
			}
			timerman[0].cur = newtime;
	}
	else{	
		/* Find the exact same node, then add on the original basis.*/
		timerman[srchTimer].timeout += timeout;
	}
	/*Record the number of nodes.*/
	timerCount++;
	/* Return operation success flag. */
	return timeradd;
}







/**
 * Delete a timer event
 * @param task_id
 * @param event_flag-Deleted task event
 * @return timestatus_t
 */
timestatus_t task_del_timer( uint8_t task_id, uint16_t event_flag)
{
	timeRe_t prevtime = task_find_timer_prev(task_id,event_flag);
	if (prevtime == -1){
		return timerror;
	}
	timeRe_t delect = timerman[prevtime].cur;//delect

	if (timerman[delect].cur == -1){
		//if head
		timerman[prevtime].cur = -1;
	}
	else{
		timerman[prevtime].cur = timerman[delect].cur;
	}
	
	timerman[delect].cur = timerman[1].cur;
	timerman[1].cur = delect;
	timerCount--;
	
	return timerDelect;
}






/**
 * task_get_time_size
 * @param void
 * @return size
 */
uint16_t task_get_time_size(void)
{
	return timerCount;
}




/**
 * Initialize the components of the task
 * @param void
 * @return none
 */
void task_init( void )
{
	
	/* Initialization time memory management table */
	task_time_init();
	
	/* Initialize heap space for task management. */
	initHp(&tkhpHandler, taskRegheap,TASK_REG_HEAP_SIZE);
	
	/* Initialize the message queue */
	task_init_msg( );
	
	/* Initialize your task */
	USER_TASKINIT();
	
}

/**
 * Generate a common event
 * @param task_type - Generate events on that task
 * @param tk_pro_cb - Event callback function
 * @param set_event - Hotkey code,Event number
 * @return none
 */
void task_new_genEx(taskType task_type , Task_Event_cb tk_pro_cb ,Task_Event set_event )
{
	task_clr_event(task_type ,task_type->task_EventBase,tk_pro_cb , set_event );
}


/**
 * Generate a system event,Its essence is no different from ordinary events, but in order to distinguish managers
 * @param task_type - Generate events on that task
 * @param tk_pro_cb - Event callback function
 * @param set_event - Hotkey code,Event number
 * @return none
 */
void task_new_sysEx(taskType task_type , Task_Event_cb tk_pro_cb ,Task_Event set_event )
{
	task_clr_event(task_type ,task_type->systask_EventBase,tk_pro_cb , set_event );
}



/**
 * Register event, used to save the type of event,In order to facilitate the call,please use task_new_genEx,task_new_sysEx
 * @param task_type - Generate events on that task
 * @param tk_pro_cb - Event callback function
 * @param set_event - Hotkey code,Event number
 * @return none
 */
void task_clr_event(taskType task_type ,task_event_l_t* event_ll, Task_Event_cb tk_pro_cb ,Task_Event set_event )
{
	task_event_l_t* event_l_pre;
	task_event_l_t* event_l;
	if (task_type != NULL && tk_pro_cb && set_event){
		task_event_l_t* new_event;
		
		task_type->task_con_event |= set_event;
		
		event_l = event_ll;
		
		event_l_pre = NULL;
		
		new_event = (task_event_l_t*)task_new_m(&tkhpHandler,sizeof(task_event_l_t));
		
		if (new_event != NULL){
			new_event->taskeventprocess = tk_pro_cb;
			new_event->task_event = set_event;
			new_event->next = NULL;

			while(event_l != NULL && event_l->task_event < set_event){
				event_l_pre = event_l;
				event_l = event_l->next;
			}
			
			if (task_type->task_EventBase == NULL || event_l_pre == NULL){
				new_event->next = task_type->task_EventBase;
				task_type->task_EventBase = new_event;
			}
			else {
				event_l_pre->next = new_event;
				new_event->next = event_l;
			}
			
/*ascending order*/			
//			while(event_l != NULL && event_l->task_event > set_event){
//				event_l_pre = event_l;
//				event_l = event_l->next;
//			}
			
//			if (event_l == NULL || event_l_pre == NULL){
//				new_event->next = task_type->task_EventBase;
//				task_type->task_EventBase = new_event;
//			}
//			else{
//				
//				event_l_pre->next = new_event;
//				new_event->next = event_l;
//			}

		}
		
	}
}


/**
 * Set an event
 * @param taskId- Task to be set
 * @param taskEvent-Event to be set
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_set_event(taskType task_type , Task_Event taskEvent)
{
	if (task_type != NULL && taskEvent & 0xffffU){
		task_type->task_event |= taskEvent;
		return (TASK_SUCCESS);
	}
	return (TASK_ERROR);
}





/**
 * Clear an event
 * @param task_Able
 * @param task_sysevent
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_cls_ordEx(taskType task_type, Task_Event event_flag )
{
  if ( task_type !=  NULL && task_type->task_event >= event_flag){
	
    task_type->task_event &= (~event_flag);  // Stuff the event bit(s)

    return ( TASK_SUCCESS );
  }
  
  return ( TASK_ERROR );

}







/**
 * set system events
 * @param task_Able
 * @param task_sysevent
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_set_sysex(taskType task_type , Task_SysEvent task_sysevent)
{
	if (task_type != NULL && task_sysevent & TASK_SYSEVENTMAX){
		task_type->task_sysevent |= task_sysevent;
		return ( TASK_SUCCESS );
	}
	return (TASK_ERROR);
}


/**
 * clear system events
 * @param task_Able
 * @param task_sysevent
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_cls_sysex(taskType task_type , Task_SysEvent task_sysevent)
{
	if (task_type != NULL && task_sysevent & TASK_SYSEVENTMAX){
		task_type->task_sysevent &= (~task_sysevent);
		return ( TASK_SUCCESS );
	}
	return (TASK_ERROR);
}



/**
 * get system events
 * @param task_Able
 * @return TASK_ErrorStatus
 */
Task_SysEvent task_get_sysEx(taskType task_type)
{
	if (task_type != NULL){
		return task_type->task_sysevent;
	}
	return 0;
}



/**
 * Update system task time
 * @param updateTime- Number of updates
 * @return void
 */
void task_update_time( uint32_t updateTime )
{
  timeRe_t srchTimer;
	
  // Look for open timer slot
  if ( timerman[0].cur != -1 ){
    // Add it to the end of the timer list
    srchTimer = timerman[0].cur;

    // Look for open timer slot
    while ( srchTimer != -1 ){
      timeRe_t freeTimer = 0;
       
      if (timerman[srchTimer].timeout <= updateTime)
      {
        timerman[srchTimer].timeout = 0;
      }
      else
      {
        timerman[srchTimer].timeout -=  updateTime;
      }
      // When timeout or delete (event_flag == 0)
      if ( timerman[srchTimer].timeout == 0 || timerman[srchTimer].task_event == 0 )
      {
        freeTimer = srchTimer;
      }
			
	  srchTimer = timerman[srchTimer].cur;
			
      if ( freeTimer ){
        if ( timerman[freeTimer].timeout == 0 ){
          task_set_event( timerman[freeTimer].task_type, timerman[freeTimer].task_event );
        }
        task_del_timer( timerman[freeTimer].task_id, timerman[freeTimer].task_event  );
      }
    }
  }
}





/**
 * Get count value
 * @param task_Able
 * @param void
 * @return taskCnt
 */
uint32_t task_get_cnt()
{
	return taskCnt;
}




/**
 * Clear count value
 * @param void
 * @return none
 */
void task_cls_cnt_time()
{
	taskCnt = 0;
}




/**
 * System operation function!
 * @param void
 * @return none
 */
void task_run_system(void)
{
	tickprev = task_get_cnt();

	task_update_time(ticknow);
	
	taskLink_t* searActive = taskHight.taskNext;

	while (searActive != NULL && searActive->task_event == 0 && searActive->task_sysevent == 0){
		searActive = searActive->taskNext;
	}
	
	if (searActive != NULL){
		task_scheduling_process((void*)searActive);
	}
	task_msg_exe();
	ticknow = task_get_cnt() - tickprev;
	
	while(ticknow == 0){
		//compensate time 
		ticknow = task_get_cnt() - tickprev;
	}
	task_cls_cnt_time();
}



/**
 * Endless loop ,Provided to external calls!
 * @param void
 * @return none
 */
void task_run(void)
{
	for(;;){
		task_run_system();
	}
}






/**
 * Start a timer for system events
 * @param task_Able
 * @param event_flag:
 * @param timeout_value: The time the task runs (ms)
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_start_timer(taskType task_Able, Task_Event event_flag, uint16_t timeout_value )
{
	
	if (task_Able != NULL){
		timestatus_t  newTimer;
		// Add timer
		newTimer = task_add_timer( task_Able, event_flag, timeout_value );
		//Return status
		return ( (newTimer != NULL) ? TASK_SUCCESS : TASK_ERROR );
		
	}
	/* Error flag is returned. */
	return TASK_ERROR;
}




/**
 * This is called by task_stop_timer_ax! 
 * @param taskID
 * @param event_flag
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_remove_timer( Task_Id taskID, Task_Event event_flag)
{

	timestatus_t  newTimer;
	// Add timer
	newTimer = task_del_timer( taskID, event_flag);

	return ( (newTimer != NULL) ? TASK_SUCCESS : TASK_ERROR );
}



/**
 * Guarantee will be deleted , Please use it in the system message! 
 * @param taskID
 * @param delect_event_id:
 * @param event_id: Current event
 * @return none
 */
void task_stop_timer_ax( Task_Id taskID,Task_Event delect_event_id, Task_Event* event_flag)
{
	if( IS_TASK_EVENT(*event_flag , delect_event_id) ){
		CLEAR_TASK_EVENT(*event_flag,delect_event_id);
	}
	else{
		task_remove_timer(taskID,delect_event_id);
	}
}



/**
 * Stop the software timer
 * @param task_type
 * @param des_event:Stopped event
 * @return none
 */
void task_stop_timer(taskType task_type,Task_Event des_event)
{
	if (task_type != NULL){
		task_stop_timer_ax(task_type->task_id,des_event,&task_type->task_event);
	}
}


#if USE_TASK_HANDLER
/**
 * Get task information
 * @param void*
 * @return none
 */
task_handler* task_get_handler(void* arg)
{
	task_handler* tmpHp = NULL ;
	tmpHp = (task_handler*)task_new_m(&tkhpHandler,sizeof(task_handler));
	if(tmpHp != NULL){
		tmpHp->task = (taskType)arg;
		tmpHp->cou_event = 0;
	}
	return tmpHp;
}


/**
 * Delete task information,This function is used in pairs with obtaining task information
 * @param void*
 * @return none
 */
void task_des_handler(task_handler* tk_hd)
{
	if ( tk_hd != NULL){
		task_del_m(&tkhpHandler,tk_hd);
	}
}


#endif



/**
 * Event handler,This function is called by task management
 * @param task
 * @param proevent
 * @return none
 */
static Task_Event task_event_proc(taskType task , Task_Event proevent)
{
	Task_Event conevent = task->task_con_event;
	task_event_l_t* act = task->task_EventBase;
	Task_Event bit = 0;
	while( proevent ){
		
		if ( conevent & (proevent & (Task_Event)1U) ) {
			act->taskeventprocess((task_u*)task);
			break;
		}
		proevent >>= 1;
		act = act->next;
		bit++;
	}
	return bit;

}


/**
 * System scheduler,This function is called by task management
 * @param task
 * @param proevent
 * @return none
 */
static void task_scheduling_process(void * arg)
{
#if USE_TASK_HANDLER
	#error "The idea has not been realized"
	task_handler* tk_hd = task_get_handler(arg);
	if(tk_hd == NULL)
		return;
#endif
	Task_Event proBit = 0;
	/* sys event handling.*/
	proBit = task_event_proc( ((taskType)arg),((taskType)arg)->task_sysevent );
	
	/* clear event*/
	task_cls_sysex( ((taskType)arg),(Task_Event)TASK_EVENT_BIT << proBit );
	
	/* Ordinary event handling. */
	proBit = task_event_proc( ((taskType)arg),((taskType)arg)->task_event );
	task_cls_ordEx(((taskType)arg),(Task_Event)TASK_EVENT_BIT << proBit);
#if USE_TASK_HANDLER
	task_des_handler(tk_hd);
#endif
}



/**
 * Provide the heart of the system 
 * @param void
 * @return none
 */
void task_info(void)
{
	taskCnt++;
}



