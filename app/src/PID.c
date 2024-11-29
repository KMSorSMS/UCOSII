// #include "os_time.c"
#include "stm32f401xe.h"
#include "MPU6050.h"
#include "PID.h"
#include <math.h>
#include "Reciever.h"
#include "gy86_task.h"
#define DT 1000.0f
float Thro=0,Roll=0,Pitch=0,Yaw=0;
// 我们认为x是roll，y是pitch
enum {ROLL,PITCH,YAW,THROTTLE};
enum {X,Y,Z};


//----PID结构体实例化----
PID_Typedef pitch_angle_PID;	//pitch角度环的PID
PID_Typedef pitch_rate_PID;		//pitch角速率环的PID

PID_Typedef roll_angle_PID;   //roll角度环的PID
PID_Typedef roll_rate_PID;    //roll角速率环的PID

PID_Typedef yaw_angle_PID;    //yaw角度环的PID 
PID_Typedef yaw_rate_PID;     //yaw角速率环的PID

// 偏航角（z轴旋转）弧度制
double target_x;
// 横滚角（y轴旋转）弧度制
double target_y;
// 俯仰角（x轴旋转）弧度制
double target_z;
// 油门
double target_thro;
void threeaxisrot(double r11, double r12, double r21, double r31, double r32, double* x, double* y, double* z)
{
    *x = atan2(r31, r32);
    *y = asin(r21);
    *z = atan2(r11, r12);
}
void cal_angel(double* x, double* y, double* z)
{
    threeaxisrot(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3, -2 * (q1 * q3 - q0 * q2), 2 * (q2 * q3 + q0 * q1),
                 q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3, x, y, z);
}

// // target and init_val is angle or w
// // we use the inceasing PID(增量式PID), because if we use position PID, the sum_err maybe
// // out of bound
// float PID_control(float A, float B, float C, float e_k, float e_k_1, float e_k_2)
// {
//   return A * e_k - B * e_k_1 + C * e_k_2;
// }

//函数名：PID_Postion_Cal()
//描述：位置式PID
static void PID_Postion_Cal(PID_Typedef * PID,float target,float measure,int32_t dertT)
{
	float termI=0;
	float dt= dertT/1000000.0;

	//误差=期望值-测量值
	PID->Error=target-measure;

	PID->Deriv= (PID->Error-PID->PreError)/dt;

    // 没有起飞时将Integ清零
    if(!takeoff_stat){
        PID->Integ=0;
    }

    // Integ在未起飞之前都是0，所以这里不用对这个式子进行修改
	PID->Output=(PID->P * PID->Error) + (PID->I * PID->Integ) + (PID->D * PID->Deriv);    //PID:比例环节+积分环节+微分环节

	PID->PreError=PID->Error;
	//仅用于角度环和角速度环的
    if(fabs(PID->Output) < Thro-1000&&takeoff_stat)		              //比油门还大时不积分（未起飞时也不积分）
    {
        termI=(PID->Integ) + (PID->Error) * dt;     //积分环节
        if(termI > - PID->iLimit && termI < PID->iLimit && PID->Output > - PID->iLimit && PID->Output < PID->iLimit)       //在-300~300时才进行积分环节
                PID->Integ=termI;
    }
}


//函数名：CtrlAttiAng(void)
//描述：对飞行器姿态控制（pitch，roll，yaw）控制中，串级PID中的角度环控制
void CtrlAttiAng(void)
{
	float angTarget[3]={0};
    
    angTarget[ROLL]=(float)(target_x);
    angTarget[PITCH]=(float)(target_y); 

	PID_Postion_Cal(&roll_angle_PID,angTarget[ROLL],gy86_x,DT);	 
	PID_Postion_Cal(&pitch_angle_PID,angTarget[PITCH],gy86_y,DT);
}

//函数名：CtrlAttiRate(void)
//描述：对飞行器姿态控制（pitch，roll，yaw）控制中，串级PID中的角速度环控制
void CtrlAttiRate(void)
{
 	float yawRateTarget=0;

	yawRateTarget=-(float)target_z;  //yawRateTarget是目标角速度，是RAD/s,target_z是目标角速度

	//原参数对应于 DMP的直接输出gyro , 是RAD.
	PID_Postion_Cal(&pitch_rate_PID,pitch_angle_PID.Output,mpu6050Data.gyro_y,DT);	
	PID_Postion_Cal(&roll_rate_PID,roll_angle_PID.Output,mpu6050Data.gyro_x,DT);//gyroxGloble
	PID_Postion_Cal(&yaw_rate_PID,yawRateTarget,mpu6050Data.gyro_z,DT);          //DMP_DATA.GYROz

	Pitch = pitch_rate_PID.Output;
	Roll  = roll_rate_PID.Output;
	Yaw   = yaw_rate_PID.Output;
}

void Init_PID(){
    pitch_angle_PID.P = 8.0;
    pitch_angle_PID.I = 0;//1.0;		//0
    pitch_angle_PID.D = 0;

    pitch_angle_PID.iLimit = 300;	//or 1000

    pitch_rate_PID.P  = 16.5;
    pitch_rate_PID.I  = 0.5; 		//0.5
    pitch_rate_PID.D  = 0.132;

    pitch_rate_PID.iLimit = 300;
////////////////////////////////////////////
    roll_angle_PID.P = 8.0;
    roll_angle_PID.I = 0;//1.0;
    roll_angle_PID.D = 0;
    roll_angle_PID.iLimit = 300;	//or 1000

    roll_rate_PID.P  = 16.5;
    roll_rate_PID.I  = 0.5; 	//3
    roll_rate_PID.D  = 0.132;
    roll_rate_PID.iLimit = 300;
///////////////////////////////////////////
    yaw_angle_PID.P = 0;
    yaw_angle_PID.I = 0;
    yaw_angle_PID.D = 0;

    yaw_rate_PID.P  = 100;
    yaw_rate_PID.I  = 0;
    yaw_rate_PID.D  = 0;
}