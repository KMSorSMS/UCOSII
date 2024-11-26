#pragma once
#include <stdint.h>
#ifndef __RECEIVER_H
#define __RECEIVER_H

#include "main.h"

#define PRECISION 50
#define DUTY_MIDVAL 1500
#define RANGE 1000

// RUSH CH3
// YAW CH4
// PITCH CH2
// ROLL CH1
#define THROTTLE_CH 2
#define YAW_CH 3
#define PITCH_CH 1
#define ROLL_CH 0

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 3

#define APP_YAW_DB	 70 //dead band 
#define APP_PR_DB		 50

#define ANGLE_MAX 15.0f

#define YAW_RATE_MAX 60

double dbScaleLinear(double x, double x_end, double deadband);
void PWM_TIM_Init(uint16_t arr, uint16_t psc);
void TIM2_IRQHandler(void);
//	void PWM_TIM4_Init(uint16_t arr,uint16_t psc);
void TIM4_IRQHandler(void);
// void print_capture_pwm(uint16_t arr);
uint16_t PPMtoPWM(uint16_t ppm);
extern uint16_t TIM2_Channel1_DataBuf[8];
void update_target(void);

#endif
