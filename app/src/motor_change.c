#include "motor_change.h"
#include <stdint.h>
#include "Reciever.h"
#include "Motor.h"
#include "PID.h"
#include "usart.h"
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
float cast_to_range(float val,float begin,float end);
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
	int print_rate = 0;
	float partial = 0.5;
	while(1){
		Thro = cast_to_range((float) PPMtoPWM(TIM2_Channel1_DataBuf[THROTTLE_CH]),1000.0,2000.0);
		CtrlAttiRate();
		int16_t Motor[4];
		// 打印遥控器的值
		//将输出值融合到四个电机
		if(Thro < 1050){
			Pitch = 0, Roll = 0, Yaw = 0;
		}else{
			Pitch = cast_to_range(Pitch, -(Thro-1000)*partial, (Thro-1000)*partial);
			Roll = cast_to_range(Roll, -(Thro-1000)*partial, (Thro-1000)*partial);
			Yaw = cast_to_range(Yaw, -(Thro-1000)*partial, (Thro-1000)*partial);
			Pitch = 0;
			Yaw = 0;
		}

		Motor[2] = (int16_t)cast_to_range((Thro - Pitch - Roll - Yaw ), 1000, 1800);    //M3  
		Motor[0] = (int16_t)cast_to_range((Thro + Pitch - Roll + Yaw ), 1000, 1800);    //M1
		Motor[3] = (int16_t)cast_to_range((Thro - Pitch + Roll + Yaw ), 1000, 1800);    //M4 
		Motor[1] = (int16_t)cast_to_range((Thro + Pitch + Roll - Yaw ), 1000, 1800);    //M2
		if(gy86_x > 1.2217 || gy86_y > 1.2217 || gy86_x < -1.2217 || gy86_y < -1.2217){
			Motor[0] = 1000;
			Motor[1] = 1000;
			Motor[2] = 1000;
			Motor[3] = 1000;
			pitch_rate_PID.Integ = 0;
			roll_rate_PID.Integ = 0;
		}
		print_rate += 1;
		if(print_rate % 100 == 0) {
			// usart_send("target_x: %d,target_y: %d,target_z: %d\n",(int)target_x,(int)target_y,(int)target_z);
			// usart_send("ch1: %d,ch2: %d,ch3: %d,ch4: %d\n",TIM2_Channel1_DataBuf[0]+500,TIM2_Channel1_DataBuf[1]+500,TIM2_Channel1_DataBuf[2]+500,TIM2_Channel1_DataBuf[3]+500);
			usart_send("roll : %d,pitch : %d,yaw : %d,throttle : %d\n",(int)Roll,(int)Pitch,(int)Yaw,(int)Thro);
			usart_send("motor0: %d,motor1: %d,\n motor2: %d,motor3: %d\n",Motor[0],Motor[1],Motor[2],Motor[3]);
			usart_send("roll_rate_itegre: %d,\n pitch_rate_itegre: %d\n",(int16_t)(roll_rate_PID.Integ*100),(int16_t)(pitch_rate_PID.Integ*100));
			print_rate = 0;
		}
		MotorPWMSet(Motor[0],Motor[1],Motor[2],Motor[3],0,0);
		OSTimeDly(10); // 因为一个tick是1毫秒
	}
}

float cast_to_range(float val,float begin,float end){
	// return ((val-1000) * (end - begin))/ 1000 + begin;
	if(val < begin){
		return begin;
	}else if(val > end){
		return end;
	}else{
		return val;
	}
}