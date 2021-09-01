# 使用任务管理，开发单片机应用程序

----

### 快速构建:

- #### 初始化task

- #### 提供1ms定时器

- #### 注册任务&&设置事件&&清除事件

----

### 初始化:

在主函数里面调用

~~~c
int main()
{
    ....
    init_Tasks();//初始化task
    
    //当初始化完成之后
    //执行
    run_Task();    
}
~~~

### 提供1ms心脏节拍:

~~~c
//使用宏定义在右侧添加1ms中断，该宏文件在taskConfig.h
#define TASK_CORE_TICK()    
~~~



### 注册任务&&设置事件&&清除事件:

~~~c
//设置事件值是热键码
#define TEST_EVENT 0x0001

//定义全局变量,该变量作用域不能是局部的
taskAble task1Handler;

//注册任务就必须提供回调函数,回调函数类型为 void fun(void* arg)
static void test1_Process(void * arg);

//user_TaskInit 函数进行初始化
//在taskConfig.h 的 USER_TASKINIT() 宏定义中可以修改为别的函数
void user_TaskInit(void)
{
    // test1_Process 为回调函数
    // 1 是任务优先级,数值越大优先级越高
	task1Handler = task_RegisterTaskApp(test1_Process,1);
    
    //设置 task1 的 TEST_EVENT 事件，注意的是，该值是一个热键码
	task_SetEvent(task1Handler , TEST_EVENT);
}

static void test1_Process(void * arg)
{
	taskAble taskable = (taskAble)arg;
	Task_Event taskEvent = taskable->task_event;
    
    // taskEvent & TEST_EVENT,对应着设置的事件
    if(taskEvent & TEST_EVENT)
    {
        pritnf("this is test task!\n");
        .....
        //当 TEST_EVENT 事件处理完成之后，要调用 task_ClearEvent 函数把该事件清除
        //否则任务管理会出现一直运行该事件
        task_ClearEvent(taskable,TEST_EVENT);
    }
}
~~~

**按照上面所描述的配置，一个简单的任务管理就完成了。**





# 常用功能:

*task任务管理*

