#include "usart.h"
#include "NVIC.h"
#include "PID.h"



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

uint8_t g_rx_buffer[RXBUFFERSIZE];                         //串口1接收缓冲区
uint8_t g_usart_rx_buf[USART_REC_LEN];                      //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
uint16_t g_usart_rx_sta = 0;

//定义HAL_UART_MspInit函数,初始化串口2
//PA2是TX，PA3是RX
void UART_MspInit() {
    //判断是否是串口1,如果是串口1,则初始化串口1
    if (USART_UX == USART1) {
        //使能USART1时钟。RCC_APB2ENR_USART1EN是USART1的时钟使能位（掩码）
        RCC->APB2ENR|=RCC_APB2ENR_USART1EN;
    }else if (USART_UX == USART2) {
        //使能USART2时钟。RCC_APB1ENR_USART2EN是USART2的时钟使能位（掩码）
        RCC->APB1ENR|=RCC_APB1ENR_USART2EN;
    }
    //使能GPIOA时钟。RCC_AHB1ENR_GPIOAEN是GPIOA的时钟使能位（掩码）
    RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;
    //初始化PA2(TX)
    GPIOA->MODER|=2<<(USART_TX_PIN<<1);             //PA2设置为复用模式
    GPIOA->OTYPER|=1<<USART_TX_PIN;                 //开漏。因为F401RE有外部上拉电阻,所以可以使用开漏输出
    GPIOA->OSPEEDR|=2<<(USART_TX_PIN<<1);           //高速
    GPIOA->PUPDR&=~(3<<(USART_TX_PIN<<1));          //无上下拉
    GPIOA->AFR[USART_TX_PIN/8]|=7<<((USART_TX_PIN%8)<<2);            //PA2复用为USART2_TX
    //初始化PA3(RX)
    GPIOA->MODER|=2<<(USART_RX_PIN<<1);             //PA2设置为复用模式
    GPIOA->OTYPER|=1<<USART_RX_PIN;                 //开漏。因为F401RE有外部上拉电阻,所以可以使用开漏输出
    GPIOA->OSPEEDR|=2<<(USART_RX_PIN<<1);           //高速
    GPIOA->PUPDR&=~(3<<(USART_RX_PIN<<1));          //无上下拉
    GPIOA->AFR[USART_RX_PIN/8]|=7<<((USART_RX_PIN%8)<<2);            //PA2复用为USART2_RX
    //设置中断优先级,第一个参数是中断号,第二个参数是抢占优先级,第三个参数是响应优先级
    //原来是设置为0抢占，0响应，这里也是一样的设置优先级为0，0。
    NVIC->IP[(uint32_t)USART_UX_IRQn] = 0x00;                 // 设置中断优先级
    //中断号除了低五位的位就表示ISERx中的x。而低五位表示ISERx中的第几位
    NVIC->ISER[(uint32_t)(USART_UX_IRQn) >> 5] = (uint32_t)(1UL << (((uint32_t)USART_UX_IRQn) & 0x1FUL));       //使能中断
    // my_nvic_set_priority(USART_UX_IRQn,0,0);
    // my_nvic_enable(USART_UX_IRQn);
}

//接收完成回调函数
void UART_RxCpltCallback() {
#if OS_CRITICAL_METHOD == 3u /* Allocate storage for CPU status register */
    OS_CPU_SR cpu_sr = 0u;
#endif

    //下面都是对临界资源的操作，所以也算是临界区
    OS_ENTER_CRITICAL();
    //由于这里已经在临界区的范围内了，所以没必要再手动关一次特定的中断
    // //这里需要关闭接收中断，防止在数据处理的过程中，数据被新的中断传来的数据覆盖
    // /*Disable the  interrupt*/
    // //Disable the UART Error Interrupt: (Frame error, noise error, overrun error)
    // USART_UX->CR3 &= ~USART_CR3_EIE;
    // //Disable the UART Data Register not empty（RXNE） Interrupt
    // USART_UX->CR1 &= ~USART_CR1_RXNEIE;
    if ((g_usart_rx_sta & 0x8000) == 0)             /* 接收未完成 */
    {
        //经过测试，picocom终端中一个回车键只会向32发送0x0d（默认配置）
        //所以需要在picocom中设置将0d映射为0x0d和0x0a
        if (g_usart_rx_sta & 0x4000)                /* 接收到了0x0d（即回车键） */
        {
            if (g_rx_buffer[0] != 0x62)             /* 接收到的不是0x0a（即不是换行键） */
            {
                g_usart_rx_sta = 0;                 /* 接收错误,重新开始 */
            }
            else                                    /* 接收到的是0x0a（即换行键） */
            {
                g_usart_rx_buf[g_usart_rx_sta & 0X3FFF] = 0x00; /* 添加结束符 */
                g_usart_rx_sta |= 0x8000;           /* 接收完成了 */
                //接收到一条完整的数据，发送信号量
                OSSemPost(uasrt_rx_sem);
            }
        }
        else                                        /* 还没收到0X0d（即回车键） */
        {
            if (g_rx_buffer[0] == 0x61)
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
    //同样的由于进入临界区关中断的存在，这里已经不需要再去使能特定的中断了
    OS_EXIT_CRITICAL();
    // /*Set the  interrupt*/                                   //前面的中断的设置其实没那么重要
    // //Enable the UART Error Interrupt: (Frame error, noise error, overrun error)
    // USART_UX->CR3 |= USART_CR3_EIE;
    // //Enable the UART Data Register not empty（RXNE） Interrupt
    // USART_UX->CR1 |= USART_CR1_RXNEIE;
}

//USART2h是在APB1总线上的。APB2总线的时钟频率是
void USART_Init(unsigned int bound) {
    /* Init the low level hardware : GPIO, CLOCK */
    UART_MspInit();

    /* Disable the peripheral */
    USART_UX->CR1 &=  ~USART_CR1_UE;            //这是为了防止串口在初始化的时候接收到数据

    /* Set the UART */
    //Set the overSampling
    USART_UX->CR1 &= ~USART_CR1_OVER8;                       //16倍过采样
    //Set the Baud and the overSampling
    double USART_DIV=(double)APB1FCK/(16*bound);            
    uint16_t mantissa = USART_DIV;
    uint16_t fraction = (USART_DIV - mantissa) *16+0.5;         //这里是为了四舍五入才加上0.5的
    mantissa+=fraction>>4;
    USART_UX->BRR = (mantissa << 4) | fraction;
    //Set the Word length
    USART_UX->CR1 &= ~USART_CR1_M;                          //8位数据
    //Set the Stop bits
    USART_UX->CR2 &= ~USART_CR2_STOP;                        //1位停止位
    //Set the Parity
    USART_UX->CR1 &= ~USART_CR1_PCE;                         //不使用校验位,如果有校验位会占用WORDLENGTH的位数
    //Set hardware flow control
    USART_UX->CR3 &= ~USART_CR3_RTSE;                        //不使用硬件流控制
    //Set the Mode, Receive and Transmit
    USART_UX->CR1 |= USART_CR1_TE | USART_CR1_RE;            //使能发送和接收
    
    /* Enable the peripheral */
    USART_UX->CR1 |= USART_CR1_UE;

    /*Set the  interrupt*/                                   //前面的中断的设置其实没那么重要
    //Enable the UART Error Interrupt: (Frame error, noise error, overrun error)
    USART_UX->CR3 |= USART_CR3_EIE;
    //Enable the UART Data Register not empty（RXNE） Interrupt
    USART_UX->CR1 |= USART_CR1_RXNEIE;
}

//串口1中断服务函数。只使能了接收中断
void USART_UX_IRQHandler(void) {
#if OS_CRITICAL_METHOD == 3u /* Allocate storage for CPU status register */
    OS_CPU_SR cpu_sr = 0u;
#endif

    //tell the OS that we are starting an ISR
    OS_ENTER_CRITICAL();
    OSIntEnter();
    //USART_UX外设的接口也算是一个临界资源，所以这里也算是临界区
    uint32_t isrflags   = USART_UX->SR;
    uint32_t cr1its     = USART_UX->CR1;
    
    /* UART in mode Receiver -------------------------------------------------*/
    //里面都是对临界资源的写操作，所以也算是临界区
    if (((isrflags & USART_SR_RXNE) != (uint32_t)0x00) && ((cr1its & USART_CR1_RXNEIE) != (uint32_t)0x00)){
        //因为缓冲区只有一个字节的大小，所以这里就不再对数组指针进行自增了（数组名是一个常量指针，也改不了）
        *g_rx_buffer = (uint8_t)(USART_UX->DR & (uint8_t)0x00FF);
        /*Call legacy weak Rx complete callback*/
        UART_RxCpltCallback();
    }
    OS_EXIT_CRITICAL();
    //tell the OS that we are leaving the ISR
    OSIntExit();
}

//本函数封装了printf函数，通过信号量的机制实现实现串口打印互斥的功能（因为串口的寄存器相当于是临界资源）
//需要注意的是，本函数只能在OS的任务中使用，因为需要使用信号量（中断中无法获取一个信号量）
int usart_send(const char *format, ...){
    int len=0;
    INT8U err=0;
    va_list args;
    //初始化可变参数
    va_start(args, format);
    //使用打印函数的时候需要使用信号量实现互斥。timeout设置为0，表示如果信号量被占用，就一直等待
    OSSemPend(uasrt_tx_sem, 0, &err);
    vprintf(format, args);
    OSSemPost(uasrt_tx_sem);
    //清理可变参数（相当于是一个free）
    va_end(args);
    return len;
}

//本函数实现串口的数据接收，这个任务的优先级要比较高，为了防止
void usart_receive(void* args){
    INT8U err=0;
    while(1){
        //等待中断处理函数释放信号量（uasrt_rx_semi应该是一个资源信号量,但是最大为1，所以也算是一个互斥信号量）
        OSSemPend(uasrt_rx_sem, 0, &err);
        //接收到数据后进入临界区，需要实现互斥（copilot说互斥信号量更好）
        usart_send("usart_receive:接收到数据\n");
	    usart_send("usart_receive:数据长度为：%d\n", g_usart_rx_sta & 0x3fff);
        usart_send("usart_receive:数据内容为：%s\n", g_usart_rx_buf);
        int index = 0,number_index=1;
        int decimal_point_seen = 0;
        float result[4] = {0.0f,0.0f,0.0f,0.0f},fraction=0.1f;
        while(g_usart_rx_buf[index]){
            if(g_usart_rx_buf[index] >= '0' && g_usart_rx_buf[index] <= '9'){
                if(decimal_point_seen){
                    result[number_index-1] = result[number_index-1] + (g_usart_rx_buf[index] - '0') * fraction;
                    fraction *= 0.1f;
                } else {
                    result[number_index-1] = result[number_index-1] * 10 + (g_usart_rx_buf[index] - '0');
                }
            } else if (g_usart_rx_buf[index] == '.'){
                decimal_point_seen = 1;
            } else if (g_usart_rx_buf[index] == ' '){
                number_index++;
                decimal_point_seen = 0;
                fraction = 0.1f;
            }
            index++;
        }
        usart_send("the p float number is %d,the i float number is %d,the d float number is %d\n",(int)(result[0]*100),(int)(result[1]*100),(int)(result[2]*100));
        roll_rate_PID.P = result[0];
        pitch_rate_PID.P = result[0];
        roll_rate_PID.I = result[1];
        pitch_rate_PID.I = result[1];
        roll_rate_PID.D = result[2];
        pitch_rate_PID.D = result[2];
        usart_send("the input is: %d %d %d %d\n",(int)(result[0]*100),(int)(result[1]*100),(int)(result[2]*100),(int)(result[3]*100));
        roll_angle_PID.P = result[3];
        pitch_angle_PID.P = result[3];
        
        roll_rate_PID.Integ = 0;
        pitch_rate_PID.Integ = 0;
        
        //由于中断不能获取信号量，所以这里需要使用共享内存的方式告诉ISR，数据已经处理完了，并且还能清除数据的长度信息
        //也正是这个标记的存在，使得uasrt_rx_sem信号量数量最多只有一个
        g_usart_rx_sta = 0;			//清除接收标记
        //接收完一条数据之后不能释放信号量因为数据是由中断函数产生的，所以释放信号量也只能由中断函数执行
    }
}