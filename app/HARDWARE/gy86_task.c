#include "HMC.h"
#include "MPU6050.h"
#include "gy86_task.h"

HmcData hmcData;
MPU6050Data mpu6050Data; 

void GY86_task(){
    iic_init();
    MPU6050_Init();
    HMC5883L_Init();
    while(1){
        Multiple_Read_HMC5883(&(hmcData.x), &(hmcData.y), &(hmcData.z));
      	My_ACC_Read_MPU6050(&(mpu6050Data.acc_x), &(mpu6050Data.acc_y), &(mpu6050Data.acc_z));
	    My_GYRO_Read_MPU6050(&(mpu6050Data.gyro_x), &(mpu6050Data.gyro_y), &(mpu6050Data.gyro_z));
        OSTimeDly(1000 * 5); // 延时5s，因为一个tick是10微秒
    }
}


















