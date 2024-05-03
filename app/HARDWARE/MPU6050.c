
#include "MPU6050.h"
#include "os_cfg.h"

static float Xtest,Ytest,Ztest,x,y,z;

static void Raw_Read_MPU6050(int16_t* x, int16_t* y, int16_t* z, int16_t readType);

//初始化MPU6050，寄存器的宏定义在MPU6050.h中
void MPU6050_Init(void){
	// //获取PWR_MGMT_1的值
    //唤醒MPU6050
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_PWR_MGMT_1, 0x01);
	/*量程初始化*/
    //陀螺仪传感器,±2000dps
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_GYRO_CONFIG, 0x18);//设置陀螺仪满量程范围为±2000dps
    //加速度传感器,±2g
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_ACCEL_CONFIG, 0x01);//设置加速度传感器满量程范围为±2g
	//进行自测
	MPU_Self_Test();
	/*将自测位清零*/
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_GYRO_CONFIG, 0x18);
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_ACCEL_CONFIG, 0x01);
    //陀螺仪采样率为1K/(1+0x07)=125Hz
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_SMPLRT_DIV, 0x07);
    //低通滤波器的截止频率为1K,带宽为5Hz
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_CONFIG, 0x06);
    //打开旁路模式
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_INT_PIN_CFG, 0x02);
    //关闭IIC主模式
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_USER_CTRL, 0x00);
}

//使能时数据减去失能时数据才是自测应答位(self-test response)
//所以加速度量程的范围为ACC_RANGE,陀螺仪量程的范围为GYRO_RANGE
void MPU_Self_Test(void){
	/*自测GYRO*/
	//读取未使能自测位时的值
	GYRO_Read_MPU6050(&x,&y,&z);
	//使能三个轴的自测位
	Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_GYRO_CONFIG,MPU_TEST|0x18);
	//printf("MPU6050_GYRO_CONFIG:%d\n",Peripheral_ReadByte(MPU6050_ADDRESS,MPU6050_GYRO_CONFIG));
	//停顿
	OSTimeDly(100 * 1000 ); // 延时1ms
	//读取使能自测时的值
	GYRO_Read_MPU6050(&Xtest,&Ytest,&Ztest);

	//读取未使能自测位时的值
	ACC_Read_MPU6050(&x,&y,&z);
	//使能了三个轴的自测位
	Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_ACCEL_CONFIG,MPU_TEST|0x01);
	//加入停顿
	OSTimeDly(100 * 1000 ); // 延时1ms
	//读取使能自测时的值
	ACC_Read_MPU6050(&Xtest,&Ytest,&Ztest);
	//进行硬件检查
}

void ACC_Read_MPU6050(float* x,float* y,float* z){
	int16_t tempX,tempY,tempZ;
    uint8_t BUF[MPU_BUF_SIZE];			//缓冲区
    Peripheral_ReadBytes(MPU6050_ADDRESS,MPU6050_ACCEL_XOUT_H,BUF,6);		//连续读取六个字节
	//读取原始数据
	tempX=(((int16_t)BUF[0])<<8)|((int16_t)BUF[1]);
	tempY=(((int16_t)BUF[2])<<8)|((int16_t)BUF[3]);
	tempZ=(((int16_t)BUF[4])<<8)|((int16_t)BUF[5]);
	//映射成实际值(单位为mg)
	*x=(float)tempX*ACC_RANGE*1000/65536;
	*y=(float)tempY*ACC_RANGE*1000/65536;
	*z=(float)tempZ*ACC_RANGE*1000/65536;
}

void GYRO_Read_MPU6050(float* x,float* y,float* z){
	int16_t tempX,tempY,tempZ;
    uint8_t BUF[MPU_BUF_SIZE];			//缓冲区
    Peripheral_ReadBytes(MPU6050_ADDRESS,MPU6050_GYRO_XOUT_H,BUF,6);		//连续读取六个字节
	//读取原始数据
	tempX=(((int16_t)BUF[0])<<8)|((int16_t)BUF[1]);
	tempY=(((int16_t)BUF[2])<<8)|((int16_t)BUF[3]);
	tempZ=(((int16_t)BUF[4])<<8)|((int16_t)BUF[5]);
	//映射成实际值(单位为度/秒)
	*x=(float)tempX*GYRO_RANGE/65536;
	*y=(float)tempY*GYRO_RANGE/65536;
	*z=(float)tempZ*GYRO_RANGE/65536;
}

void My_ACC_Read_MPU6050(float* x, float* y, float* z) {
	int16_t tempX, tempY, tempZ;
	//Raw_Read_MPU6050(&tempX,&tempY,&tempZ,MPU6050_ACCEL_XOUT_H);
	//映射成实际值(单位为mg)
	Raw_Read_MPU6050(&tempX, &tempY, &tempZ, MPU6050_ACCEL_XOUT_H);
	*x = (float)tempX * ACC_RANGE * 1000 / 65536;
	*y = (float)tempY * ACC_RANGE * 1000 / 65536;
	*z = (float)tempZ * ACC_RANGE * 1000 / 65536;
} 


void My_GYRO_Read_MPU6050(float* x, float* y, float* z) {
	int16_t tempX, tempY, tempZ;
	Raw_Read_MPU6050(&tempX, &tempY, &tempZ, MPU6050_GYRO_XOUT_H);
	//映射成实际值(单位为度/秒)
	*x = (float)tempX * GYRO_RANGE / 65536;
	*y = (float)tempY * GYRO_RANGE / 65536;
	*z = (float)tempZ * GYRO_RANGE / 65536;
}

void Raw_Read_MPU6050(int16_t* x, int16_t* y, int16_t* z, int16_t readType) {
	uint8_t BUF[MPU_BUF_SIZE];			//缓冲区
	Peripheral_ReadBytes(MPU6050_ADDRESS, readType, BUF, 6);		//连续读取六个字节
	//读取原始数据
	*x = (((int16_t)BUF[0]) << 8) | ((int16_t)BUF[1]);
	*y = (((int16_t)BUF[2]) << 8) | ((int16_t)BUF[3]);
	*z = (((int16_t)BUF[4]) << 8) | ((int16_t)BUF[5]);
	//映射成实际值(单位为mg)
}


