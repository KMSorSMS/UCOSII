#pragma once
#ifndef MOTORPWM_H
#define MOTORPWM_H
    #include "main.h"
    extern void MotorPWMInit(void);
    extern void MotorPWMTest(uint16_t M);
    extern void MotorPWMSet(uint16_t M1,uint16_t M2,uint16_t M3,uint16_t M4,uint16_t M5,uint16_t M6);
	extern void startPWM(void);
	extern void MOTOR_PWM_TEST(uint16_t M);
	extern void MOTOR_PWM_SET(uint16_t M1,uint16_t M2,uint16_t M3,uint16_t M4,uint16_t M5,uint16_t M6);
#endif
