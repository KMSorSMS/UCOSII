#ifndef OS_STK_H
#define OS_STK_H
#include "os_cpu.h"
#include "ucos_ii.h"
#define MY_TASK_SIZE_0 256
#define MY_TASK_SIZE_1 256

#define MY_TASK_SIZE_2 1024
extern OS_STK my_task_0[MY_TASK_SIZE_0];
extern OS_STK my_task_1[MY_TASK_SIZE_1];
extern OS_STK my_task_2[MY_TASK_SIZE_2];
#define MADGWICK_TASK_SIZE 256
extern OS_STK madgwick_stk[MADGWICK_TASK_SIZE];

#define USART_RX_TASK_SIZE 256
extern OS_STK uasrt_rx_task[USART_RX_TASK_SIZE];

extern OS_EVENT *uasrt_tx_sem;                //声明串口发送数据的互斥信号量（信号量本质上是一个事件）
extern OS_EVENT *uasrt_rx_sem;                //声明串口接收数据的资源信号量（这个资源最多只有一条所以可以直接当互斥信号量用）
extern OS_EVENT *madgwick_sem;                //姿态解算的信号量
// OS_EVENT *uasrt_rx_sem;
#define MOTOR_TASK_STK_SIZE 1024
extern OS_STK motor_change_task_stk[MOTOR_TASK_STK_SIZE];
#endif
