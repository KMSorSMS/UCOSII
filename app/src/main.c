#include "main.h"
int a = 11;

int main()
{
#if OS_CRITICAL_METHOD == 3u /* Allocate storage for CPU status register */
    OS_CPU_SR cpu_sr = 0u;
#endif
    OSCtxSw();
    return 0;
}

void my_task_0_t_(void)
{
    while (1)
    {
        a = 10;
    }
}