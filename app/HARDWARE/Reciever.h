#pragma once
#include <stdint.h>
#ifndef __RECEIVER_H
#define __RECEIVER_H

#include "main.h"

#define PRECISION 50
#define DUTY_MIDVAL 1500

void PWM_TIM_Init(uint16_t arr, uint16_t psc);
void TIM2_IRQHandler(void);
//	void PWM_TIM4_Init(uint16_t arr,uint16_t psc);
void TIM4_IRQHandler(void);
void print_capture_pwm(uint16_t arr);
uint16_t PPMtoPWM(uint16_t ppm);
extern uint16_t TIM2_Channel1_DataBuf[8];
#endif
