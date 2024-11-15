#include "main.h"
#include "HMC.h"
#include "MPU6050.h"
#include "Motor.h"
#include "NVIC.h"
#include "OS_stk.h"
#include "PID.h"
#include "Reciever.h"
#include "os_cpu.h"
#include "tools.h"
#include "myiic.h"
#include "ucos_ii.h"
#include "gy86_task.h"
#include "motor_change.h"
#include "os_trace.h"
#include "usart.h"
#include "Madgwick.h"
OS_STK my_task_0[MY_TASK_SIZE_0];
OS_STK my_task_1[MY_TASK_SIZE_1];
OS_STK my_task_2[MY_TASK_SIZE_2];
OS_STK madgwick_stk[MADGWICK_TASK_SIZE];
OS_STK uasrt_rx_task[USART_RX_TASK_SIZE];
OS_STK motor_change_task_stk[MOTOR_TASK_STK_SIZE];
OS_EVENT *uasrt_tx_sem;
OS_EVENT *uasrt_rx_sem;
OS_EVENT *madgwick_sem;
// 写一个时钟初始化的函数，配置为HSE，PLLM为4，PLLN为84，PLLP分频为2，PLLQ分频为4，还有AHB的地方分频为1 ，得到主频为84Mhz
void RCC_Configuration(void)
{
    // 先把PLL和PLL2S disable了
    RCC->CR &= ~0x05000000;
    // 先该各个分频系数,并且选择PLLSRC，把HSE设置为PLL的输入源
    RCC->PLLCFGR = 0b00000100010000000001010100000100; // 0x4401504的二进制是：0000 0100 0100 0000 0001 0101 0000 0100
    // 上面的配置是：PLLM=4, PLLN=84, PLLP=2, PLLQ=4，并且设置HSE为PLL的输入源
    // 设置AHB的分频系数为1
    RCC->CFGR &= ~0xF0;
    // 设置APB1的分频系数为2，APB2的分频系数为1
    RCC->CFGR |= 0x1000;
    RCC->CFGR &= ~0xE000;
    // 设置启动HSE，开启PLL和PLL2S
    RCC->CR |= 0b00000101000000010000000000000000; // 0x5010000
    // 加入保护代码，检查HSE和PLL、PLL2S的启动状态：
    while ((RCC->CR & 0x00020000) == 0); // 等待HSE启动成功
    while ((RCC->CR & 0x02000000) == 0); // 等待PLL启动成功
    while ((RCC->CR & 0x08000000) == 0); // 等待PLL2S启动成功
    // HSE启动成功后，使能FLASH预存取缓冲区
    FLASH->ACR |= FLASH_ACR_PRFTEN;
    // 设置FLASH的延时周期
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS;
    // 更改系统的时钟源为PLL
    RCC->CFGR |= 0x00000002;
    // 关闭HSI
    RCC->CR &= ~0x00000001;
    // 接下来我们需要设置外设的时钟使能，点灯的时候用到AHB1上的GPIOA
    RCC->AHB1ENR |= 0x00000001;
}

OS_MEM *CommTxBuf;
INT8U   CommTxPart[100][32];


int main()
{
    // 时钟初始化
    RCC_Configuration();
    // 启动systick中断
    OS_CPU_SysTickInitFreq(84000000); // 84Mhz
    my_nvic_priorityGroupConfig(4);
    // LED2初始化
    LED2_Init();
    //usart初始化
    USART_Init(115200);
    usart_send("usart_send:Hello World\n");
    PWM_TIM_Init(20000-1,84-1);
    iic_TIM_Init(10-1,84-1);
    MotorPWMInit();
    iic_init();
    MPU6050_Init();
    HMC5883L_Init();
    OS_TRACE_INIT();
    Init_PID();
    OSInit();
    // 创建串口发送互斥信号量(需要注意的是信号量的创建只能在OSInit之后，因为需要使用OSInit中的事件队列)
    uasrt_tx_sem=OSSemCreate(1);
    uasrt_rx_sem=OSSemCreate(0);
    madgwick_sem=OSSemCreate(0);
    INT8U err=0;
    CommTxBuf = OSMemCreate(CommTxPart, 100, 32, &err);
    // OS_TRACE_START();
    // 创建两个个自己的任务
    // pnext用于指向任务扩展部分，暂时没有用到
    // opt是可选项，根据被置位的位来进行一些额外的操作，暂时没有用到
    (void)OSTaskCreateExt(my_task_0_t_, (void *)0, &my_task_0[MY_TASK_SIZE_0 - 1u], 12,1,my_task_0,sizeof(my_task_0),NULL,0,"LED_OFF");
    // (void)OSTaskCreateExt(rate_pid, (void *)0, &my_task_1[MY_TASK_SIZE_1 - 1u], 9,2,my_task_1,sizeof(my_task_1),NULL,0,"LED_ON");
    (void)OSTaskCreateExt(GY86_task, 0, &my_task_2[MY_TASK_SIZE_2-1u], 8,3,my_task_2,sizeof(my_task_2),NULL,0,"GY86");
    //串口接收数据任务（发送不创建任务，而是直接使用线程安全的print）
    (void)OSTaskCreateExt(usart_receive, (void *)0, &uasrt_rx_task[USART_RX_TASK_SIZE - 1u], 11,4, uasrt_rx_task, sizeof(uasrt_rx_task), NULL, 0, "USART_RX");
    // (void)OSTaskCreateExt(angle_pid,(void*)0,&motor_change_task_stk[MOTOR_TASK_STK_SIZE-1u],10,5,motor_change_task_stk,sizeof(motor_change_task_stk),NULL,0,"motor_change");
    // (void)OSTaskCreateExt(madgwick_task,(void*)0,&madgwick_stk[MADGWICK_TASK_SIZE-1u],7,5,madgwick_stk,sizeof(madgwick_stk),NULL,0,"madgwick");
    // OS启动
    OSStart();
    return 0;
}

void my_task_0_t_(void *args)
{
    while (1)
    {
        LED2_OFF()
        // delay_used_by_iic(100*1000*3);
        // LED2_ON();
        // delay_used_by_iic(100*1000*3);
        // 任务0是关灯，关完后调用OS延时函数--OSTimeDly()
        // usart_send("LED2_OFF\n");
        OSTimeDly(1000 * 4); // 延时4s，因为一个tick是1毫秒
        // 任务一采用点灯，点完后调用OS延时函数--OSTimeDly()
        LED2_ON()
        // usart_send("LED2_ON\n");
        OSTimeDly(1000 * 2); // 延时2s，因为一个tick是1毫秒
    }
}

// void my_task_1_t_(void *args)
// {
//     while (1)
//     {
//         // 任务一采用点灯，点完后调用OS延时函数--OSTimeDly()
//         LED2_ON()
//         // usart_send("LED2_ON\n");
//         OSTimeDly(1000 * 2); // 延时2s，因为一个tick是1毫秒
//     }
// }