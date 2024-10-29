#ifndef __PID_H
#define __PID_H
#include <math.h>

void cal_angel(double* x,double* y,double* z);

// 偏航角（z轴旋转）
extern double former_x;
// 横滚角（y轴旋转）
extern double former_y;
// 俯仰角（x轴旋转）
extern double former_z;
#endif
