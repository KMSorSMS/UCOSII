#include "motor_change.h"
#include <stdint.h>
#include "Reciever.h"
#include "Motor.h"
#include "PID.h"
#define THRO_CHANNEL 0
typedef struct axis_val{
	double err_k;
	double err_k_1;
	double err_k_2;
}AXIS_VAR;

typedef struct pid_val{
	AXIS_VAR x_axis;
	AXIS_VAR y_axis;
	AXIS_VAR z_axis;
}PID_VAR;

// void changeMotorTask(void *args){
// 	(void)args;
// 	while(1){
// 		print_capture_pwm((uint16_t)1);
//         OSTimeDly(10 * 3); // 因为一个tick是1毫秒
// 	}
// }

void angle_pid(void *args){

	while(1){
		CtrlAttiAng();
		OSTimeDly(20); // 因为一个tick是1毫秒
	}
}

// 右下为M0，左下为M1，右上M2，左上为M3
void rate_pid(void *args){
	while(1){
		CtrlAttiRate();
		int16_t Motor[4];
		Thro = TIM2_Channel1_DataBuf[THRO_CHANNEL];
		//将输出值融合到四个电机
		Motor[2] = (int16_t)(Thro - Pitch - Roll - Yaw );    //M3  
		Motor[0] = (int16_t)(Thro + Pitch - Roll + Yaw );    //M1
		Motor[3] = (int16_t)(Thro - Pitch + Roll + Yaw );    //M4 
		Motor[1] = (int16_t)(Thro + Pitch + Roll - Yaw );    //M2
		MotorPWMSet(Motor[0],Motor[1],Motor[2],Motor[3],0,0);
		OSTimeDly(10); // 因为一个tick是1毫秒
	}
}