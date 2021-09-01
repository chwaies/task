
/**************************************************************************
  * @brief : task system 
  * @author : Waies
  * @copyright :  
  * @version :v1.0
  * @note : 
  * @history : 
*/

#include "taskconfig.h"
#include "task.h"
#include "string.h"

/* time */
#define timeRe_t int8_t


typedef struct taskLink
{
	Task_Id task_id;
	Task_Event task_event;
	Task_SysEvent task_sysevent;
	Task_Function taskProcess;
	struct taskLink* taskPrev;
	struct taskLink* taskNext;
}taskLink_t;

typedef struct taskTime
{
	uint32_t tasktime;
	uint32_t prevtime;
	uint32_t errortime;
	Task_Function task;
}taskTime_t;

typedef struct taskTimerRec
{
	Task_Id task_id;
	Task_Event task_event;
	uint16_t timeout;
	int16_t cur;
	taskAble task_able;
//  uint16_t reloadTimeout;
} taskTimerRec_t;


typedef struct taskMessage
{
	taskAble task;
	void* data;
	struct taskMessage* nextMess;
}taskMessage_t;


typedef taskTimerRec_t TimerMan[TIMESIZEO];

/* Message queue */
taskMessage_t* taskMesTail = NULL;
taskMessage_t* taskMesfront = NULL;

/* task time management */
static TimerMan timerman;

/* task message management table  */
//static taskMessage messageman;

/* task count calculate */
static uint16_t timerCount = 0;

static Task_Id taskLowId = 0xFF;
static taskLink_t taskHight =		 \
{									 \
.task_id = 0xff,					 \
.task_event = 0x00U,                 \
.taskNext = NULL,					 \
.taskPrev = NULL,					 \
.taskProcess = NULL,				 \
};

/* task memory management base address */
uint8_t taskRegheap[TASKREGHEAPSIZE];

/* task memory management handle */
hpSeBind tkhpHandler;

/* System heart */
static __IO uint32_t task_count;

/* stm32 timer description handle */
extern TIM_HandleTypeDef htim6;

/* The time used before the task runs */
static	__IO uint32_t ticknow = 0;

/* The time elapsed after the task has run */
static	__IO uint32_t tickprev = 0;


//Application time node
static timeRe_t new_timerSL(void);

static timeRe_t find_Timer( uint8_t task_id, uint16_t event_flag );
static timeRe_t find_TiemrPrev( uint8_t task_id, uint16_t event_flag );
static timestatus_t add_Timer( taskAble task_Able, uint16_t event_flag, uint16_t timeout );
static timestatus_t delect_Timer( uint8_t task_id, uint16_t event_flag);



/* message */
static taskMessFlag init_taskMessage(void);

 /**
 * Application time node
 * @param void
 * @return timeRe_t
 */
static timeRe_t new_timerSL(void)
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
void init_TimeRec(void)
{
	for(int i = 1; i < TIMESIZEO-1;i++)
	{
		timerman[i].cur = i+1;
	}
	timerman[0].cur = -1;
	timerman[TIMESIZEO-1].cur = 0;
}




/**
 * Initialize the message queue
 * @param void
 * @return  none
 */
taskMessFlag init_taskMessage(void)
{
	taskMesTail = taskMesfront = (taskMessage_t*)task_new_m(&tkhpHandler,sizeof(taskMessage_t));
	if (taskMesTail != NULL)
	{
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
 * @return  taskAble - Returns the address of the task handle,
 *			this parameter is used to facilitate management
 */
taskAble task_RegisterTaskApp(Task_Function RegTask, Task_Id taskid)
{
	/* For searching */
	taskLink_t* pTaskHight = &taskHight;
	
	/* Determine whether to initialize the registry */
	if (RegTask != NULL)
	{	
		/* Application node is used to save liao */
		taskLink_t* newtask = (taskLink_t*)task_new_m(&tkhpHandler,sizeof(taskLink_t));
		if (newtask != NULL)
		{
			taskLink_t* taskIterat;
			taskLink_t* taskIteratNext = NULL;
			newtask->task_id = taskid;	// taskid;
			newtask->task_event = 0;
			newtask->task_sysevent = 0;
			newtask->taskProcess = RegTask;
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
			return (taskAble)newtask;
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
Task_Id get_TaskLowId(void)
{
	return taskLowId;
}




/**
 * Delete a registered task , Usually refers to the layer
 * @param RegTask - The address of the incoming task handle
 * @return TASK_ErrorStatus - Can it be deleted?
 */
TASK_ErrorStatus delect_ResTaskApp(taskAble* RegTask )
{
	if (RegTask != NULL)
	{
		/* Used to traverse the entire task system. */
		taskLink_t* pxPrevDelect = &taskHight;
		
		/* Iteratively find the task to be deleted. */
		for (; pxPrevDelect != NULL && pxPrevDelect->taskNext->taskProcess != (*RegTask)->taskProcess; pxPrevDelect = pxPrevDelect->taskNext)
		{
		}
		/* Whether to find the task. */
		if (pxPrevDelect == NULL)
		{
			/* No task to delete was found. */
			task_TEST();
		}
		else
		{
			/* Start deleting task. */
			taskLink_t* pxdelect = pxPrevDelect->taskNext;
			if (pxdelect->taskNext != NULL)
			{
				pxPrevDelect->taskNext = pxdelect->taskNext;
				pxdelect->taskNext->taskPrev = pxPrevDelect;
			}
			else
			{
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
	else
	{
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
taskMessFlag send_TaskMessage(taskAble task, void* data)
{
	if (task != NULL)
	{
		if (taskMesTail == NULL && taskMesfront == NULL)
		{
			init_taskMessage();
		}
		
		if (taskMesTail != NULL && taskMesfront != NULL)
		{
			taskMessage_t* newMess = (taskMessage_t*)task_new_m( &tkhpHandler ,sizeof(taskMessage_t));
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
taskMessFlag get_TaskMessage(taskAble task, void** res)
{
	(*res) = NULL;

	if (task != NULL)
	{
		if (taskMesTail  == NULL && taskMesfront == NULL)
			return mes_resError;
		else
		{
			if (taskMesTail != taskMesfront)
			{
				taskMessage_t* pxMess = taskMesfront->nextMess;
				if (pxMess->task == task)
				{
					(*res) = pxMess->data;
					taskMesfront->nextMess = pxMess->nextMess;
					if (pxMess == taskMesTail)
					{
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








/**
 * Find the previous node of a timed task
 * @param task_id
 * @param event_flag- find task event
 * @return timeRe_t
 */
timeRe_t find_TiemrPrev( uint8_t task_id, uint16_t event_flag )
{
	timeRe_t searchTime = timerman[0].cur;

	if (timerman[searchTime].task_id == task_id && timerman[searchTime].task_event == event_flag)
	{
		searchTime = 0;
		return searchTime;
	}

	timeRe_t searchPrevtime = 0;
	while (searchTime != -1 )
	{
		if (timerman[searchTime].task_id == task_id && timerman[searchTime].task_event == event_flag)
		{
			break;
		}
		searchPrevtime = searchTime;
		searchTime = timerman[searchTime].cur;

	}
	if (searchTime == -1)
	{
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
timeRe_t find_Timer( uint8_t task_id, uint16_t event_flag )
{
	
	timeRe_t srchTimer = timerman[0].cur;
	
	if(timerman[1].cur == 0)
	{
		return 0;
	}
	
	while(srchTimer != -1)
	{
		if(task_id == timerman[srchTimer].task_id &&
			event_flag == timerman[srchTimer].task_event )
		{
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
timestatus_t add_Timer( taskAble task_Able, uint16_t event_flag, uint16_t timeout )
{
	/*Search whether there are exactly the same nodes.*/
	timeRe_t srchTimer = find_Timer( task_Able->task_id,event_flag );
	
	if(srchTimer == 0)
	{
		//mem full
		return timerror;
	}
	
	/* If there is no exactly the same node, then create a new one. */
	if(srchTimer == -1)
	{
			/* Apply for a node. If it is zero, return an error code. */
			timeRe_t newtime = new_timerSL();
			if (newtime == 0)
			{
				/* Oh, something went wrong! */
				return timerror;
			}
			timerman[newtime].task_event = event_flag;
			timerman[newtime].task_id = task_Able->task_id;
			timerman[newtime].timeout = timeout;
			timerman[newtime].task_able = task_Able;
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
	else
	{	
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
timestatus_t delect_Timer( uint8_t task_id, uint16_t event_flag)
{
	timeRe_t prevtime = find_TiemrPrev(task_id,event_flag);
	if (prevtime == -1)
	{
		return timerror;
	}
	timeRe_t delect = timerman[prevtime].cur;//delect

	if (timerman[delect].cur == -1)
	{
		//if head
		timerman[prevtime].cur = -1;
	}
	else
	{
		timerman[prevtime].cur = timerman[delect].cur;
	}
	
	timerman[delect].cur = timerman[1].cur;
	timerman[1].cur = delect;
	timerCount--;
	
	return timerDelect;
}






/**
 * get_Timersize
 * @param void
 * @return size
 */
uint16_t get_Timersize(void)
{
	return timerCount;
}




/**
 * Initialize the components of the task
 * @param void
 * @return none
 */
void init_Tasks( void )
{
	
	/* Initialization time memory management table */
	init_TimeRec();
	
	/* Initialize the message queue */
	//init_taskMessage( );
	
	/* Initialize heap space for task management. */
	initHp(&tkhpHandler, taskRegheap,TASKREGHEAPSIZE);
	
	
	USER_TASKINIT();
	/* Start the heartbeat of task management! */
	HAL_TIM_Base_Start_IT(&htim6);//offer task time kernel
	
	
}




/**
 * Set an event
 * @param taskId- Task to be set
 * @param taskEvent-Event to be set
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_SetEvent(taskAble task_able , Task_Event taskEvent)
{
	if (task_able != NULL && taskEvent & 0xffffU)
	{
		task_able->task_event |= taskEvent;
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
TASK_ErrorStatus task_ClearEvent(taskAble task_able, Task_Event event_flag )
{
  if ( task_able !=  NULL && task_able->task_event >= event_flag)
  {
	
    task_able->task_event &= (~event_flag);  // Stuff the event bit(s)

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
TASK_ErrorStatus task_SysSetEvent(taskAble task_able , Task_SysEvent task_sysevent)
{
	if (task_able != NULL && task_sysevent & TASK_SYSEVENTMAX)
	{
		task_able->task_sysevent |= task_sysevent;
		task_SetEvent(task_able,TASK_SYSTEMEVENT);
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
TASK_ErrorStatus task_SysClearEvent(taskAble task_able , Task_SysEvent task_sysevent)
{
	if (task_able != NULL && task_sysevent & TASK_SYSEVENTMAX)
	{
		task_able->task_sysevent &= (~task_sysevent);
		return ( TASK_SUCCESS );
	}
	return (TASK_ERROR);
}



/**
 * get system events
 * @param task_Able
 * @return TASK_ErrorStatus
 */
Task_SysEvent task_GetSysEvent(taskAble task_able)
{
	if (task_able != NULL)
	{
		return task_able->task_sysevent;
	}	
	return 0;
}



/**
 * Update system task time
 * @param updateTime- Number of updates
 * @return void
 */
void timer_Update( uint32_t updateTime )
{
  timeRe_t srchTimer;
	
  // Look for open timer slot
  if ( timerman[0].cur != -1 )
  {
    // Add it to the end of the timer list
    srchTimer = timerman[0].cur;

    // Look for open timer slot
    while ( srchTimer != -1 )
    {
      timeRe_t freeTimer = 0;
       
      if (timerman[srchTimer].timeout <= updateTime)
      {
        timerman[srchTimer].timeout = 0;
      }
      else
      {
        timerman[srchTimer].timeout -=  updateTime;
      }
      
//      // Check for reloading
//      if ( (timerman[srchTimer].timeout == 0) && (timerman[srchTimer].timeout) && (timerman[srchTimer].timeout) )
//      {
//        // Notify the task of a timeout
//        task_SetEvent(timerman[srchTimer].task_id,timerman[srchTimer].event_flag );
//        
//      }
      
      // When timeout or delete (event_flag == 0)
      if ( timerman[srchTimer].timeout == 0 || timerman[srchTimer].task_event == 0 )
      {
        freeTimer = srchTimer;
      }
			
	  srchTimer = timerman[srchTimer].cur;
			
      if ( freeTimer )
      {
        if ( timerman[freeTimer].timeout == 0 )
        {
          task_SetEvent( timerman[freeTimer].task_able, timerman[freeTimer].task_event );
        }
        delect_Timer( timerman[freeTimer].task_id, timerman[freeTimer].task_event  );
      }
    }
  }
}





/**
 * Get count value
 * @param task_Able
 * @param void
 * @return task_count
 */
uint32_t get_KernalCount()
{
	return task_count;
}




/**
 * Clear count value
 * @param void
 * @return none
 */
void set_KernalCountClear()
{
		task_count = 0;
}






/**
 * System operation function!
 * @param void
 * @return none
 */
void task_RunSystem(void)
{
	__IO uint8_t idx = 0;
	
	tickprev = get_KernalCount();

	timer_Update(ticknow);
	
	taskLink_t* searActive = taskHight.taskNext;

	while (searActive != NULL && searActive->task_event == 0)
	{
		searActive = searActive->taskNext;
	}
	
	if (searActive != NULL)
	{
		taskAble active = (taskAble)searActive;
		searActive->taskProcess(active);
	}
	
	ticknow = get_KernalCount() - tickprev;
	
	while(ticknow == 0)
	{
		//compensate time 
		ticknow = get_KernalCount() - tickprev;
	}
	set_KernalCountClear();
}



/**
 * Endless loop ,Provided to external calls!
 * @param void
 * @return none
 */
void run_Task(void)
{
	for(;;)
	{
		task_RunSystem();
	}
}






/**
 * Start a timer for system events
 * @param task_Able
 * @param event_flag:
 * @param timeout_value: The time the task runs (ms)
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_StartTimerEx(taskAble task_Able, Task_Event event_flag, uint16_t timeout_value )
{
	
	if (task_Able != NULL)
	{
		timestatus_t  newTimer;
		// Add timer
		newTimer = add_Timer( task_Able, event_flag, timeout_value );
		//Return status
		return ( (newTimer != NULL) ? TASK_SUCCESS : TASK_ERROR );
		
	}
	/* Error flag is returned. */
	return TASK_ERROR;
}




/**
 * This is called by task_StopTimerGuara! 
 * @param taskID
 * @param event_flag
 * @return TASK_ErrorStatus
 */
TASK_ErrorStatus task_StopTimerEx( Task_Id taskID, Task_Event event_flag)
{

	timestatus_t  newTimer;
	// Add timer
	newTimer = delect_Timer( taskID, event_flag);

  return ( (newTimer != NULL) ? TASK_SUCCESS : TASK_ERROR );
}



/**
 * Guarantee will be deleted , Please use it in the system message! 
 * @param taskID
 * @param delect_event_id:
 * @param event_id: Current event
 * @return none
 */
void task_StopTimerGuara( Task_Id taskID,Task_Event delect_event_id, Task_Event* event_flag)
{
	if( IS_TASK_EVENT(*event_flag , delect_event_id) )
	{
		CLEAR_TASK_EVENT(*event_flag,delect_event_id);
	}
	else
	{
		task_StopTimerEx(taskID,delect_event_id);
	}
}



/**
 * Provide the heart of the system 
 * @param void
 * @return none
 */
TASK_CORE_TICK()
{
	if(htim == &htim6)
	{
		task_count++;
	}
}



