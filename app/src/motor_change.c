#include "motor_change.h"
#include "Reciever.h"
#include "Motor.h"

void changeMotorTask(void *args){
	args = args;
	while(1){
		
		print_capture_pwm((uint16_t)1);
		
	}
}