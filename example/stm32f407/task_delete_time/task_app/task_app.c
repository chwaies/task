

#include "task.h"
#include "stdio.h"
#include "key.h"

#define TEST_PRI   1

//?????????????

//????
#define TEST_GEN_EVENT 	0x0001

#define TEST_KEY_EVENT  0x0002

//????
#define TEST_SYS_EVENT  0x0001


//??????,????????????
taskType task1Handler;


void uart1_start_printf(task_u* arg)
{
	printf("this is a test\n");
    task_start_timer(task1Handler,TEST_GEN_EVENT,1000);
}

void uart1_stop_printf(task_u* arg)
{
	printf("stop task1Handler TEST_GEN_EVENT \n");
    task_stop_timer(task1Handler,TEST_GEN_EVENT);
}

void key_process(task_u* arg)
{
	KeyDriver();
	task_start_timer(task1Handler,TEST_KEY_EVENT,30);
}

void user_TaskInit(void)
{

	task1Handler = task_reg_app(TEST_PRI);

    task_new_genEx(task1Handler ,uart1_start_printf ,TEST_GEN_EVENT );
    
	task_new_genEx(task1Handler,key_process,TEST_KEY_EVENT);
	
    // ????????,??????????
	task_new_sysEx(task1Handler ,uart1_stop_printf ,TEST_SYS_EVENT );
	
	
    
    //????,????????
	task_set_event(task1Handler , TEST_GEN_EVENT);
	task_set_event(task1Handler, TEST_KEY_EVENT);
    
}

//????
//??????
void fun(void)
{
    //????printf????
    task_set_sysex(task1Handler , TEST_SYS_EVENT);
}






void task_hardware_init(void)
{
	
}


