#include "task.h"
#include "printf.h"
#include "task.h"

#define TEST_PRI   1

//?????????,??????
#define TEST_EVENT1 0x0001

//??????,????????????
taskType task1Handler;

//????????
void task1_event1_process(task_u* arg)
{
	printf_u1("this is a test\n");
}


//?????task_init??
//user_TaskInit ?????
void user_task_init(void)
{
    // 1 ??????,?????????
    // 2 ????????????
	task1Handler = task_reg_app(TEST_PRI);
    
    
    // 1 ????????? TEST_EVENT1 ??
    //??1 ?? TEST_EVENT1 ?????????
    //??2 ?TEST_EVENT1 ???????
    //??3 ???????
    task_new_genEx(task1Handler ,task1_event1_process ,TEST_EVENT1 );
    
    //??????????,???????????
    //?? task1Handler ? TEST_EVENT ??,
    // TEST_EVENT ????,????????,????task_new_genEx ??????
	task_set_event(task1Handler , TEST_EVENT1);
    
}

