#pragma once
#ifndef MS5611_H
#define MS5611_H
#include "myiic.h"
#include "usart.h"
#include "main.h"
/*

the Prom[0] is : 0
the Prom[1] is : 0XB74D Pressure sensitivity | SENST1
the Prom[2] is : 0XBD24 Pressure offset | OFFT1
the Prom[3] is : 0X7328 Temperature coefficient of pressure sensitivity | TCS
the Prom[4] is : 0X6A3E Temperature coefficient of pressure offset | TCO
the Prom[5] is : 0X81C1 Reference temperature | TREF
the Prom[6] is : 0X6E62 Temperature coefficient of the temperature | TEMPSENS 
the Prom[7] is : 0X2

D1 Digital pressure value
D2 Digital temperature value 
*/
	#define MS5611_ADDR             0xEE   //0xee //
	#define MS5611_RESETCMD 0x1E
	#define MS5611_CD1_256CMD 0x40
	#define MS5611_CD1_512CMD 0x42
	#define MS5611_CD1_1024CMD 0x44 
	#define MS5611_CD1_2048CMD 0x46
	#define MS5611_CD1_4096CMD 0x48
	#define MS5611_CD2_256CMD 0x50
	#define MS5611_CD2_512CMD 0x52
	#define MS5611_CD2_1024CMD 0x54
	#define MS5611_CD2_2048CMD 0x56
	#define MS5611_CD2_4096CMD 0x58
	#define MS5611_ADC_READ 0x00
	#define CMD_PROM_RD             0xA0 // Prom read command
	#define PROM_NB                 8
	#define MS5611_OSR				0x08	//CMD_ADC_4096
	extern float Read_MS5611_TEMP(void);
	extern float Read_MS5611_PRE(void);
	extern void MS5611_Init(void);
	extern uint16_t ms5611_prom[PROM_NB+1];
	extern void MY_MS5611_READ_D(uint8_t cmd, uint32_t* D);
	extern void MS5611_Read_D(uint8_t cmd, uint32_t* D);
#endif

