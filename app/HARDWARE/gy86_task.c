#include "Global.h"
#include "HMC.h"
#include "MPU6050.h"
#include "os_cpu.h"
#include "ucos_ii.h"
#include "gy86_task.h"
#include "usart.h"

#define data_len 100
HmcData hmcData;
MPU6050Data mpu6050Data;
// static U32 print_per_time = 1;
void usart1_send_char(U8 c)
{
    while ((USART_UX->SR & 0X40) == 0);     /* 等待上一个字符发送完成 */ 
    USART_UX->DR=c;  
} 
// 有临界区时会导致串口接收数据任务异常
void send_upper(){
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
    for(U8 i=0; i < (data[3]+4); i++) 
    { 
        sumcheck += data[i];  
        addcheck += sumcheck;  
    }
    data[13] = sumcheck;
    data[14] = addcheck;
    // 发送数据
    for(U8 i=0; i< (data[3]+6) ; i++)
    {
        usart1_send_char(data[i]);
    }
}
// 优先级降低后systemView显示正常
void GY86_task()
{
    // U32 print_rate = 0;
    while (1)
    {
//         // usart_send("**Start HMC5883L\n");
// #if OS_CRITICAL_METHOD == 3u /* Allocate storage for CPU status register */
//         OS_CPU_SR cpu_sr = 0u;
// #endif
//         OS_ENTER_CRITICAL();
//         if (OSIntNesting < 255u)
//         {
//             OSIntNesting++;
//         }
//         OS_EXIT_CRITICAL();
        Multiple_Read_HMC5883(&(hmcData.x), &(hmcData.y), &(hmcData.z));
        My_ACC_Read_MPU6050(&(mpu6050Data.acc_x), &(mpu6050Data.acc_y), &(mpu6050Data.acc_z));
        My_GYRO_Read_MPU6050(&(mpu6050Data.gyro_x), &(mpu6050Data.gyro_y), &(mpu6050Data.gyro_z));
        MadgwickAHRSupdate(mpu6050Data.gyro_x, mpu6050Data.gyro_y, mpu6050Data.gyro_z, mpu6050Data.acc_x, mpu6050Data.acc_y, mpu6050Data.acc_z,hmcData.y,-hmcData.x,hmcData.z);
        // 打印数据
        // if (print_rate %  print_per_time == 0)
        // {
            // print_rate = 0;
            send_upper();
        // }
        // print_rate++;


        // MadgwickAHRSupdate(mpu6050Data.gyro_x, mpu6050Data.gyro_y, mpu6050Data.gyro_z, 0, 0,0,0,0,0);
        // MadgwickAHRSupdate(0, 0, 0, mpu6050Data.acc_x, mpu6050Data.acc_y, mpu6050Data.acc_z,0,0,0);
        // if (print_rate %  print_per_time == 0)
        // {
            // print_rate = 0;
            // usart_send("hmc.x: %d,hmc.y: %d,hmc.x: %d\n",hmcData.x,hmcData.y,hmcData.z);
            // usart_send("MPU.acc_x: %d,MPU.acc_y: %d,MPU.acc_z: %d\n",(int)mpu6050Data.acc_x,(int)mpu6050Data.acc_y,(int)mpu6050Data.acc_z);
            // usart_send("gyro_x: %d,gyro_y: %d,gyro_z: %d\n",(int)(mpu6050Data.gyro_x*100),(int)(mpu6050Data.gyro_y*100),(int)(mpu6050Data.gyro_z*100));
            // 分别打印整数部分和小数部分（保留5位小数）
            // const float left_nums = 100000;
            // usart_send("q0: %d,q1: %d,q2: %d,q3: %d\n",(int)(q0*left_nums),(int)(q1*left_nums),(int)(q2*left_nums),(int)(q3*left_nums));
            // usart_send("float test %f\n", 1.0f);
        // }
        // print_rate++;
        // usart_send("HMC5883L\n");
        // OS_ENTER_CRITICAL();
        // if (OSIntNesting > 0u)
        // { /* Prevent OSIntNesting from wrapping       */
            // OSIntNesting--;
        // }
        // OS_EXIT_CRITICAL();
        OSTimeDly(15 * 1); // 一个tick是1毫秒
    }
}
