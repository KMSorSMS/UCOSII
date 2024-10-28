#ifndef MPU6050_H   
#define MPU6050_H

#include "myiic.h"
#include "main.h"

/* MPU6050 Register Address ------------------------------------------------------------*/
#define MPU6050_ADDRESS 0xD0 //IIC写入时的地址字节数据，0x68左移一位，I2C_ReadByte()、I2C_WriteByte()时用
#define MPU6050_Addr_Real 0x68 //i2cread()、i2cwrite()时用这个地址
#define MPU6050_Device_ID 0x68

#define MPU6050_SMPLRT_DIV 0x19 //陀螺仪采样率，典型值：0x07(125Hz)
#define MPU6050_CONFIG 0x1A //低通滤波频率，典型值：0x06(5Hz)
#define MPU6050_GYRO_CONFIG 0x1B //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define MPU6050_ACCEL_CONFIG 0x1C //加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define MPU6050_INT_PIN_CFG 0x37
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_XOUT_L 0x3C
#define MPU6050_ACCEL_YOUT_H 0x3D
#define MPU6050_ACCEL_YOUT_L 0x3E
#define MPU6050_ACCEL_ZOUT_H 0x3F
#define MPU6050_ACCEL_ZOUT_L 0x40
#define MPU6050_TEMP_OUT_H 0x41
#define MPU6050_TEMP_OUT_L 0x42
#define MPU6050_GYRO_XOUT_H 0x43
#define MPU6050_GYRO_XOUT_L 0x44
#define MPU6050_GYRO_YOUT_H 0x45
#define MPU6050_GYRO_YOUT_L 0x46
#define MPU6050_GYRO_ZOUT_H 0x47
#define MPU6050_GYRO_ZOUT_L 0x48
#define MPU6050_USER_CTRL 0x6A
#define MPU6050_PWR_MGMT_1 0x6B //电源管理，典型值：0x00(正常启用)
#define MPU6050_WHO_AM_I 0x75 //IIC地址寄存器(默认数值0x68，只读)
#define GYRO_RANGE 4000
#define ACC_RANGE 4
#define MPU_BUF_SIZE  20
#define MPU_TEST 0xE0


// extern void MPU6050_Bypass(void);
extern void MPU6050_Init(void);
extern void MPU_Self_Test(void);
extern void ACC_Read_MPU6050(float* x,float* y,float* z);
extern void GYRO_Read_MPU6050(float* x,float* y,float* z);
extern void RAW_READ_MPU6050(int16_t* x, int16_t* y, int16_t* z, int16_t readType);
extern void startMPU6050(void);
extern void My_ACC_Read_MPU6050(float* x, float* y, float* z);
extern void My_GYRO_Read_MPU6050(float* x, float* y, float* z);
extern void MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
extern volatile float q0 , q1, q2 , q3 ;
void MPU6050_Read();

#endif
