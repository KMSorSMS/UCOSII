#ifndef __PID_H
#define __PID_H

void cal_angel(float* yaw,float* roll,float* pitch);
// 偏航角（z轴旋转）
double former_x;
// 横滚角（y轴旋转）
double former_y;
// 俯仰角（x轴旋转）
double former_z;

#endif
