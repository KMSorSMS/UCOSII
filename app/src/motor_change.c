#include "motor_change.h"
#include <stdint.h>
#include "Reciever.h"
#include "Motor.h"
#include "PID.h"
#include "usart.h"
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
uint16_t cast_to_range(uint16_t val,uint16_t begin,uint16_t end);
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

void rate_pid(void *args){
	while(1){
		CtrlAttiRate();
		int16_t Motor[4];
		Thro = cast_to_range(TIM2_Channel1_DataBuf[THRO_CHANNEL],1100,1700);
		// 打印遥控器的值
		usart_send("ch1: %d,ch2: %d,ch3: %d,ch4: %d\n",TIM2_Channel1_DataBuf[0]+500,TIM2_Channel1_DataBuf[1]+500,TIM2_Channel1_DataBuf[2]+500,TIM2_Channel1_DataBuf[3]+500);
		//将输出值融合到四个电机
		Motor[2] = (int16_t)(Thro - Pitch - Roll - Yaw );    //M3  
		Motor[0] = (int16_t)(Thro + Pitch + Roll - Yaw );    //M1
		Motor[3] = (int16_t)(Thro - Pitch + Roll + Yaw );    //M4 
		Motor[1] = (int16_t)(Thro + Pitch - Roll + Yaw );    //M2
		MotorPWMSet(Motor[0],Motor[1],Motor[2],Motor[3],0,0);
		OSTimeDly(10); // 因为一个tick是1毫秒
	}
}

uint16_t cast_to_range(uint16_t val,uint16_t begin,uint16_t end){
	return ((val-1000) * (end - begin))/ 1000 + begin;
}