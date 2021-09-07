

#include "task.h"
#include "stdio.h"
#include "key.h"
#include <string.h>


#define TEST_PRI   1

//?????????????

//????
#define TEST_GEN_EVENT 	0x0001

#define KEY_SCNAF_ACTIVE 0x0002

//??????,????????????
taskType task1Handler;


void test_msg(task_u* arg)
{
	char* str = NULL;
	if( mes_getSucc == task_get_msg(task1Handler,(void*)&str) )
	{
		printf("%s\n",str);
        
		task_del_m(&tkhpHandler,str);
	}
}

void button_scanf(task_u *arg)
{
	KeyDriver();
	task_start_timer(task1Handler,KEY_SCNAF_ACTIVE , 30);
}


void user_TaskInit(void)
{

	task1Handler = task_reg_app(TEST_PRI);

    task_new_genEx(task1Handler ,test_msg ,TEST_GEN_EVENT );
    
	task_new_genEx(task1Handler, button_scanf , KEY_SCNAF_ACTIVE);
	
	task_set_event(task1Handler, KEY_SCNAF_ACTIVE);
	
	
}



void fun(void)
{
	char* str = (char*)task_new_m(&tkhpHandler,10);
	char* srcstr = "hello";
	memset(str,0,10);
	memcpy(str,srcstr,strlen(srcstr));
	task_send_msg(task1Handler, TEST_GEN_EVENT ,str);
}
