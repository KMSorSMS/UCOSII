#ifndef OS_STK_H
#define OS_STK_H
#include "ucos_ii.h"
#define MY_TASK_SIZE_0 256
#define MY_TASK_SIZE_1 256
#define MY_TASK_SIZE_2 1024
OS_STK my_task_0[MY_TASK_SIZE_0];
OS_STK my_task_1[MY_TASK_SIZE_1];
OS_STK my_task_2[MY_TASK_SIZE_2];

#endif
