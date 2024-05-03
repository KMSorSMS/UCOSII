#ifndef __HMC_H
#define __HMC_H

#include "main.h"
#include "myiic.h"
extern float Xoffset,Yoffset;

extern void Multiple_Read_HMC5883(int16_t* x, int16_t* y, int16_t* z);
extern void HMC5883L_Init(void);
extern void HMC5883_SendByte(uint8_t data, uint8_t addr);

#define HMC_BUF_SIZE 20
#define HMC_DATA_SIZE 6
#define HMC_Slave_Address   0x3C     //定义器件在IIC总线中的从地址  write


#endif
