#include "MS5611.h"
uint16_t ms5611_prom[PROM_NB+1];
void MS5611_Reset(void){
	iic_start();                            //其实信号
    iic_send_byte(MS5611_ADDR);         //发送从机地址
    if(iic_wait_ack() ==1){				    //等待应答
		printf("ack_err\n");
	}
    iic_send_byte(MS5611_RESETCMD);                    //发送重置命令
    if(iic_wait_ack() ==1){				    //等待应答
		printf("ack_err\n");
	}
    iic_stop();                             //停止信号
}

void MS5611_Read_D(uint8_t cmd,uint32_t* D){
	//定义三个变量用于移位
	uint8_t res1,res2,res3;
    iic_start();                          
    iic_send_byte(MS5611_ADDR);         		
    if(iic_wait_ack() ==1){				  
        printf("ack_err\n");
    }
    iic_send_byte(cmd);           
    if(iic_wait_ack() ==1){				  
        printf("ack_err\n");
    }
    iic_stop();                            

    iic_start();                            
    iic_send_byte(MS5611_ADDR);     		
    if(iic_wait_ack() ==1){				    
        printf("ack_err\n");
    }
	iic_send_byte(MS5611_ADC_READ);     		
    if(iic_wait_ack() ==1){				    
        printf("ack_err\n");
    }
	iic_stop(); 
	
	iic_start();                            
    iic_send_byte(MS5611_ADDR+1);     		
    if(iic_wait_ack() ==1){				    
        printf("ack_err\n");
    }
    res1 = iic_read_byte(1);                 
	res2 = iic_read_byte(1);
	res3 = iic_read_byte(0);
    iic_stop();                             
	
	uint16_t temp=(((uint16_t)res1)<<8)|((uint16_t)res2);
	*D =(((uint32_t)temp)<<8)|((uint32_t)res3);
}


//按照手册上顺序即可
void MS5611_Read_PROM_Byte(uint8_t adress,uint16_t* para){
	//定义两个变量用于移位
	uint8_t res1,res2;
    iic_start();                            
    iic_send_byte(MS5611_ADDR);         		
    if(iic_wait_ack() ==1){				    
        printf("ack_err\n");
    }
    iic_send_byte(adress);                 
    if(iic_wait_ack() ==1){				   
        printf("ack_err\n");
    }
    iic_stop();                             

    iic_start();                            
    iic_send_byte(MS5611_ADDR + 1);     		
    if(iic_wait_ack() ==1){				    
        printf("ack_err\n");
    }
    res1 = iic_read_byte(1);                 
	res2 = iic_read_byte(0);
    iic_stop();                           
	
	*para=(((uint16_t)res1)<<8)|((uint16_t)res2);
}

void MS5611_Read_Prom(void)
{
	uint8_t i;
//	uint8_t BUF[2] = {0};
	for(i = 0;i<PROM_NB;i++){
		MS5611_Read_PROM_Byte(CMD_PROM_RD + 2 * i,ms5611_prom + i);
//		Peripheral_ReadBytes(MS5611_ADDR,CMD_PROM_RD+2 *i,BUF,2);
//		ms5611_prom[i] = ((uint16_t)BUF[0] << 8 | (uint16_t)BUF[1]);
//		BUF[0] = 0;
//		BUF[1] = 0;
	}
//	Peripheral_ReadBytes(MS5611_ADDR,CMD_PROM_RD,BUF,PROM_NB*2);
//	for(i=0;i<PROM_NB;i++){
//		ms5611_prom[i] = ((uint16_t)BUF[i*2] << 8 | (uint16_t)BUF[i*2+1]);
//	}
}

float Read_MS5611_TEMP(void){
	uint32_t D1=0,D2=0;
	MS5611_Read_D(MS5611_CD1_4096CMD,&D1);
	MS5611_Read_D(MS5611_CD2_4096CMD,&D2);
	// printf("D1: %#X,D2: %#X \n",D1,D2);//测试用，可以删除
	float temperature = 0,dT = 0,div = 1<<23;
	dT = D2 - (ms5611_prom[5]<<8);
	temperature = 2000 + (dT * (float)ms5611_prom[6])/div;
	printf("Temperature is :  %f °C\n",temperature/100);
	return temperature;
}

float Read_MS5611_PRE(void){
    uint32_t D1=0,D2=0;
    MS5611_Read_D(MS5611_CD1_4096CMD,&D1);
    MS5611_Read_D(MS5611_CD2_4096CMD,&D2);
    // printf("D1: %#X,D2: %#X \n",D1,D2); //测试用，可以删除
    // float temperature = 0;
    float dT = 0,div = 1<<23,mul = 1<<16;
    dT = D2 - (ms5611_prom[5]<<8);
    // temperature = 2000 + (dT * (float)ms5611_prom[6])/div; //计算温度,测试用，可以删除
    // printf("In PRE the Temperature is :  %f °C\n",temperature/100); //测试用，可以删除
    float OFF = 0,SENS = 0;
    div = 1<<7;
    OFF = (float)ms5611_prom[2] * mul + ((float)ms5611_prom[4] * dT) / div;
    mul = 1<<15,div = 1<<8;
    SENS = (float)ms5611_prom[1] * mul + ((float)ms5611_prom[3] * dT) / div;
    float P = 0;
    div = 1<<21;
    float div2 = 1<<15;
    P = ((D1 * SENS / div) - OFF) / div2;
    printf("Pressure is :  %f mbar\n",P/100);
    return P;
}



void MS5611_Init(void){
	MS5611_Reset();
	MS5611_Read_Prom();
	for(int i=0;i<PROM_NB;i++){
		printf("the Prom[%d] is : %#X\n",i,ms5611_prom[i]);
	}
}
