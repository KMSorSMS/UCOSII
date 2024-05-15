#ifndef __MyIIC_H
#define __MyIIC_H

#include "main.h"
#include "stm32f4xx.h"


#define IIC_SCL_GPIO_PORT               GPIOC
#define IIC_SCL_GPIO_PIN                (10)   // SCL 对应 PC10

#define IIC_SDA_GPIO_PORT               GPIOC
#define IIC_SDA_GPIO_PIN                (12)   // SDA 对应 PC12

#define IIC_READ_SDA     ((IIC_SDA_GPIO_PORT->IDR>>IIC_SDA_GPIO_PIN)&0b1) /* 读取SDA */

// 这里读取iic的数据读取状态
int getIICStatus();
// 下面清除iic的数据状态
void clearIICStatus();


/* IIC所有操作函数 */
extern void iic_init(void);            /* 初始化IIC的IO口 */
extern void iic_start(void);           /* 发送IIC开始信号 */
extern void iic_stop(void);            /* 发送IIC停止信号 */
extern void iic_ack(void);             /* IIC发送ACK信号 */
extern void iic_nack(void);            /* IIC不发送ACK信号 */
extern uint8_t iic_wait_ack(void);     /* IIC等待ACK信号 */
extern void iic_send_byte(uint8_t txd);/* IIC发送一个字节 */
extern uint8_t iic_read_byte(unsigned char ack);/* IIC读取一个字节 */
uint8_t iic_send_bytes(uint8_t* buffer,int size); /* 发送多个字节 */
void iic_read_bytes(uint8_t* buffer,int size); /* 读取多个字节 */
extern INT32U IIC_TIME;
extern void iic_TIM_Init(uint16_t arr ,uint16_t psc);

extern void Peripheral_SendByte(uint8_t devAddress,uint8_t data, uint8_t addr);
extern void Peripheral_ReadBytes(uint8_t devAddress,uint8_t address,uint8_t* buffer ,uint8_t size);
extern void Peripheral_SendBytes(uint8_t devAddress, uint8_t addr,uint8_t*buffer, uint8_t size);

static inline void delay_used_by_iic(INT32U delay_10us)
{
    INT32U cur_time = IIC_TIME;
    while (IIC_TIME - cur_time < delay_10us);
}
			  
#endif
