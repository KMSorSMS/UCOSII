#include "HMC.h"

//******************************************************
//连续读出HMC5883内部角度数据，地址范围0x00~0x05
//******************************************************
void Multiple_Read_HMC5883(int16_t* x, int16_t* y, int16_t* z)
{
    uint8_t buf[6];
	Peripheral_ReadBytes(HMC_Slave_Address,0x03,buf,HMC_DATA_SIZE);
    *x = (int16_t)(buf[0] << 8) | (buf[1]);
    *z = (int16_t)(buf[2] << 8) | (buf[3]);
    *y = (int16_t)(buf[4] << 8) | (buf[5]);
}

void HMC5883_SendByte(uint8_t data, uint8_t addr)
{
    iic_start();                           //起始信号
    iic_send_byte(HMC_Slave_Address);      //发送设备地址+写信号,发送通讯地址
    iic_wait_ack();                        //等待应答
    iic_send_byte(addr);                   //发送存储单元地址，从0x00开始
    iic_wait_ack();                        //等待应答
    iic_send_byte(data);                   //发送数据
    iic_wait_ack();                        //等待应答
    iic_stop();                            //停止信号
}


//2初始化HMC5883，根据需要请参考pdf进行修改
void HMC5883L_Init()
{
    //HMC5883L配置寄存器初始化设置
    //------------------------------------
    HMC5883_SendByte(0x78, 0x00);          //0111 1000       
    //------------------------------------
    HMC5883_SendByte(0x40, 0x01);          //0100 0000
    //------------------------------------
    HMC5883_SendByte(0x00, 0x02);          //0000 0000
}

