#include "Reciever.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/_intsup.h>
#include "Motor.h"
#include "stm32f401xe.h"
#include "usart.h"
#include "NVIC.h"


/**
	use TIM2_CH1 as PPM capture channel
*/
uint16_t TIM2_Channel1_DataBuf[8] = {0};
int start = 0;
uint16_t TIM2_CH1_Cap_Sta = 0;
uint16_t TIM2_CH1_Start_Val = 0;
uint16_t TIM2_CH1_End_Val = 0;
uint32_t TIM2_CH1_Counter = 0;


void PWM_TIM_Init(uint16_t arr,uint16_t psc){
	/**初始化GPIO*/
	RCC->AHB1ENR |= (1<<0);//打开GPIOA时钟
	
	RCC->APB1ENR |= (1<<0);//打开TIM2时钟
	
	
	GPIOA->MODER &= ~(0x03);//将PA0MOIDER位清零
	GPIOA->MODER |= (0x02);//将PA0设置位复用模式
	GPIOA->OSPEEDR |= (0x02);//设置PA0口为高速  
	GPIOA->PUPDR |= (0x02);//引脚设置为下拉，因为要判断高电平的持续时间
	
	GPIOA->AFR[0] &= ~(0xff);//将PA01复用位清零
	GPIOA->AFR[0] |= (0x1<<0);//PA0复用为TIM2通道1
	
	/**TIM2时钟配置*/
	TIM2->CR1 &= ~(0Xf<<1);//UEVenable，更新请求源，关闭单脉冲，向上计数
	TIM2->CR1 |= (0x1<<7);//预装载影子寄存器
	TIM2->SMCR &= ~(0x7);//选择内部时钟 Slave mode disable,这样定时器将不会受到触发事件的影响
	
	TIM2->PSC = psc;//设置分频系数
	TIM2->ARR = arr;//设置重装载寄存器
	
	/**通道一*/
	TIM2->CCMR1 &= ~(0x3<<0);//先清零
	TIM2->CCMR1 |= (0x01);//CC1S设置为输入模式，将CC1映射到TI1
	TIM2->CCMR1 &= ~(0x3<<2);//设置IC1PSC为不分频，只要每检测到一个边沿就触发一次捕获
	TIM2->CCMR1 &= ~(0xf<<4);//设置通道滤波为0000 不做滤波处理，只要采集到边沿就触发
	
	TIM2->CCER |= (0x1<<0);//捕获使能CC1E
	TIM2->CCER &= ~(0x1<<1);//捕获上升沿CC1P
	
	TIM2->DIER |= (0x1<<0);//使能更新中断更新__HAL_TIM_ENABLE_IT
	TIM2->DIER |= (0x1<<1);//使能channel1捕获中断HAL_TIM_IC_Start_IT
	
	
	TIM2->CR1 |= (0x1<<0);//使能CEN计数器
	my_nvic_set_priority(TIM2_IRQn,1,3);
	my_nvic_enable(TIM2_IRQn);
//	HAL_NVIC_SetPriority(TIM2_IRQn,1,3);//设置中断优先级
//	HAL_NVIC_EnableIRQ(TIM2_IRQn);//使能中断

	TIM2->CNT -= 0xffff; 

	
}


void TIM2_IRQHandler(void){
	uint16_t N = 0;
	uint16_t arr = TIM2->ARR;
	static uint8_t start_ = 0;
	static uint8_t in_num =0;
	if(TIM2->SR & (0x01<<0)){//更新事件中断处理,CNt溢出
		//channel_1
		if(TIM2_CH1_Cap_Sta & 0x40){//捕获到高电平了
			if((TIM2_CH1_Cap_Sta & 0x3F) == 0x3F){//高电平时间过长
				TIM2_CH1_Start_Val = 0;//捕获时间舍弃
				TIM2_CH1_Cap_Sta = 0;
				printf("----------------------out ch1-------------- \n\r");
			}else{
				TIM2_CH1_Cap_Sta++;//溢出次数加一
			}
		}
	}
	if(TIM2->SR & (0x1<<1)){//channel_1捕获中断
		if(TIM2_CH1_Cap_Sta & 0x40){//捕获到一个下降沿
			TIM2_CH1_End_Val = TIM2->CCR1;//获取当前CNT的值
			N = TIM2_CH1_Cap_Sta & 0x3f;//溢出的次数
			TIM2_CH1_Counter = N*(arr+1);
			TIM2_CH1_Counter += TIM2_CH1_End_Val;
			TIM2_CH1_Counter -= TIM2_CH1_Start_Val;
			
			if(TIM2_CH1_Counter >= 3500){
				// usart_send("in_num:%d\n",(int)in_num);
				// usart_send("startPPM: %d\n",(int)TIM2_CH1_Counter);
				start_=1;
				in_num = 0;
				start = TIM2_CH1_Counter;
			}
			else if(start_ == 1){
				TIM2_Channel1_DataBuf[in_num++] =  TIM2_CH1_Counter;
				// usart_send("PPM_chal%d: %d\n",in_num-1,(int)TIM2_Channel1_DataBuf[in_num]);
				if(in_num >= 8){
					start_ = 0;
				}
			}
			// usart_send("PPM: %d\n",(int)TIM2_CH1_Counter);
			TIM2_CH1_Cap_Sta = 0;
			TIM2->CCER &= ~(0x1<<1);//捕获上升沿CC1P
			TIM2_CH1_Counter = 0;
		}else{//第一次捕获到上升沿
			TIM2_CH1_End_Val &= ~(0xffff);//将后面记录的CNT值清零
			TIM2_CH1_Cap_Sta = 0x40;//标记捕获到上升沿
			TIM2_CH1_Start_Val = TIM2->CCR1;//记录开始得到CNT
			TIM2->CCER |= (0x1<<1);
		}
	}
	TIM2->SR &= ~(0xffff);//清除中断标志位
}



void print_capture_pwm(uint16_t arr){
	usart_send("start PPM:%d\n",start);
	for(int i = 0;i<8;i++){
		usart_send("TIM2_CH%d cap: %d \n",i,(int)TIM2_Channel1_DataBuf[i]);
	}
	int M1 = PPMtoPWM(TIM2_Channel1_DataBuf[0]);
	int M2 = PPMtoPWM(TIM2_Channel1_DataBuf[0]);
	int M3 = PPMtoPWM(TIM2_Channel1_DataBuf[0]);
	int M4 = PPMtoPWM(TIM2_Channel1_DataBuf[0]);
	int M5 = PPMtoPWM(TIM2_Channel1_DataBuf[0]);
	int M6 = PPMtoPWM(TIM2_Channel1_DataBuf[0]);
	MotorPWMSet(M1,M2,M3,M4,M5,M6);
}

int PPMtoPWM(uint8_t ppm){
	return (int)ppm+500;
}








