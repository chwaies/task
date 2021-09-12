#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

/* How many task time management nodes */
#define TASK_TIME_SIZEO    20+2
/* 0x400 is 1K */
#define TASK_REG_HEAP_SIZE  0x400


/*  task heap manage. */


//#define task_new_m(hpBase,xSize)  task_new_m(hpBase,xSize)
//#define task_del_m(hpBase,xSize)  task_new_m(hpBase,xSize)

#define TIME_SAVE_TYPE   		  uint16_t
#define TASK_SYSEVENTMAX          (0XFFFFU)


#define USER_TASKINIT()








#endif
