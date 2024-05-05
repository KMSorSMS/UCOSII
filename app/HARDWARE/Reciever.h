#pragma once
#include <stdint.h>
#ifndef __RECEIVER_H
#define __RECEIVER_H


#include "main.h"
void PWM_TIM_Init(uint16_t arr, uint16_t psc);
void TIM2_IRQHandler(void);
//	void PWM_TIM4_Init(uint16_t arr,uint16_t psc);
void TIM4_IRQHandler(void);
void print_capture_pwm(uint16_t arr);
int PPMtoPWM(uint8_t ppm);
extern uint16_t TIM2_Channel1_DataBuf[8];
#endif
