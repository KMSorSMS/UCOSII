#include "HMC.h"

float Xoffset = 0, Yoffset = 0;


uint16_t Single_Read_HMC5883(uint16_t address) {
    uint16_t res;

    iic_start();                                //起始信号
    iic_send_byte(HMC_Slave_Address);           //发送设备地址+写信号
    if(iic_wait_ack() ==1){						//等待应答
		printf("ack_err\n");
	}

    iic_send_byte(address);                     //发送存储单元地址，从0x00开始    
    if(iic_wait_ack() ==1){						//等待应答
		printf("ack_err\n");
	}
    iic_stop();                                 //停止信号

    iic_start();                                //起始信号
    iic_send_byte(HMC_Slave_Address + 1);       //发送设备地址+读信号
    if(iic_wait_ack() ==1){						//等待应答
		printf("ack_err\n");
	}
    res = iic_read_byte(0);                     //读出寄存器数据
    iic_nack();                                 //发送nACK

    iic_stop();                                 //停止信号
    return res;
}
//******************************************************
//连续读出QMC5883内部角度数据，地址范围0x00~0x05
//******************************************************
void Multiple_Read_HMC5883(int16_t* x, int16_t* y, int16_t* z)
{
//    uint8_t i = 0;
    uint8_t BUF[HMC_BUF_SIZE];

//    //连续读出6个数据，存储在BUF中
//    iic_start();                            //起始信号
//    iic_send_byte(HMC_Slave_Address);       //发送设备地址+写信号
//    iic_wait_ack();                         //等待应答
//    iic_send_byte(0x03);                    //发送存储单元地址，从0x03开始
//    iic_wait_ack();                         //等待应答
//    iic_start();                            //起始信号
//    iic_send_byte(HMC_Slave_Address + 1);   //发送设备地址+读信号
//    iic_wait_ack();                         //等待应答
//    for (i = 0; i < HMC_BUF_SIZE; i++)
//    {
//        if (i != HMC_BUF_SIZE - 1)
//        {
//            BUF[i] = iic_read_byte(1);      //读出寄存器数据
//        }
//        else
//        {
//            BUF[i] = iic_read_byte(0);      //读出寄存器数据
//        }
//    }
//    iic_stop();                             //停止信号
	Peripheral_ReadBytes(HMC_Slave_Address,0x03,BUF,HMC_DATA_SIZE);
    *x = (int16_t)(BUF[0] << 8) | (BUF[1]);
    *z = (int16_t)(BUF[2] << 8) | (BUF[3]);
    *y = (int16_t)(BUF[4] << 8) | (BUF[5]);
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


//2初始化HMC5883，根据需要请参考pdf进行修改****
void HMC5883L_Init()
{
//    //在正式初始化之前，我们需要做一个磁场方向的校准
//    int i = 0;
//    float GaX = 0, GaY = 0, GaXmin = 0, GaXmax = 0, GaYmin = 0, GaYmax = 0;
//    short x, y, z;
//    while (i != 200)
//    {
//        Multiple_Read_HMC5883(&x, &y, &z);
//        GaX = x;
//        GaY = y;
//        GaXmax = GaXmax < GaX && GaX - GaXmax < 50 ? GaX : GaXmax;

//        GaXmin = GaXmin > GaX && GaXmin - GaX < 50 ? GaX : GaXmin;

//        GaYmax = GaYmax < GaY && GaY - GaYmax < 50 ? GaY : GaYmax;

//        GaYmin = GaYmin > GaY && GaYmin - GaY < 50 ? GaY : GaYmin;

//        HAL_Delay(1);
//        printf("\ncounts %d", i);
//        i++;
//    }

//    Xoffset = (GaXmax + GaXmin) / 2;
//    Yoffset = (GaYmax + GaYmin) / 2;
//    printf("\noffsetX: %f , offsetY: %f\n", Xoffset, Yoffset);
//    HAL_Delay(1000 * 5);//延时5s

    // //作为测试，我们先获取设备当前的配置寄存器的值
    // for (int i = 0;i < 13;i++)//读取13个寄存器的值
    // {

    //     printf("%d:%#X\r\n", i, Single_Read_HMC5883(i));
    // }
    //HMC5883L配置寄存器初始化设置
    //------------------------------------
    HMC5883_SendByte(0x78, 0x00);          //0111 1000       
    //------------------------------------
    HMC5883_SendByte(0x40, 0x01);          //0100 0000
    //------------------------------------
    HMC5883_SendByte(0x00, 0x02);          //0000 0000
    // //作为测试，我们获取设备修改后的配置寄存器的值
    // printf("\n------------after initialize------------\n");
    // for (int i = 0;i < 13;i++)//读取13个寄存器的值
    // {

    //     printf("%d:%#X\r\n", i, Single_Read_HMC5883(i));
    // }
}

