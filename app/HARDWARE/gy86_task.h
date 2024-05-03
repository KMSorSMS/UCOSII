#ifndef GY86_TASK
#define GY86_TASK

#include <stdint.h>
#include "main.h"

typedef struct MPU6050Data{
	float acc_x; 
    float acc_y;
    float acc_z;
	float gyro_x;
    float gyro_y;
    float gyro_z;
} MPU6050Data; // 定义存储加速度和陀螺仪数据的结构体的单元

typedef struct HmcData{
    int16_t x;
    int16_t y;
    int16_t z;
}HmcData; // 定义存储地磁数据的结构体单元

extern HmcData hmcData;
extern MPU6050Data mpu6050Data; 
extern void GY86_task();

#endif