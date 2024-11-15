#include "Global.h"
#include "HMC.h"
#include "MPU6050.h"
#include "Madgwick.h"
#include "Motor.h"
#include "OS_stk.h"
#include "Reciever.h"
#include "os_cpu.h"
#include "ucos_ii.h"
#include "gy86_task.h"
#include "usart.h"
#include "PID.h"

#define data_len 100
#define RAD_TO_DEG (180.0 / 3.14159265358979323846)

HmcData hmcData;
MPU6050Data mpu6050Data;
double gy86_x;
double gy86_y;
double gy86_z;
static U32 print_per_time = 100;
void usart1_send_char(U8 c)
{
    while ((USART_UX->SR & 0X40) == 0); /* 等待上一个字符发送完成 */
    USART_UX->DR = c;
}
// 有临界区时会导致串口接收数据任务异常
void send_upper()
{
    char data[data_len] = {0};
    const static I16 scale = 10000;
    data[0] = 0xAA;
    data[1] = 0xFF;
    data[2] = 0x04;
    data[3] = 9;
    I16 q0_scale = q0 * scale;
    data[4] = q0_scale & 0xFF;
    data[5] = (q0_scale >> 8) & 0xFF;
    I16 q1_scale = q1 * scale;
    data[6] = q1_scale & 0xFF;
    data[7] = (q1_scale >> 8) & 0xFF;
    I16 q2_scale = q2 * scale;
    data[8] = q2_scale & 0xFF;
    data[9] = (q2_scale >> 8) & 0xFF;
    I16 q3_scale = q3 * scale;
    data[10] = q3_scale & 0xFF;
    data[11] = (q3_scale >> 8) & 0xFF;
    data[12] = 0x00;
    // 计算校验和
    U8 sumcheck = 0;
    U8 addcheck = 0;
    for (U8 i = 0; i < (data[3] + 4); i++)
    {
        sumcheck += data[i];
        addcheck += sumcheck;
    }
    data[13] = sumcheck;
    data[14] = addcheck;
    // 发送数据
    for (U8 i = 0; i < (data[3] + 6); i++)
    {
        usart1_send_char(data[i]);
    }
}
float cast_to_range(float val, float begin, float end)
{
    // return ((val-1000) * (end - begin))/ 1000 + begin;
    if (val < begin)
    {
        return begin;
    }
    else if (val > end)
    {
        return end;
    }
    else
    {
        return val;
    }
}
// 优先级降低后systemView显示正常
void GY86_task()
{
    U32 print_rate = 0;
    float partial = 0.5;
    U32 fre = 0;
    while (1)
    {
        if (fre >= 1000000)
        {
            fre = 0;
        }
        print_rate += 1;
        fre += 1;
        Multiple_Read_HMC5883(&(hmcData.x), &(hmcData.y), &(hmcData.z));
        My_ACC_Read_MPU6050(&(mpu6050Data.acc_x), &(mpu6050Data.acc_y), &(mpu6050Data.acc_z));
        My_GYRO_Read_MPU6050(&(mpu6050Data.gyro_x), &(mpu6050Data.gyro_y), &(mpu6050Data.gyro_z));
        MadgwickAHRSupdate(mpu6050Data.gyro_x, mpu6050Data.gyro_y, mpu6050Data.gyro_z, mpu6050Data.acc_x, mpu6050Data.acc_y, mpu6050Data.acc_z,
                           hmcData.y, -hmcData.x, hmcData.z);

        send_upper();
        cal_angel(&gy86_x, &gy86_y, &gy86_z);
        if (print_rate % print_per_time == 0)
        {
            usart_send("gy86_x: %d,gy86_y: %d,gy86_z: %d\n", (int)(gy86_x * RAD_TO_DEG * 10), (int)(gy86_y * RAD_TO_DEG * 10),
                       (int)(gy86_z * RAD_TO_DEG * 10));
        }
        if (fre % 10 == 0)
        {
            CtrlAttiAng();
        }
        if (fre % 5 == 0)
        {
            Thro = cast_to_range((float)PPMtoPWM(TIM2_Channel1_DataBuf[THROTTLE_CH]), 1000.0, 2000.0);
            CtrlAttiRate();
            int16_t Motor[4];
            // 打印遥控器的值
            // 将输出值融合到四个电机
            if (Thro < 1050)
            {
                Pitch = 0, Roll = 0, Yaw = 0;
            }
            else
            {
                Pitch = cast_to_range(Pitch, -(Thro - 1000) * partial, (Thro - 1000) * partial);
                Roll = cast_to_range(Roll, -(Thro - 1000) * partial, (Thro - 1000) * partial);
                Yaw = cast_to_range(Yaw, -(Thro - 1000) * partial, (Thro - 1000) * partial);
                Pitch = 0;
                Yaw = 0;
            }

            Motor[2] = (int16_t)cast_to_range((Thro - Pitch - Roll - Yaw), 1000, 1800); // M3
            Motor[0] = (int16_t)cast_to_range((Thro + Pitch - Roll + Yaw), 1000, 1800); // M1
            Motor[3] = (int16_t)cast_to_range((Thro - Pitch + Roll + Yaw), 1000, 1800); // M4
            Motor[1] = (int16_t)cast_to_range((Thro + Pitch + Roll - Yaw), 1000, 1800); // M2
            if (gy86_x > 1.2217 || gy86_y > 1.2217 || gy86_x < -1.2217 || gy86_y < -1.2217)
            {
                Motor[0] = 1000;
                Motor[1] = 1000;
                Motor[2] = 1000;
                Motor[3] = 1000;
                pitch_rate_PID.Integ = 0;
                roll_rate_PID.Integ = 0;
            }
            if (print_rate % 100 == 0)
            {
                // usart_send("target_x: %d,target_y: %d,target_z: %d\n",(int)target_x,(int)target_y,(int)target_z);
                // usart_send("ch1: %d,ch2: %d,ch3: %d,ch4:
                // %d\n",TIM2_Channel1_DataBuf[0]+500,TIM2_Channel1_DataBuf[1]+500,TIM2_Channel1_DataBuf[2]+500,TIM2_Channel1_DataBuf[3]+500);
                usart_send("roll : %d,pitch : %d,yaw : %d,throttle : %d\n", (int)Roll, (int)Pitch, (int)Yaw, (int)Thro);
                usart_send("motor0: %d,motor1: %d,\n motor2: %d,motor3: %d\n", Motor[0], Motor[1], Motor[2], Motor[3]);
                usart_send("roll_rate_itegre: %d,\n pitch_rate_itegre: %d\n", (int16_t)(roll_rate_PID.Integ * 100),
                           (int16_t)(pitch_rate_PID.Integ * 100));
                print_rate = 0;
            }
            MotorPWMSet(Motor[0], Motor[1], Motor[2], Motor[3], 0, 0);
        }
        OSTimeDly(1 * 1); // 一个tick是1毫秒
    }
}