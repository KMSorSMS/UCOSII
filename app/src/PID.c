#include "os_time.c"
#include "stm32f401xe.h"
#include "MPU6050.h"
#include "PID.h"

// 偏航角（z轴旋转）
double former_x;
// 横滚角（y轴旋转）
double former_y;
// 俯仰角（x轴旋转）
double former_z;
void threeaxisrot(double r11, double r12, double r21, double r31, double r32, double* x, double* y, double* z)
{
    *x = atan2(r31, r32);
    *y = -asin(r21);
    *z = -atan2(r11, r12);
}
void cal_angel(double* x, double* y, double* z)
{
    threeaxisrot(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3, -2 * (q1 * q3 - q0 * q2), 2 * (q2 * q3 + q0 * q1),
                 q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3, x, y, z);
}

// target and init_val is angle or w
// we use the inceasing PID(增量式PID), because if we use position PID, the sum_err maybe 
// out of bound
void PID_control(double target,double init_val){
  // the delta t
  double dt=0.1;
  // const val of PID
  float Kp=0,Ki=0,Kd=0;
  // the rate of the increasing(or decreasing) of init_val
  double v=0;
  // the delta v in increasing PID
  double dv=0;
  // init the err
  double errk=target-init_val;
  double errk_2=0,errk_1=0;
  while(1){
    // cal the v. We can use the val to control the motor
    dv=Kp*(errk-errk_1)+Ki*errk+Kd*(errk-2*errk_1+errk_2);
    // update the v
    v+=dv;
    // update the init_val, we should use cal_angel func when we debug the PID
    init_val+=v*dt;
    // update the err
    errk_2=errk_1;
    errk_1=errk;
    errk=target-init_val;
    // the frequency of the loop. We set it to 0.1s
    OSTimeDly(100);
  }
}