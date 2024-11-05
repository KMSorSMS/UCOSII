#include "motor_change.h"
#include "Reciever.h"
#include "Motor.h"
#include "PID.h"

typedef struct axis_val{
	double target_angle;
	double target_speed;
	double err_k;
	double err_k_1;
	double err_k_2;
}AXIS_VAR;

typedef struct pid_val{
	AXIS_VAR x_axis;
	AXIS_VAR y_axis;
	AXIS_VAR z_axis;
}PID_VAR;

void changeMotorTask(void *args){
	(void)args;
	while(1){
		print_capture_pwm((uint16_t)1);
        OSTimeDly(10 * 3); // 因为一个tick是1毫秒
	}
}

void x_axis_changeMotorTask(void *args){
	(void)args;
	float dv = 0;
	float A = 0, B = 0, C = 0;
	while(1){
		// dv = PID_control(0, 0, 0);
		OSTimeDly(1 * 5); // 因为一个tick是1毫秒
	}
}