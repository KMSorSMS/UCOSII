#include "usart.h"



int _write(int fd,char *ptr,int len){
    while ((USART_UX->SR & 0X40) == 0);     /* 等待上一个字符发送完成 */
    for(int i=0;i<len;i++){
        USART_UX->DR = (uint8_t)ptr[i];             /* 将要发送的字符 ch 写入到DR寄存器 */
        while ((USART_UX->SR & 0X40) == 0);     /* 等待上一个字符发送完成 */
    }
    return len;
}

//实现一个float to string的函数，将float转换成字符串
char * _float_to_char(float x, char *p, int char_buff_size) {
    char *s = p + char_buff_size; // go to end of buffer
    uint16_t decimals;  // variable to store the decimals
    int units;  // variable to store the units (part to left of decimal place)
    if (x < 0) { // take care of negative numbers
        decimals = (int)(x * -100) % 100; // make 1000 for 3 decimals etc.
        units = (int)(-1 * x);
    } else { // positive numbers
        decimals = (int)(x * 100) % 100;
        units = (int)x;
    }

    *--s = (decimals % 10) + '0';
    decimals /= 10; // repeat for as many decimal places as you need
    *--s = (decimals % 10) + '0';
    *--s = '.';

    while (units > 0) {
        *--s = (units % 10) + '0';
        units /= 10;
    }
    if (x < 0) *--s = '-'; // unary minus sign for negative numbers
    return s;
}

UART_HandleTypeDef g_uart_handle;                          //定义串口1句柄
uint8_t g_rx_buffer[RXBUFFERSIZE];                         //串口1接收缓冲区
uint8_t g_usart_rx_buf[USART_REC_LEN];                      //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
uint16_t g_usart_rx_sta = 0;

void USART1_Init(unsigned int bound) {
    g_uart_handle.Instance = USART_UX;                       //USART1作为实例
    g_uart_handle.Init.BaudRate = bound;                   //波特率
    g_uart_handle.Init.WordLength = UART_WORDLENGTH_8B;    //字长,8位,代表串口每次发送8位数据
    g_uart_handle.Init.StopBits = UART_STOPBITS_1;         //停止位,1位,代表停止位为1位
    g_uart_handle.Init.Parity = UART_PARITY_NONE;          //校验位,无校验位，如果有校验位会占用WORDLENGTH的位数
    g_uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;    //硬件流控制,无硬件流控制
    g_uart_handle.Init.Mode = UART_MODE_TX_RX;             //发送接收模式
    g_uart_handle.Init.OverSampling = UART_OVERSAMPLING_16; //采样率,16倍过采样,也就是和stm32F103的串口一样
    while(HAL_UART_Init(&g_uart_handle) != HAL_OK);
    //开启串口的异步接收中断
    HAL_UART_Receive_IT(&g_uart_handle, (uint8_t*)g_rx_buffer, RXBUFFERSIZE);
}
//定义HAL_UART_MspInit函数,初始化串口1
void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef gpio_init_struct = {0};
    if (huart->Instance == USART_UX) {                          //判断是否是串口1,如果是串口1,则初始化串口1
        __HAL_RCC_USART1_CLK_ENABLE();                      //使能USART1时钟
		__HAL_RCC_GPIOA_CLK_ENABLE();                       //使能GPIOA时钟
        gpio_init_struct.Pin = GPIO_PIN_9;                  //PA9
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;            //复用开漏输出,因为F401RE有外部上拉电阻,所以可以使用开漏输出
        gpio_init_struct.Pull = GPIO_NOPULL;                //无上下拉
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      //高速
        gpio_init_struct.Alternate = GPIO_AF7_USART1;       //复用为USART1
        HAL_GPIO_Init(GPIOA, &gpio_init_struct);            //初始化PA9
		gpio_init_struct.Pin = GPIO_PIN_10;
		gpio_init_struct.Mode = GPIO_MODE_AF_PP;
		HAL_GPIO_Init(GPIOA, &gpio_init_struct);
		
        HAL_NVIC_SetPriority(USART_UX_IRQn, 0, 0);            //设置中断优先级,第一个参数是中断号,第二个参数是抢占优先级,第三个参数是响应优先级
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);                    //使能中断

    }
}

//串口1中断服务函数
void USART_UX_IRQHandler(void) {
	  HAL_UART_IRQHandler(&g_uart_handle);   /* 调用HAL库中断处理公用函数 */
}
//接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart->Instance == USART_UX) {                          //判断是否是串口1
            if ((g_usart_rx_sta & 0x8000) == 0)             /* 接收未完成 */
            {
                if (g_usart_rx_sta & 0x4000)                /* 接收到了0x0d（即回车键） */
                {
                    if (g_rx_buffer[0] != 0x0a)             /* 接收到的不是0x0a（即不是换行键） */
                    {
                        g_usart_rx_sta = 0;                 /* 接收错误,重新开始 */
                    }
                    else                                    /* 接收到的是0x0a（即换行键） */
                    {
                        g_usart_rx_sta |= 0x8000;           /* 接收完成了 */
                    }
                }
                else                                        /* 还没收到0X0d（即回车键） */
                {
                    if (g_rx_buffer[0] == 0x0d)
                        g_usart_rx_sta |= 0x4000;
                    else
                    {
                        g_usart_rx_buf[g_usart_rx_sta & 0X3FFF] = g_rx_buffer[0];
                        g_usart_rx_sta++;

                        if (g_usart_rx_sta > (USART_REC_LEN - 1))
                        {
                            g_usart_rx_sta = 0;             /* 接收数据错误,重新开始接收 */
                        }
                    }
                }
            }

            HAL_UART_Receive_IT(&g_uart_handle, (uint8_t*)g_rx_buffer, RXBUFFERSIZE);
    }
}
