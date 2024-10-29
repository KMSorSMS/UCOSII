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