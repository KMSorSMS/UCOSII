#include "MPU6050.h"
#include <stdio.h>
//初始化MPU6050，寄存器的宏定义在MPU6050.h中
void MPU6050_Init(void){
	// //获取PWR_MGMT_1的值
	printf("MPU6050_PWR_MGMT1:%#X\r\n",Peripheral_ReadByte(MPU6050_ADDRESS,MPU6050_PWR_MGMT_1));
    //唤醒MPU6050
    Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_PWR_MGMT_1, 0x01);
	// //获取PWR_MGMT_1的值
	printf("MPU6050_PWR_MGMT1:%#X\r\n",Peripheral_ReadByte(MPU6050_ADDRESS,MPU6050_PWR_MGMT_1));
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
	// //获取PWR_MGMT_1的值
	// printf("MPU6050_SMPLRT_DIV:%#X\r\n",Peripheral_ReadByte(MPU6050_ADDRESS,MPU6050_SMPLRT_DIV));
    // //打印MPU6050的ID
    // printf("MPU6050_WHO_AM_I:%#X\r\n",Peripheral_ReadByte(MPU6050_ADDRESS,MPU6050_WHO_AM_I));
}

//使能时数据减去失能时数据才是自测应答位(self-test response)
//所以加速度量程的范围为ACC_RANGE,陀螺仪量程的范围为GYRO_RANGE
void MPU_Self_Test(void){
	float Xtest,Ytest,Ztest,x,y,z;
	/*自测GYRO*/
	//读取未使能自测位时的值
	GYRO_Read_MPU6050(&x,&y,&z);
	//使能三个轴的自测位
	Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_GYRO_CONFIG,MPU_TEST|0x18);
	//printf("MPU6050_GYRO_CONFIG:%d\n",Peripheral_ReadByte(MPU6050_ADDRESS,MPU6050_GYRO_CONFIG));
	//停顿
	HAL_Delay(1000*100);
	//读取使能自测时的值
	GYRO_Read_MPU6050(&Xtest,&Ytest,&Ztest);
//	printf("Xtest:%f\n",Xtest);
//	printf("Ytest:%f\n",Ytest);
//	printf("Ztest:%f\n",Ztest);
//	printf("x:%f\n",x);
//	printf("y:%f\n",y);
//	printf("z:%f\n",z);
	//进行硬件检查
	if(Xtest-x>=10&&Xtest-x<=105){
		printf("MPU6050_GYRO_X_PASS\r\n");
	}
	else{
		printf("MPU6050_GYRO_X_NO_PASS\r\n");
	}
	if(Ytest-y>=-105&&Ytest-y<=-10){
		printf("MPU6050_GYRO_Y_PASS\r\n");
	}
	else{
		printf("MPU6050_GYRO_Y_NO_PASS\r\n");
	}
	
	if(Ztest-z>=10&&Ztest-z<=105){
		printf("MPU6050_GYRO_Z_PASS\r\n");
	}
	else{
		printf("MPU6050_GYRO_Z_NO_PASS\r\n");
	}
	/*自测ACC*/
	//读取未使能自测位时的值
	ACC_Read_MPU6050(&x,&y,&z);
	//使能了三个轴的自测位
	Peripheral_SendByte(MPU6050_ADDRESS,MPU6050_ACCEL_CONFIG,MPU_TEST|0x01);
	//加入停顿
	HAL_Delay(1000*100);
	//读取使能自测时的值
	ACC_Read_MPU6050(&Xtest,&Ytest,&Ztest);
	//进行硬件检查
//	printf("Xtest:%f\n",Xtest);
//	printf("Ytest:%f\n",Ytest);
//	printf("Ztest:%f\n",Ztest);
//	printf("x:%f\n",x);
//	printf("y:%f\n",y);
//	printf("z:%f\n",z);
	if(Xtest-x>=300&&Xtest-x<=950){
		printf("MPU6050_ACC_X_PASS\r\n");
	}
	else{
		printf("MPU6050_ACC_X_NO_PASS\r\n");
	}
	if(Ytest-y>=300&&Ytest-y<=950){
		printf("MPU6050_ACC_Y_PASS\r\n");
	}
	else{
		printf("MPU6050_ACC_Y_NO_PASS\r\n");
	}
	
	if(Ztest-z>=300&&Ztest-z<=950){
		printf("MPU6050_ACC_Z_PASS\r\n");
	}
	else{
		printf("MPU6050_ACC_Z_NO_PASS\r\n");
	}
	
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
	Raw_Read_MPU6050(&tempX,&tempY,&tempZ,MPU6050_ACCEL_XOUT_H);
	//映射成实际值(单位为mg)
	// RAW_READ_MPU6050(&tempX, &tempY, &tempZ, MPU6050_ACCEL_XOUT_H);
	*x = (float)tempX * ACC_RANGE * 1000 / 65536;
	*y = (float)tempY * ACC_RANGE * 1000 / 65536;
	*z = (float)tempZ * ACC_RANGE * 1000 / 65536;
}


void My_GYRO_Read_MPU6050(float* x, float* y, float* z) {
	int16_t tempX, tempY, tempZ;
	// RAW_READ_MPU6050(&tempX, &tempY, &tempZ, MPU6050_GYRO_XOUT_H);
	Raw_Read_MPU6050(&tempX, &tempY, &tempZ, MPU6050_GYRO_XOUT_H);

	//映射成实际值(单位为度/秒)
	*x = (float)tempX * GYRO_RANGE / 65536;
	*y = (float)tempY * GYRO_RANGE / 65536;
	*z = (float)tempZ * GYRO_RANGE / 65536;
}

int printFreq = 0;

void startMPU6050() {
	int16_t x = 0, y = 0, z = 0;
	Multiple_Read_HMC5883(&x, &y, &z);                      //连续读取三轴角度数据，存储在BUF中 
	// MULTIPLE_READ_HMC5883(&x, &y, &z);
	// printf("HMC5883L x:%d y:%d z:%d \r\n", x, y, z);
	uint32_t D1 = 0, D2 = 0;
	MS5611_Read_D(MS5611_CD1_4096CMD,&D1);
	MS5611_Read_D(MS5611_CD2_4096CMD,&D2);
	// MY_MS5611_READ_D(MS5611_CD1_4096CMD, &D1);
	// MY_MS5611_READ_D(MS5611_CD2_4096CMD, &D2);
	// printf("MS5611: D1: %#X,D2: %#X \n", D1, D2);
	float temperature = 0, dT = 0, div = 0x800000;
	dT = D2 - (ms5611_prom[5] << 8);
	temperature = 2000 + (dT * (float)ms5611_prom[6]) / div;
	// printf("HS5611 : Temperature is :  %f C\n", temperature / 100);
	//		Read_MS5611_TEMP();
	//		Read_MS5611_PRE();
	dT = 0, div = 1 << 23;
	float mul = 1 << 16;
	dT = D2 - (ms5611_prom[5] << 8);
	// temperature = 2000 + (dT * (float)ms5611_prom[6])/div; //计算温度,测试用，可以删除
	// printf("In PRE the Temperature is :  %f °C\n",temperature/100); //测试用，可以删除
	float OFF = 0, SENS = 0;
	div = 1 << 7;
	OFF = (float)ms5611_prom[2] * mul + ((float)ms5611_prom[4] * dT) / div;
	mul = 1 << 15, div = 1 << 8;
	SENS = (float)ms5611_prom[1] * mul + ((float)ms5611_prom[3] * dT) / div;
	float P = 0;
	div = 1 << 21;
	float div2 = 1 << 15;
	P = ((D1 * SENS / div) - OFF) / div2;
	// printf("HS5611 : Pressure is :  %f mbar\n", P / 100);
	float acc_x = 0, acc_y = 0, acc_z = 0;
	float gyro_x = 0, gyro_y = 0, gyro_z = 0;
	//ACC_Read_MPU6050(&acc_x,&acc_y,&acc_z);
	My_ACC_Read_MPU6050(&acc_x, &acc_y, &acc_z);
	// printf("acc_x:%f\tacc_y:%f\tacc_z:%f\r\n", acc_x, acc_y, acc_z);
	//GYRO_Read_MPU6050(&gyro_x,&gyro_y,&gyro_z);
	My_GYRO_Read_MPU6050(&gyro_x, &gyro_y, &gyro_z);
	//保持一定频率打印数据
	if(printFreq<5){
		printFreq++;
		return;
	}
	//由于printf这里是gcc工具链，不支持%f，所以需要使用下面的函数：float to string来转化
	printf("HMC5883L x:%d y:%d z:%d \r\n", x, y, z);
	char buffer[10];
	printf("HS5611 : Temperature is :  %s C\r\n", _float_to_char(temperature / 100, buffer, 10));
	printf("HS5611 : Pressure is :  %s mbar\r\n", _float_to_char(P / 100, buffer, 10));
	printf("acc_x:%s\tacc_y:%s\tacc_z:%s\r\n", _float_to_char(acc_x, buffer, 10), _float_to_char(acc_y, buffer, 10), _float_to_char(acc_z, buffer, 10));
	HAL_Delay(1000 * 100);
	printf("gyro_x:%s\tgyro_y:%s\tgyro_z:%s\r\n", _float_to_char(gyro_x, buffer, 10), _float_to_char(gyro_y, buffer, 10), _float_to_char(gyro_z, buffer, 10));
	printf("\r\n");

	printFreq = 0;
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

