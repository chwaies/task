#include <stdio.h>
#include <string.h>
#include "sysapp.h"
#include "main.h"
#include "task.h"
#include "taskconfig.h"
#include "key.h"


taskType task1;
taskType task2;


void task1_event1_process(task_u* arg);
void task1_event2_process(task_u* arg);
void task1_event3_process(task_u* arg);
void task1_event4_process(task_u* arg);
void task1_event5_process(task_u* arg);
void task1_event6_process(task_u* arg);


void task2_event1_process(task_u* arg );
void task2_sysevent_process(task_u* arg);
// Set_the_scope_for_the_task
#define ______________________________________________



void user_TaskInit(void)
{

	______________________________________________
	{
		task1 = task_reg_app(1);
		task_new_genEx(task1 ,task1_event1_process ,0x01 );
		task_new_genEx(task1 ,task1_event2_process ,0x02 );
		task_new_genEx(task1, task1_event3_process,0x04);
		task_new_genEx(task1, task1_event4_process,0x08);
		task_new_genEx(task1, task1_event5_process,0x10);
		task_new_genEx(task1, task1_event6_process,0x20);
		
		
		task_set_event(task1 , 0x01);
		task_set_event(task1 , 0x02);
		task_set_event(task1 , 0x04);
		task_set_event(task1 , 0x08);
		task_set_event(task1 , 0x10);

	}
	______________________________________________
	
	
	______________________________________________
	{
		task2 = task_reg_app(2);	
		task_new_genEx(task2 ,task2_event1_process ,0x01 );

//		task_new_sysEx(task2,task2_sysevent_process,0x01);

		//task_set_event(task2,0x01);
	}
	______________________________________________

}

//0x01
void task1_event1_process(task_u* arg)
{
	printf("1-1\n");
	//task_set_sysex(task2,0x01);
	task_start_timer(task1,0x01,1000);
}

//0x02
void task1_event2_process(task_u* arg )
{
	printf("1-2\n");
	task_start_timer(task1,0x02,1000);
}


//0x04
void task1_event3_process(task_u* arg )
{

	char* str = NULL;
	if( mes_getSucc == task_get_msg(task1,(void*)&str) )
	{
		printf("%s\n",str);
		task_del_m(&tkhpHandler,str);
	}
}

//0x08
void task1_event4_process(task_u* arg )
{
	printf("1-4\n");
	task_start_timer(task1,0x08,1000);
}


//0x10
void task1_event5_process(task_u* arg )
{
	//printf("1-5\n");
	task_start_timer(task1,0x10,30);
	KeyDriver();
}



//0x20
void task1_event6_process(task_u*arg )
{
	printf("key send task1 event6\n");
	
	taskType tmpTask1 = ((taskType)(arg));
	
	task_stop_timer(tmpTask1,0x08);
}

//task2 0x01
void task2_event1_process(task_u* arg )
{
	printf("task2\n");
	task_start_timer(task2,0x01,500);
}



void task2_sysevent_process(task_u* arg)
{
	printf("task2_sys_process\n");
}

