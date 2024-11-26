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

#define DATA_LEN 100
#define RAD_TO_DEG (180.0 / 3.14159265358979323846)

HmcData hmcData;
MPU6050Data mpu6050Data;
double gy86_x;
double gy86_y;
double gy86_z;
// static U32 print_per_time = 1000;

extern float cast_to_range(float val, float begin, float end);

void usart1_send_char(U8 c)
{
    while ((USART_UX->SR & 0X40) == 0); /* 等待上一个字符发送完成 */
    USART_UX->DR = c;
}
void send_controller(U8 *data){
    U8 data_len = data[3];
    // 计算校验和
    U8 sumcheck = 0;
    U8 addcheck = 0;
    for (U8 i = 0; i < (data_len + 4); i++)
    {
        sumcheck += data[i];
        addcheck += sumcheck;
    }
    data[data_len + 4] = sumcheck;
    data[data_len + 5] = addcheck;
    // 发送数据
    for (U8 i = 0; i < (data_len + 6); i++)
    {
        usart1_send_char(data[i]);
    }

}

// 有临界区时会导致串口接收数据任务异常
void send_upper(int16_t Motor[4])
{
    // 发送姿态信息
    U8 data[DATA_LEN] = {0};
    I16 scale = 10000;
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
    send_controller(data);
    // 发送目标角度
    scale = 100;
    data[2] = 0x0A;
    data[3] = 6;
    I16 target_x_scale = target_x * scale * RAD_TO_DEG;
    data[4] = target_x_scale & 0xFF;
    data[5] = (target_x_scale >> 8) & 0xFF;
    I16 target_y_scale = target_y * scale * RAD_TO_DEG;
    data[6] = target_y_scale & 0xFF;
    data[7] = (target_y_scale >> 8) & 0xFF;
    I16 target_z_scale = target_z * scale * RAD_TO_DEG;
    data[8] = target_z_scale & 0xFF;
    data[9] = (target_z_scale >> 8) & 0xFF;
    send_controller(data);
    // 发送遥控器数据
    data[2] = 0x40;
    data[3] = 20;

    // I16 Rol = 1000 + 500;
    I16 Rol = TIM2_Channel1_DataBuf[ROLL_CH] + 500;
    data[4] = Rol & 0xFF;
    data[5] = (Rol >> 8) & 0xFF;

    // I16 Pit = 1200 + 500;
    I16 Pit = TIM2_Channel1_DataBuf[PITCH_CH] + 500;
    data[6] = Pit & 0xFF;
    data[7] = (Pit >> 8) & 0xFF;

    // I16 Thro_local = 900 + 500;
    I16 Thro_local = TIM2_Channel1_DataBuf[THROTTLE_CH] + 500;
    data[8] = Thro_local & 0xFF;
    data[9] = (Thro_local >> 8) & 0xFF;
    
    // I16 Yaw = 800 + 500;
    I16 Yaw = TIM2_Channel1_DataBuf[YAW_CH] + 500;
    data[10] = Yaw & 0xFF;
    data[11] = (Yaw >> 8) & 0xFF;

    for(U8 i=12;i<20+4;i++){
        data[i] = 0;
    }
    send_controller(data);

    // PWM控制量
    scale = 10000;
    I16 rush = 1000;
    data[2] = 0x20;
    data[3] = 8;
    I16 Motor0 = (Motor[0]-1000) * scale / rush;
    data[4] = Motor0 & 0xFF;
    data[5] = (Motor0 >> 8) & 0xFF;
    I16 Motor1 = (Motor[1]-1000) * scale / rush;
    data[6] = Motor1 & 0xFF;
    data[7] = (Motor1 >> 8) & 0xFF;
    I16 Motor2 = (Motor[2]-1000) * scale / rush;
    data[8] = Motor2 & 0xFF;
    data[9] = (Motor2 >> 8) & 0xFF;
    I16 Motor3 = (Motor[3]-1000) * scale / rush;
    data[10] = Motor3 & 0xFF;
    data[11] = (Motor3 >> 8) & 0xFF;
    send_controller(data);
    // 姿态控制量
    data[2] = 0x21;
    data[3] = 8;
    I16 CTRL_ROL = Roll * 10;
    data[4] = CTRL_ROL & 0xFF;
    data[5] = (CTRL_ROL >> 8) & 0xFF;
    I16 CTRL_PIT = Pitch * 10;
    data[6] = CTRL_PIT & 0xFF;
    data[7] = (CTRL_PIT >> 8) & 0xFF;
    I16 CTRL_YAW = Yaw * 10;
    data[8] = CTRL_YAW & 0xFF;
    data[9] = (CTRL_YAW >> 8) & 0xFF;
    I16 CTRL_THR = (Thro-1000) * 10;
    data[10] = CTRL_THR & 0xFF;
    data[11] = (CTRL_THR >> 8) & 0xFF;
    send_controller(data);
    // // 用户自定义帧，发送pid参数信息
    // data[2] = 0x0;
    // data[3] = 8;
    // I16 P_angle = roll_angle_PID.P * 100;
    // data[4] = P_angle & 0xFF;
    // data[5] = (P_angle >> 8) & 0xFF;
    // I16 P_rate = roll_rate_PID.P * 100;
    // data[6] = P_rate & 0xFF;
    // data[7] = (P_rate >> 8) & 0xFF;
    // I16 I_rate = roll_rate_PID.I * 100;
    // data[8] = I_rate & 0xFF;
    // data[9] = (I_rate >> 8) & 0xFF;
    // I16 D_rate = roll_rate_PID.D * 100;
    // data[10] = D_rate & 0xFF;
    // data[11] = (D_rate >> 8) & 0xFF;
    // send_controller(data);

}

// 优先级降低后systemView显示正常
void GY86_task()
{
    // U32 print_rate = 0;
    float partial = 0.5;
    U32 fre = 0;
    int16_t Motor[4];
    while (1)
    {
        if (fre >= 1000000)
        {
            fre = 0;
        }
        // print_rate += 1;
        fre += 1;
        if (fre % 10 == 0)
        {
            Multiple_Read_HMC5883(&(hmcData.x), &(hmcData.y), &(hmcData.z));
            My_ACC_Read_MPU6050(&(mpu6050Data.acc_x), &(mpu6050Data.acc_y), &(mpu6050Data.acc_z));
            My_GYRO_Read_MPU6050(&(mpu6050Data.gyro_x), &(mpu6050Data.gyro_y), &(mpu6050Data.gyro_z));
            MadgwickAHRSupdate(mpu6050Data.gyro_x, mpu6050Data.gyro_y, mpu6050Data.gyro_z, mpu6050Data.acc_x, mpu6050Data.acc_y, mpu6050Data.acc_z,
                               hmcData.y, -hmcData.x, hmcData.z);

            // send_upper();
            // send_upper(Motor);s
            cal_angel(&gy86_x, &gy86_y, &gy86_z);
        }
        // if (print_rate % print_per_time == 0)
        // {
        //     usart_send("gy86_x: %d,gy86_y: %d,gy86_z: %d\n", (int)(gy86_x * RAD_TO_DEG * 10), (int)(gy86_y * RAD_TO_DEG * 10),
        //                (int)(gy86_z * RAD_TO_DEG * 10));
        // }
        if (fre % 10 == 0)
        {
            CtrlAttiAng();
        }
        if (fre % 5 == 0)
        {
            Thro = cast_to_range(target_thro, 1000.0, 2000.0);
            CtrlAttiRate();
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
                // Pitch = 0;
                // Yaw = 0;
            }

            Motor[2] = (int16_t)cast_to_range((Thro + Pitch + Roll - Yaw), 1000, 2000); // M3
            Motor[0] = (int16_t)cast_to_range((Thro - Pitch + Roll + Yaw), 1000, 2000); // M1
            Motor[3] = (int16_t)cast_to_range((Thro + Pitch - Roll + Yaw), 1000, 2000); // M4
            Motor[1] = (int16_t)cast_to_range((Thro - Pitch - Roll - Yaw), 1000, 2000); // M2
            if (gy86_x > 1.5708 || gy86_y > 1.5708 || gy86_x < -1.5708 || gy86_y < -1.5708)
            {
                Motor[0] = 1000;
                Motor[1] = 1000;
                Motor[2] = 1000;
                Motor[3] = 1000;
                pitch_rate_PID.Integ = 0;
                roll_rate_PID.Integ = 0;
            }
            // if (print_rate % print_per_time == 0)
            // {
            //     // usart_send("target_x: %d,target_y: %d,target_z: %d\n",(int)target_x,(int)target_y,(int)target_z);
            //     // usart_send("ch1: %d,ch2: %d,ch3: %d,ch4:
            //     // %d\n",TIM2_Channel1_DataBuf[0]+500,TIM2_Channel1_DataBuf[1]+500,TIM2_Channel1_DataBuf[2]+500,TIM2_Channel1_DataBuf[3]+500);
            //     // usart_send("roll : %d,pitch : %d,yaw : %d,throttle : %d\n", (int)Roll, (int)Pitch, (int)Yaw, (int)Thro);
            //     // usart_send("motor0: %d,motor1: %d,\n motor2: %d,motor3: %d\n", Motor[0], Motor[1], Motor[2], Motor[3]);
            //     // usart_send("roll_rate_itegre: %d,\n pitch_rate_itegre: %d\n", (int16_t)(roll_rate_PID.Integ * 100),
            //             //    (int16_t)(pitch_rate_PID.Integ * 100));
            //     print_rate = 0;
            // }
            MotorPWMSet(Motor[0], Motor[1], Motor[2], Motor[3], 0, 0);
        }
        if(fre % 50 == 0){
            send_upper(Motor);
        }
        OSTimeDly(1 * 1); // 一个tick是1毫秒
    }
}