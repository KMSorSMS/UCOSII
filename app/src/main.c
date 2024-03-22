#include "main.h"
#include "OS_stk.h"
int a = 11;


int main()
{
    OSInit();
    // 创建一个自己的任务
    (void)OSTaskCreate(my_task_0_t_, (void *)0, &my_task_0[MY_TASK_SIZE_0-1u], 20);
    // OS启动
    OSStart();
    return 0;
}

void my_task_0_t_(void* args)
{
    while (1)
    {
        a = 10;
    }
}