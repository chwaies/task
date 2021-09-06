#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

#include "sysapp.h"
#include "task.h"
#include "stdio.h"
#include "task_mem.h"



/* How many task time management nodes */
#define TASK_TIME_SIZEO    20+2
/* 0x400 is 1K */
#define TASK_REG_HEAP_SIZE  0x400



extern hpSeBind tkhpHandler;
/*  task heap manage. */


//#define task_new_m(hpBase,xSize)  task_new_m(hpBase,xSize)
//#define task_del_m(hpBase,xSize)  task_new_m(hpBase,xSize)

#define TASK_TIMER_SAVE_TYPE   		  uint16_t
#define TASK_SYSEVENTMAX          (0XFFFFU)



#define USER_TASKINIT()






#endif
