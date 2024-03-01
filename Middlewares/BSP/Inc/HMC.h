#pragma once
#ifndef __HMC_H
#define __HMC_H

#include "myiic.h"
#include "main.h"

extern float Xoffset,Yoffset;

extern void Multiple_Read_HMC5883(int16_t* x, int16_t* y, int16_t* z);
extern void HMC5883L_Init(void);
extern uint16_t Single_Read_HMC5883(uint16_t address);
extern void HMC5883_SendByte(uint8_t data, uint8_t addr);
extern void MULTIPLE_READ_HMC5883(int16_t* x, int16_t* y, int16_t* z);
extern void HMC5883L_INIT(void);

#define HMC_BUF_SIZE 20
#define HMC_DATA_SIZE 6
#define HMC_Slave_Address   0x3C     //定义器件在IIC总线中的从地址  write


#endif
