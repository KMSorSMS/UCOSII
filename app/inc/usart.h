#pragma once
#ifndef USART_H
#define USART_H
#include "ucos_ii.h"
#include "stm32f401xe.h"
#include <stdarg.h>                         // 可变参数头文件
#include <stdio.h>                           // 使用vprintf函数需要包含的头文件
// 在此处将串口2的信息修改为串口1的信息即可实现串口1的初始化
#define USART_REC_LEN 200
#define RXBUFFERSIZE 1

// ************************************************************************
#define USART_UX_IRQHandler USART1_IRQHandler // 串口2中断服务函数
#define USART_UX_IRQn USART1_IRQn             // 串口2中断通道,相当于中断号
#define USART_UX USART1                       //需要初始化的是串口2
#define USART_TX_PIN 9                         // 串口2：PA2  串口1：PC9 
#define USART_RX_PIN 10                          // 串口2：PA3  串口1：PC10
#define APB1FCK 84000000                       // 串口2：42000000 串口1：84000000

// #define USART_UX_IRQHandler USART2_IRQHandler // 串口2中断服务函数
// #define USART_UX_IRQn USART2_IRQn             // 串口2中断通道,相当于中断号
// #define USART_UX USART2                       //需要初始化的是串口2
// #define USART_TX_PIN 2                         // 串口2：PA2  串口1：PC9 
// #define USART_RX_PIN 3                          // 串口2：PA3  串口1：PC10
// #define APB1FCK 42000000                       // 串口2：42000000 串口1：84000000

// ************************************************************************
#define USART_SHARE_BUF_SIZE 256               // 串口共享缓冲区大小
extern uint16_t g_usart_rx_sta;               // 接收状态标记
extern uint8_t g_rx_buffer[RXBUFFERSIZE];     // HAL库USART接收Buffer
extern uint8_t g_usart_rx_buf[USART_REC_LEN]; // 串口1接收缓冲区，也就是共享内存
extern void USART_Init(unsigned int bound);
extern char * _float_to_char(float x, char *p, int char_buff_size);
extern int usart_send(const char *format, ...);   // 串口发送数据函数
extern void usart_receive(void* args);                  // 串口接收数据函数
extern OS_EVENT *uasrt_tx_sem;                      //串口发送互斥信号量，在OS_stk.h中声明，在主函数中初始化
extern OS_EVENT *uasrt_rx_sem;
#endif
