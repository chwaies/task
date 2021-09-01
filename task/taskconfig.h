#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

#include "sysapp.h"
#include "task.h"
#include "stdio.h"
#include "heap_4.h"



/* How many task time management nodes */
#define TIMESIZEO    20+2 
/* 0x400 is 1K */
#define TASKREGHEAPSIZE  0x400





/* Provide system heart clock */
#define TASK_CORE_TICK()    void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)

extern hpSeBind tkhpHandler;
/*  task heap manage. */
#define task_new_m(hpBase,xSize)  hpNew(hpBase,xSize)
#define task_del_m(hpBase,xSize)  hpdel(hpBase,xSize)


extern void user_TaskInit(void);
#define USER_TASKINIT()          user_TaskInit()










#endif
