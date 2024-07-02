#include "myiic.h"
#include "os_cpu.h"
#include "stm32f401xe.h"
#include "stm32f4xx.h"
#include "tools.h"
#include "NVIC.h"
#include "ucos_ii.h"
#include <stdint.h>

static int iic_status = 0;
INT32U IIC_TIME = 0;

void IIC_SCL(uint8_t x)
{
    if (x)
    {
        GPIOC->ODR |= (1 << IIC_SCL_GPIO_PIN); // 设置 SCL 引脚为高电平
    }
    else
    {
        GPIOC->ODR &= ~(1 << IIC_SCL_GPIO_PIN); // 设置 SCL 引脚为低电平
    }
}

void IIC_SDA(uint8_t x)
{
    if (x)
    {
        GPIOC->ODR |= (1 << IIC_SDA_GPIO_PIN); // 设置 SDA 引脚为高电平
    }
    else
    {
        GPIOC->ODR &= ~(1 << IIC_SDA_GPIO_PIN); // 设置 SDA 引脚为低电平
    }
}

int getIICStatus() { return iic_status; }

void clearIICStatus() { iic_status = 0; }

/**
 * @brief       初始化IIC
 * @param       无
 * @retval      无
 */
void iic_init(void)
{
    // 使能 GPIOC 时钟
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    // 配置 PC10 (SCL) 引脚为推挽输出
    GPIOC->MODER &= ~(GPIO_MODER_MODER10);  // 清除原来的模式
    GPIOC->MODER |= (GPIO_MODER_MODER10_0); // 设置为推挽输出模式

    GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_10); // 清除原来的输出类型，设置为推挽输出模式

    GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR10); // 设置 GPIO 速度为全高速模式

    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR10); // 清除上下拉设置
    GPIOC->PUPDR |= GPIO_PUPDR_PUPDR10_0;  // 这里我们默认设置为上拉模式

    // 配置 PC12 (SDA) 引脚为开漏输出
    GPIOC->MODER &= ~(GPIO_MODER_MODER12);  // 清除原来的模式
    GPIOC->MODER |= (GPIO_MODER_MODER12_0); // 设置为输出模式

    GPIOC->OTYPER |= GPIO_OTYPER_OT_12; // 设置为开漏输出类型

    GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR12); // 设置 GPIO 速度，设置为超高速模式

    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR12);  // 清除上下拉设置
    GPIOC->PUPDR |= (GPIO_PUPDR_PUPDR10_0); // 设置为上拉模式
}

/** TIM4_CH1 --- pb6
 */
void iic_TIM_Init(uint16_t arr, uint16_t psc)
{
    RCC->AHB1ENR |= (1 << 1); // 打开GPIOB时钟

    RCC->APB1ENR |= (1 << 2); // 打开TIM4时钟

    GPIOB->MODER |= (0x2 << 12);
    GPIOB->OSPEEDR |= (0x2 << 12);
    GPIOB->PUPDR |= (0x2 << 12);

    GPIOB->AFR[0] &= ~(0xf << 24);
    GPIOB->AFR[0] |= (0x2 << 24);

    TIM4->CR1 &= ~(0Xf << 1);
    TIM4->CR1 |= (0x1 << 7); // 预装载影子寄存器
    TIM4->SMCR &= ~(0x7);    // 选择内部时钟 Slave mode disable,这样定时器将不会受到触发事件的影响

    TIM4->PSC = psc; // 设置分频系数
    TIM4->ARR = arr; // 设置重装载寄存器

    TIM4->DIER |= (0x1 << 0);
    TIM4->CR1 |= (0x1 << 0); // 使能CEN计数器
    my_nvic_set_priority(TIM4_IRQn, 0, 3);
    my_nvic_enable(TIM4_IRQn);
    TIM4->CNT -= 0xffff;
}

void TIM4_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u /* Allocate storage for CPU status register */
    OS_CPU_SR cpu_sr = 0u;
#endif
    OS_ENTER_CRITICAL();
    if (OSRunning == OS_TRUE)
    {
        if (OSIntNesting < 255u)
        {
            OSIntNesting++; /* Increment ISR nesting level                        */
        }
    }
    OS_EXIT_CRITICAL();
    if (TIM4->SR & (0x1 << 0))
    {
        IIC_TIME++;
    }
    TIM4->SR &= ~(0xffff); // 清除中断标志位
    OS_ENTER_CRITICAL();
    if (OSRunning == OS_TRUE)
    {
        if (OSIntNesting > 0u)
        { /* Prevent OSIntNesting from wrapping       */
            OSIntNesting--;
        }
    }
    OS_EXIT_CRITICAL();
}

/**
 * @brief       IIC延时函数,用于控制IIC读写速度
 * @param       无
 * @retval      无
 */
static void iic_delay(void) { delay_used_by_iic(1); /* 10us的延时 */ }

/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void iic_start(void)
{
    IIC_SDA(1);
    IIC_SCL(1);
    iic_delay();
    IIC_SDA(0); /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    iic_delay();
    IIC_SCL(0); /* 钳住I2C总线，准备发送或接收数据 */
    iic_delay();
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void iic_stop(void)
{
    IIC_SDA(0); /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    iic_delay();
    IIC_SCL(1);
    iic_delay();
    IIC_SDA(1); /* 发送I2C总线结束信号 */
    iic_delay();
}

/**
 * @brief       等待应答信号到来
 * @param       无
 * @retval      1，接收应答失败
 *              0，接收应答成功
 */
uint8_t iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    IIC_SDA(1); /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    iic_delay();
    IIC_SCL(1); /* SCL=1, 此时从机可以返回ACK */
    iic_delay();

    while (IIC_READ_SDA) /* 等待应答 */
    {
        waittime++;

        if (waittime > 250)
        {
            iic_stop();
            rack = 1;
            break;
        }
    }

    IIC_SCL(0); /* SCL=0, 结束ACK检查 */
    iic_delay();
    iic_status |= rack;
    return rack;
}

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void iic_ack(void)
{
    IIC_SDA(0); /* SCL 0 -> 1  时 SDA = 0,表示应答 */
    iic_delay();
    IIC_SCL(1); /* 产生一个时钟 */
    iic_delay();
    IIC_SCL(0);
    iic_delay();
    IIC_SDA(1); /* 主机释放SDA线 */
    iic_delay();
}

/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void iic_nack(void)
{
    IIC_SDA(1); /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    iic_delay();
    IIC_SCL(1); /* 产生一个时钟 */
    iic_delay();
    IIC_SCL(0);
    iic_delay();
}

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 * @note        发送前的SDL状态应该为低电平，发送后SDA依旧为低电平
 */
void iic_send_byte(uint8_t data)
{
    uint8_t t;

    for (t = 0; t < 8; t++)
    {
        IIC_SDA((data & 0x80) >> 7); /* 高位先发送 */
        iic_delay();
        IIC_SCL(1);
        iic_delay();
        IIC_SCL(0);
        data <<= 1; /* 左移1位,用于下一次发送 */
    }
    IIC_SDA(1); /* 发送完成, 主机释放SDA线 */
}

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++) /* 接收1个字节数据 */
    {
        receive <<= 1; /* 高位先输出,所以先收到的数据位要左移 */
        IIC_SCL(1);
        iic_delay();

        if (IIC_READ_SDA)
        {
            receive++;
        }

        IIC_SCL(0);
        iic_delay();
    }

    if (!ack)
    {
        iic_nack(); /* 发送nACK */
    }
    else
    {
        iic_ack(); /* 发送ACK */
    }

    return receive;
}

uint8_t iic_send_bytes(uint8_t *buffer, int size)
{
    for (int i = 0; i < size; i++)
    {
        iic_send_byte(buffer[i]); // 发送数据
        if (iic_wait_ack() == 1)
        { // 等待应答
            return 1;
        }
    }
    return 0;
}

void iic_read_bytes(uint8_t *buffer, int size)
{
    // 连续读取size个字节
    for (int i = 0; i < size; i++)
    {
        // 读出寄存器数据
        if (i < size - 1)
        {
            buffer[i] = iic_read_byte(1); // 前size-1次发送ACK
        }
        else
        {
            buffer[i] = iic_read_byte(0); // 发送nACK
        }
    }
}

uint8_t Peripheral_ReadByte(uint8_t devAddress, uint16_t address)
{
    uint8_t res;
    iic_start();               // 起始信号
    iic_send_byte(devAddress); // 发送设备地址+写信号
    iic_wait_ack();
    iic_send_byte(address); // 发送存储单元地址，从0x00开始
    iic_wait_ack();
    iic_stop(); // 停止信号

    iic_start();                   // 起始信号
    iic_send_byte(devAddress + 1); // 发送设备地址+读信号
    iic_wait_ack();
    res = iic_read_byte(0); // 读出寄存器数据
    iic_nack();             // 发送nACK

    iic_stop(); // 停止信号
    return res;
}

void Peripheral_SendByte(uint8_t devAddress, uint8_t addr, uint8_t data)
{
    iic_start();               // 起始信号
    iic_send_byte(devAddress); // 发送设备地址+写信号,发送通讯地址
    iic_wait_ack();            // 等待应答
    iic_send_byte(addr);       // 发送存储单元地址，从0x00开始
    iic_wait_ack();            // 等待应答
    iic_send_byte(data);       // 发送数据
    iic_wait_ack();            // 等待应答
    iic_stop();                // 停止信号
}

void Peripheral_ReadBytes(uint8_t devAddress, uint8_t address, uint8_t *buffer, uint8_t size)
{
    iic_start();               // 起始信号
    iic_send_byte(devAddress); // 发送设备地址+写信号
    iic_wait_ack();
    iic_send_byte(address); // 发送存储单元地址，从0x00开始
    iic_wait_ack();

    iic_start();                   // 起始信号
    iic_send_byte(devAddress + 1); // 发送设备地址+读信号
    iic_wait_ack();
    iic_read_bytes(buffer, size);
    iic_stop(); // 停止信号
}

void Peripheral_SendBytes(uint8_t devAddress, uint8_t addr, uint8_t *buffer, uint8_t size)
{
    iic_start();               // 起始信号
    iic_send_byte(devAddress); // 发送设备地址+写信号,发送通讯地址
    iic_wait_ack();
    iic_send_byte(addr); // 发送存储单元地址，从0x00开始
    iic_wait_ack();
    iic_send_bytes(buffer, size);
    iic_stop(); // 停止信号
}
