#ifndef __PID_H
#define __PID_H
// 偏航角（z轴旋转）
extern double target_x;
// 横滚角（y轴旋转）
extern double target_y;
// 俯仰角（x轴旋转）
extern double target_z;

//gy86测量值
extern double gy86_x;
extern double gy86_y;
extern double gy86_z;
extern float Thro,Roll,Pitch,Yaw;

// PID结构体
typedef struct
{
    float P;
    float I;
    float D;
    float Desired;
    float Error;
    float PreError;
    float PrePreError;
    float Increment;
    float Integ;
	float iLimit;
    float Deriv;
    float Output;
}PID_Typedef;

extern PID_Typedef pitch_angle_PID;	//pitch角度环的PID
extern PID_Typedef pitch_rate_PID;		//pitch角速率环的PID

extern PID_Typedef roll_angle_PID;   //roll角度环的PID
extern PID_Typedef roll_rate_PID;    //roll角速率环的PID

extern PID_Typedef yaw_angle_PID;    //yaw角度环的PID 
extern PID_Typedef yaw_rate_PID;     //yaw角速率环的PID

void cal_angel(double* x, double* y, double* z);
// float PID_control(float A, float B, float C, float e_k, float e_k_1, float e_k_2);
void CtrlAttiAng(void);
void CtrlAttiRate(void);

void Init_PID(void);

#endif