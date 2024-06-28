#include "HMC.h"
#include "MPU6050.h"
#include "os_cpu.h"
#include "ucos_ii.h"
#include "gy86_task.h"
#include "usart.h"

HmcData hmcData;
MPU6050Data mpu6050Data;

// 有临界区时会导致串口接收数据任务异常
// 优先级降低后systemView显示正常
void GY86_task()
{
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
        // usart_send("hmc.x: %d,hmc.y: %d,hmc.x: %d\n",hmcData.x,hmcData.y,hmcData.z);
        // usart_send("MPU.acc_x: %d,MPU.acc_y: %d,MPU.acc_z: %d\n",(int)mpu6050Data.acc_x,(int)mpu6050Data.acc_y,(int)mpu6050Data.acc_z);
        usart_send("gyro_x: %d,gyro_y: %d,gyro_z: %d\n",(int)mpu6050Data.gyro_x,(int)mpu6050Data.gyro_y,(int)mpu6050Data.gyro_z);
        // usart_send("HMC5883L\n");
        // OS_ENTER_CRITICAL();
        // if (OSIntNesting > 0u)
        // { /* Prevent OSIntNesting from wrapping       */
            // OSIntNesting--;
        // }
        // OS_EXIT_CRITICAL();
        OSTimeDly(1000 * 3); // 延时5s，因为一个tick是1毫秒
    }
}
