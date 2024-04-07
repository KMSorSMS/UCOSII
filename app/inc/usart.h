#ifndef USART_H
#define USART_H

    #include "main.h"
    #include "stdio.h"
    
    // #include "stm32f4xx_hal_uart.h"
    #define USART_REC_LEN 200
    #define RXBUFFERSIZE 1
    #define USART_UX_IRQHandler USART1_IRQHandler                      //串口1中断服务函数
    #define USART_UX_IRQn USART1_IRQn                                  //串口1中断通道
    #define USART_UX USART1
    // extern UART_HandleTypeDef g_uart1_handle;                          //定义串口1句柄
    extern INT16U g_usart_rx_sta;                                    //接收状态标记
    extern INT8U g_rx_buffer[RXBUFFERSIZE];                          //HAL库USART接收Buffer
    extern INT8U g_usart_rx_buf[USART_REC_LEN];                      //串口1接收缓冲区
    extern void USART1_Init(unsigned int bound);

#endif