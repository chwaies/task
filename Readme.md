# 介绍

**这是一个事件管理软件**

**特性:**

* *任务优先级可分*
* *事件优先级可分*
* *实现任务管理*
* *内存管理*
* *较低复杂度*
* 非抢占式调度





#  使用task管理，开发单片机应用程序

----

### 快速构建:

- #### 提供1ms定时器

- #### 初始化task

- #### 注册任务&&生成事件&&设置事件

----

### 提供1ms心脏节拍:

在中断函数

~~~c
//1ms中断调用
void task_info(void);

如:
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */
  task_info();
  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}
~~~

### 初始化任务管理器:

在主函数

~~~c
int main()
{
    ....
    task_init();//初始化task
    
    //当初始化完成之后
	//用户事件，就可以注册事件了
    run_Task();    
}
~~~



### 注册任务&&生成事件&&设置事件:

注册一类任务，该类是指优先级

***taskType task_reg_app(Task_Id taskid)***

* 参数一 任务优先级，数值越大，在task中优先级更高

返回一个任务类型，用于描述任务



**向一类任务，生成一个普通事件**

***void task_new_genEx(taskType task_type , Task_Event_cb tk_pro_cb ,Task_Event clr_event)***

* task_type -  任务类型
* tk_pro_cb  - 事件回调函数， void fun (task_u* arg),该函数类型的返回值是void ，参数是task_u* 
* clr_event-  注册的事件

clr_event -  热键码越小对应的事件在task_type 所对应的优先级就越高



**向一类任务，设置事件**

***TASK_ErrorStatus task_set_event(taskType task_type , Task_Event taskEvent);***



~~~c
#include "task.h"

#define TEST_PRI   1

//设置事件值是热键码，必须是热键码
#define TEST_EVENT1 0x0001

//定义全局变量,该变量作用域不能是局部的
taskType task1Handler;

//提供事件回调函数
void task1_event1_process(task_u* arg)
{
	printf("this is a test\n");
}


//用户函数在task_init后面
//user_TaskInit 任务初始化
void user_TaskInit(void)
{
    // 1 是任务优先级,数值越大优先级越高
    // 2 该函数返回一个任务的句柄
	task1Handler = task_reg_app(TEST_PRI);
    
    
    // 1 函数作用是生成一个 TEST_EVENT1 事件
    //参数1 表示 TEST_EVENT1 事件属于哪个优先级
    //参数2 是TEST_EVENT1 事件的回调函数
    //参数3 是事件的热键码
    task_new_genEx(task1Handler ,task1_event1_process ,TEST_EVENT1 );
    
    //生成事件不会立即运行，应需要调用设置事件函数
    //设置 task1Handler 的 TEST_EVENT 事件，
    // TEST_EVENT 注意的是，该值是一个热键码,而且是由task_new_genEx 注册过的事件
	task_set_event(task1Handler , TEST_EVENT1);
    
}

~~~

一个由任务管理的事件就配置成功了。

运行打印：this is a test





# 常用功能:

*task任务管理*

完成上面的之后你会发现，task1 的TEST_EVENT1 事件运行完一次之后就不运行了

当我们想运行一个定时的事件时可以用

**向一类任务，开始一个计时器**

 ***task_start_timer(taskType task_Able, Task_Event event_flag, TIME_SAVE_TYPE timeout_value )*** 

* 参数一 任务的句柄，该参数必须是任务注册产生的

* 参数二 任务要运行的事件，热键码

* 参数三 任务运行的时间(ms)

请接着上面的配置，进行修改

```c
void task1_event1_process(task_u* arg)
{
	printf("this is a test\n");
    //启动定时事件
    task_start_timer(task1Handler,TEST_EVENT1,1000);
}
```

固件下载后你会发现printf,会隔1s 打印一次







#### 停止定时事件

**向一类任务，停止一个定时器**

***void task_stop_timer(taskType task_type,Task_Event des_event);***

- 参数一 任务的句柄，该参数必须是任务注册产生的
- 参数二 任务要停止的事件，热键码

注意事项：

* **在本事件中同一类任务中运行 task_stop_timer，才能保证事件能顺利删除**

如:

```c
#define TEST_PRI   1

//设置事件值是热键码，必须是热键码
#define TEST_EVENT1 0x0001
#define TEST_EVENT2 0x0002


//定义全局变量,该变量作用域不能是局部的
taskType task1Handler;


void uart1_start_printf(task_u* arg)
{
	printf("this is a test\n");
    task_start_timer(task1Handler,TEST_EVENT1,1000);
    
    task_set_event(task1Handler,TEST_EVENT2);
}
void uart1_stop_printf(task_u* arg)
{
	printf("stop task1Handler TEST_EVENT1 \n");
    task_stop_timer(task1Handler,TEST_EVENT1);
}

void user_TaskInit(void)
{

	task1Handler = task_reg_app(TEST_PRI);

    task_new_genEx(task1Handler ,uart1_start_printf ,TEST_EVENT1 );
    // 注册一个新事件，用于删除打印事件
	task_new_genEx(task1Handler ,uart1_stop_printf ,TEST_EVENT2 );
	task_set_event(task1Handler , TEST_EVENT1);
    
}

```

上述的方式固然可以删除本优先级的任务对应的事件，但是为了规范管理我们也可以设置一个系统事件

与之相比普通事件，系统事件可以有比普通事件更加优先处理的能力



对应的API如下：

**向一个类优先级任务，生成系统事件**

***void task_new_sysEx(taskType task_type , Task_Event_cb tk_pro_cb ,Task_Event clr_event)***

- task_type -  任务类型
- tk_pro_cb  - 事件回调函数， void fun (task_u* arg),该函数类型的返回值是void ，参数是task_u* 
- clr_event -  注册的事件

clr_event 热键码越小对应的事件在task_type 所对应的优先级就越高

```c
#define TEST_PRI   1

//注意区分系统事件和普通事件

//普通事件
#define TEST_GEN_EVENT 	0x0001

//系统事件
#define TEST_SYS_EVENT  0x0001


//定义全局变量,该变量作用域不能是局部的
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

void user_TaskInit(void)
{

	task1Handler = task_reg_app(TEST_PRI);

    task_new_genEx(task1Handler ,uart1_start_printf ,TEST_GEN_EVENT );
    
    // 注册一个系统事件，用于管理删除打印事件
	task_new_sysEx(task1Handler ,uart1_stop_printf ,TEST_SYS_EVENT );
    
    //我们让它，默认运行打印事件
	task_set_event(task1Handler , TEST_GEN_EVENT);
    
}

//信息传递
//通常是外部的
void fun(void)
{
    //执行关闭printf打印事件,注意这个api是 task_set_sysex 设置系统事件
    task_cls_sysex(task1Handler , TEST_SYS_EVENT);
}


```





#### 消息队列的使用

在 task 的管理中，实现有任务消息的处理



**向一类任务，发送一个信息**

***taskMessFlag task_send_msg(taskType task,Task_Event set_event,void\* data)***

* 参数1 ：信息发送给那一个类型的任务

* 参数2：发送给哪一个事件
* 参数3：携带的数据，该数组作用域必须是全局的

返回一个状态值，见task.h



**向一类任务，获取一个信息**

***taskMessFlag task_get_msg(taskType task, void\*\* res);***

* 参数1：在哪一个优先级的任务中获取
* 参数2：接受数据的指针

返回一个状态值，用于判断是否获取成功 见task.h



例子:

如：在按键处理中申请一个消息，发送到 task1_event3_process 对应的

```c

#define TASK1_DIS_PRINTF 0x000004
void KeyAction(uint8_t key)
{
	switch(key)
	{
		case KEY1_PRES:
		{
			char* str = (char*)task_new_m(&tkhpHandler,10);
			char* srcstr = "hello";
			memset(str,0,10);
			memcpy(str,srcstr,strlen(srcstr));
            
			task_send_msg(task1, TASK1_DIS_PRINTF ,str);
		}
}
    
    
void task1_event3_process(task_u* arg )
{
	//定义一个指针指向数据
	char* str = NULL;
	if( mes_getSucc == task_get_msg(task1,(void*)&str) )
	{
		printf("%s\n",str);
        
        //释放内存
		task_del_m(&tkhpHandler,str);
	}
}

   
```



更多功能教程更新中...





#### **配置文件解释:**taskConfig.h

```c
#define TASK_TIME_SIZEO    20+2
```

TASK_TIME_SIZEO 的宏定义为时间节点的多少，这个并没有用到内存管理，原因是怕多任务时可能内存申请的比较大，所以为了避免task能够规避风险，不进行采用

内存管理，我们可以调用 *task_get_time_size* 函数获取时间节点的多少来进行调试，修改成为一个合适的数值



```c
#define TASK_REG_HEAP_SIZE  0x400
```

TASKREGHEAPSIZE - 该宏定义是指内存管理的大小

内存管理借鉴的是freertos 的heap_4 ，在此基础上进行了修改，可以管理多个内存。



```
#define TIME_SAVE_TYPE   		  uint16_t
```

该宏定义是时间存储的数据类型，如果65536ms，如果未能满足你的工程需求，可以进行修改







**总而言之，task 现在还很年轻，更多功能还有待完善！**



































