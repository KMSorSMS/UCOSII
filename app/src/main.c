#include "main.h"
int a = 11;


int main()
{
    OSInit();
    OSStart();
    return 0;
}

void my_task_0_t_(void)
{
    while (1)
    {
        a = 10;
    }
}