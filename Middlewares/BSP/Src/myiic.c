#include "myiic.h"

/**
 * @brief       初始化IIC
 * @param       无
 * @retval      无
 */
void iic_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    IIC_SCL_GPIO_CLK_ENABLE();  /* SCL引脚时钟使能 */
    IIC_SDA_GPIO_CLK_ENABLE();  /* SDA引脚时钟使能 */

    GPIO_InitStructure.Pin = IIC_SCL_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;        /* 推挽输出 */
    GPIO_InitStructure.Pull = GPIO_PULLUP;                /* 上拉 */
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;      /* 高速 */
    HAL_GPIO_Init(IIC_SCL_GPIO_PORT, &GPIO_InitStructure);/* SCL */

    GPIO_InitStructure.Pin = IIC_SDA_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;        /* 开漏输出 */
    HAL_GPIO_Init(IIC_SDA_GPIO_PORT, &GPIO_InitStructure);/* SDA */
    /* SDA引脚模式设置,开漏输出,上拉, 这样就不用再设置IO方向了, 开漏输出的时候(=1), 也可以读取外部信号的高低电平 */

    iic_stop();     /* 停止总线上所有设备 */
}

/**
 * @brief       IIC延时函数,用于控制IIC读写速度
 * @param       无
 * @retval      无
 */
static void iic_delay(void)
{
    HAL_Delay(200);    /* 1ms的延时 */
}

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
    IIC_SDA(0);     /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    iic_delay();
    IIC_SCL(0);     /* 钳住I2C总线，准备发送或接收数据 */
    iic_delay();
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void iic_stop(void)
{
    IIC_SDA(0);     /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    iic_delay();
    IIC_SCL(1);
    iic_delay();
    IIC_SDA(1);     /* 发送I2C总线结束信号 */
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

    IIC_SDA(1);     /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    iic_delay();
    IIC_SCL(1);     /* SCL=1, 此时从机可以返回ACK */
    iic_delay();

    while (IIC_READ_SDA)    /* 等待应答 */
    {
        waittime++;

        if (waittime > 250)
        {
            iic_stop();
            rack = 1;
            break;
        }
    }

    IIC_SCL(0);     /* SCL=0, 结束ACK检查 */
    iic_delay();
    return rack;
}

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void iic_ack(void)
{
    IIC_SDA(0);     /* SCL 0 -> 1  时 SDA = 0,表示应答 */
    iic_delay();
    IIC_SCL(1);     /* 产生一个时钟 */
    iic_delay();
    IIC_SCL(0);
    iic_delay();
    IIC_SDA(1);     /* 主机释放SDA线 */
    iic_delay();
}

/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void iic_nack(void)
{
    IIC_SDA(1);     /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    iic_delay();
    IIC_SCL(1);     /* 产生一个时钟 */
    iic_delay();
    IIC_SCL(0);
    iic_delay();
}

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 */
void iic_send_byte(uint8_t data)
{
    uint8_t t;
    
    for (t = 0; t < 8; t++)
    {
        IIC_SDA((data & 0x80) >> 7);    /* 高位先发送 */
        iic_delay();
        IIC_SCL(1);
        iic_delay();
        IIC_SCL(0);
        data <<= 1;     /* 左移1位,用于下一次发送 */
    }
    IIC_SDA(1);         /* 发送完成, 主机释放SDA线 */
}

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
    {
        receive <<= 1;  /* 高位先输出,所以先收到的数据位要左移 */
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
        iic_nack();     /* 发送nACK */
    }
    else
    {
        iic_ack();      /* 发送ACK */
    }

    return receive;
}
uint8_t Peripheral_ReadByte(uint8_t devAddress,uint16_t address){
	uint8_t res;
    iic_start();                            //起始信号
    iic_send_byte(devAddress);         		//发送设备地址+写信号
    if(iic_wait_ack() ==1){				    //等待应答
        printf("ack_err\n");
    }
    iic_send_byte(address);                 //发送存储单元地址，从0x00开始    
    if(iic_wait_ack() ==1){				    //等待应答
        printf("ack_err\n");
    }
    iic_stop();                             //停止信号

    iic_start();                            //起始信号
    iic_send_byte(devAddress + 1);     		//发送设备地址+读信号
    if(iic_wait_ack() ==1){				    //等待应答
        printf("ack_err\n");
    }
    res = iic_read_byte(0);                 //读出寄存器数据
    iic_nack();                             //发送nACK

    iic_stop();                             //停止信号
    return res;
}
void Peripheral_SendByte(uint8_t devAddress, uint8_t addr,uint8_t data){
	 iic_start();                            //起始信号
    iic_send_byte(devAddress);         //发送设备地址+写信号,发送通讯地址
    if(iic_wait_ack() ==1){				    //等待应答
		printf("ack_err\n");
	}
    iic_send_byte(addr);                    //发送存储单元地址，从0x00开始
    if(iic_wait_ack() ==1){				    //等待应答
		printf("ack_err\n");
	}
    iic_send_byte(data);                    //发送数据
    if(iic_wait_ack() ==1){				    //等待应答
		printf("ack_err\n");
	}
    iic_stop();                             //停止信号
}

 
void Peripheral_ReadBytes(uint8_t devAddress,uint16_t address,uint8_t* Buffer ,uint8_t size){
	uint8_t i;
	
    iic_start();                            //起始信号
    iic_send_byte(devAddress);         		//发送设备地址+写信号
    if(iic_wait_ack() ==1){				    //等待应答
        printf("ack_err\n");
    }
    iic_send_byte(address);                 //发送存储单元地址，从0x00开始    
    if(iic_wait_ack() ==1){				    //等待应答
        printf("ack_err\n");
    }
	
	iic_start();                            //起始信号
	iic_send_byte(devAddress + 1);     		//发送设备地址+读信号
	if(iic_wait_ack() ==1){				    //等待应答
		printf("ack_err\n");
	}
	//连续读取size个字节
	for(i=0;i<size;i++){
        //读出寄存器数据
		if(i<size-1){
			Buffer[i] = iic_read_byte(1);			//前size-1次发送ACK
		}
		else{
			Buffer[i] = iic_read_byte(0); 						//发送nACK
		}
	}
    iic_stop();                             //停止信号
}

void Peripheral_SendBytes(uint8_t devAddress, uint8_t addr,uint8_t*Buffer, uint8_t size){
	uint8_t i;
	iic_start();                            //起始信号
    iic_send_byte(devAddress);         //发送设备地址+写信号,发送通讯地址
    if(iic_wait_ack() ==1){				    //等待应答
		printf("ack_err\n");
	}
    iic_send_byte(addr);                    //发送存储单元地址，从0x00开始
    if(iic_wait_ack() ==1){				    //等待应答
		printf("ack_err\n");
	}
    for(i=0;i<size;i++){
		iic_send_byte(Buffer[i]);                    //发送数据
		if(iic_wait_ack() ==1){				    //等待应答
			printf("ack_err\n");
		}
	}
    iic_stop();                             //停止信号
}










