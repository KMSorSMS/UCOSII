#include "Receiver.h"

//通道一的状态数据
uint16_t TIM2_CH1_Cap_Sta = 0;
uint16_t TIM2_CH1_Start_Val = 0;
uint16_t TIM2_CH1_End_Val = 0;
//channel1的Status和数据
uint16_t TIM2_CH2_Cap_Sta = 0;
uint16_t TIM2_CH2_Start_Val = 0;
uint16_t TIM2_CH2_End_Val = 0;



uint16_t TIM4_CH1_Cap_Sta = 0;
uint16_t TIM4_CH1_Start_Val = 0;
uint16_t TIM4_CH1_End_Val = 0;


uint16_t TIM4_CH2_Cap_Sta = 0;
uint16_t TIM4_CH2_Start_Val = 0;
uint16_t TIM4_CH2_End_Val = 0;

uint16_t TIM4_CH3_Cap_Sta = 0;
uint16_t TIM4_CH3_Start_Val = 0;
uint16_t TIM4_CH3_End_Val = 0;

uint16_t TIM4_CH4_Cap_Sta = 0;
uint16_t TIM4_CH4_Start_Val = 0;
uint16_t TIM4_CH4_End_Val = 0;

uint32_t TIM2_CH1_Counter = 0;
uint32_t TIM2_CH2_Counter = 0;

uint32_t TIM4_CH1_Counter = 0;
uint32_t TIM4_CH2_Counter = 0;
uint32_t TIM4_CH3_Counter = 0;
uint32_t TIM4_CH4_Counter = 0;



void PWM_TIM_Init(uint16_t arr,uint16_t psc){
	/**初始化GPIO*/
	/*
	测试PA0，PA1，PB8，PB9
	PA0 --->TIM2_CH1
	PA1 --->TIM2_CH2
	PB6 --->TIM4_CH1
	PB7 --->TIM4_CH2
	PB8 --->TIM4_CH3
	PB9 --->TIM4_CH4
	
	*/
	RCC->AHB1ENR |= (1<<0);//打开GPIOA时钟
	RCC->AHB1ENR |= (1<<1);//打开GPIOB时钟
	RCC->APB1ENR |= (1<<0);//打开TIM2时钟
	RCC->APB1ENR |= (1<<2);//打开TIM4时钟
	
	GPIOA->MODER &= ~(0x0f);//将PA0、1MOIDER位清零
	GPIOA->MODER |= (0x0a);//将PA0、1设置位复用模式
	GPIOA->OSPEEDR |= (0x0a);//设置四个PA口为高速  
	GPIOA->PUPDR |= (0x0a);//GPIO四个引脚设置为下拉，因为要判断高电平的持续时间
	
	GPIOB->MODER &= ~(0xff<<12);//将PB6,PB7,PB8，PB9MODER位清零
	GPIOB->MODER |= (0xaa<<12);//将PB6,PB7,PB8，PB9设置为复用模式
	GPIOB->OSPEEDR |= (0xaa<<12);//将PB6,PB7,PB8,9为高速
	GPIOB->PUPDR |= (0xaa<<12);//设置为下拉，因为要判断高电平的持续时间
	
	GPIOA->AFR[0] &= ~(0xff);//将PA01复用位清零
	GPIOA->AFR[0] |= (0x1<<0);//PA0复用为TIM2通道1
	GPIOA->AFR[0] |= (0x1<<4);//PA1复用为TIM2通道2

	
	GPIOB->AFR[1] &= ~(0xff);//将PB复用位清零
	GPIOB->AFR[0] &= ~(0xffU<<24);
	GPIOB->AFR[0] |= (0x2<<24);//PB6复用为TIM4通道1
	GPIOB->AFR[0] |= (0x2<<28);//PB7复用为TIM4通道2
	GPIOB->AFR[1] |= (0x1<<1);//PB8复用为TIM4通道3
	GPIOB->AFR[1] |= (0x1<<5);//PB9复用为TIM4通道4
	
	
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
	
	/*通道二*/
	TIM2->CCMR1 &= ~(0x3<<8);
	TIM2->CCMR1 |= (0x01<<8);//CC2S将IC2映射到TI2
	TIM2->CCMR1 &= ~(0x3<<10);//IC1PSC不分频
	TIM2->CCMR1 &= ~(0xf<<12);//IC1F不做滤波处理
	
	TIM2->CCER |= (0x1<<4);//CC2E
	TIM2->CCER &= ~(0x1<<5);//CC2P
	
	TIM2->DIER |= (0x1<<0);//使能更新中断更新__HAL_TIM_ENABLE_IT
	TIM2->DIER |= (0x1<<1);//使能channel1捕获中断HAL_TIM_IC_Start_IT
	TIM2->DIER |= (0x1<<2);//使能channel2捕获中断

	
	TIM2->CR1 |= (0x1<<0);//使能CEN计数器
	my_nvic_set_priority(TIM2_IRQn,1,3);
	my_nvic_enable(TIM2_IRQn);
//	HAL_NVIC_SetPriority(TIM2_IRQn,1,3);//设置中断优先级
//	HAL_NVIC_EnableIRQ(TIM2_IRQn);//使能中断

	 TIM2->CNT -= 0xffff; 
	
	
	/**TIM4时钟配置*/
	TIM4->CR1 &=~(0Xf<<1);//UEVenable，更新请求源，关闭单脉冲，向上计数
	TIM4->CR1 |= (0x1<<7);//预装载影子寄存器
	TIM4->SMCR &= ~(0x7);//选择内部时钟 Slave mode disable,这样定时器将不会受到触发事件的影响
	
	TIM4->PSC = psc;//设置分频系数
	TIM4->ARR = arr;//设置重装载
	/**通道一*/
	TIM4->CCMR1 &= ~(0x3<<0);//先清零
	TIM4->CCMR1 |= (0x01);//CC1S设置为输入模式，将CC1映射到TI1
	TIM4->CCMR1 &= ~(0x3<<2);//设置IC1PSC为不分频，只要每检测到一个边沿就触发一次捕获
	TIM4->CCMR1 &= ~(0xf<<4);//设置通道滤波为0000 不做滤波处理，只要采集到边沿就触发
	
	TIM4->CCER |= (0x1<<0);//捕获使能CC1E
	TIM4->CCER &= ~(0x1<<1);//捕获上升沿CC1P

	/**通道二*/
	TIM4->CCMR1 &= ~(0x3<<8);
	TIM4->CCMR1 |= (0x01<<8);//CC2S将IC2映射到TI2
	TIM4->CCMR1 &= ~(0x3<<10);//IC1PSC不分频
	TIM4->CCMR1 &= ~(0xf<<12);//IC1F不做滤波处理
	
	TIM4->CCER |= (0x1<<4);//CC2E
	TIM4->CCER &= ~(0x1<<5);//CC2P

	/**通道三*/
	TIM4->CCMR2 &= ~(0x3<<0);
	TIM4->CCMR2 |= (0x01<<0);//将IC3映射到TI3
	TIM4->CCMR2 &= ~(0x3<<2);//不分频
	TIM4->CCMR2 &= ~(0xf<<4);//不滤波
	
	TIM4->CCER |= (0x1<<8);//捕获使能
	TIM4->CCER &= ~(0x1<<9);//捕获上升沿
	/**通道四*/
	TIM4->CCMR2 &= ~(0x3<<8);
	TIM4->CCMR2 |= (0x01<<8);//将IC3映射到TI3
	TIM4->CCMR2 &= ~(0x3<<10);//不分频
	TIM4->CCMR2 &= ~(0xf<12);
	
	TIM4->CCER |=(0x1<<12);
	TIM4->CCER &= ~(0x1<<13);
	
	
	TIM4->DIER |= (0x1<<0);//使能更新中断更新__HAL_TIM_ENABLE_IT
	TIM4->DIER |= (0x1<<1);
	TIM4->DIER |= (0x1<<2);
	TIM4->DIER |= (0x1<<3);//使能channel3捕获中断
	TIM4->DIER |= (0x1<<4);//使能channel4捕获中断
	
	TIM4->CR1 |= (0x1<<0);//使能CEN计数器
	my_nvic_set_priority(TIM4_IRQn,1,3);
	my_nvic_enable(TIM4_IRQn);
//	HAL_NVIC_SetPriority(TIM4_IRQn,1,3);//设置中断优先级
//	HAL_NVIC_EnableIRQ(TIM4_IRQn);//使能中断v
	//TIM4->CNT = 0;

}

/** 输入捕获状态TIMx_CHx_Capture_Sta
* [7]: 缺省
* [6]: 0还没有捕获到高电平，1已经捕获到高电平
* [0:5]: 捕获高电平后溢出的次数，最多溢出63次 所以最长的捕获值为63*65525 = 4194303
*        注意：为了通用，我们默认ARR和CCRy都是16位寄存器，对于32位寄存器也按16位来算
*/




void TIM2_IRQHandler(void){
	uint16_t N = 0;
	uint16_t arr = TIM2->ARR;
	//CH1中断处理
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
		//channel_2
		if(TIM2_CH2_Cap_Sta & 0x40){//捕获到高电平了
			if((TIM2_CH2_Cap_Sta & 0x3f) == 0x3f){
				TIM2_CH2_End_Val = 0xffff;
				TIM2_CH2_Cap_Sta = 0;
			}else{
				TIM2_CH2_Cap_Sta++;
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
			TIM2_CH1_Cap_Sta = 0;
			TIM2->CCER &= ~(0x1<<1);//捕获上升沿CC1P
		}else{//第一次捕获到上升沿
			//TIM2_CH1_Cap_Sta &= ~(0x1<<7);
			TIM2_CH1_End_Val &= ~(0xffff);//将后面记录的CNT值清零
			TIM2_CH1_Cap_Sta = 0x40;//标记捕获到上升沿
			TIM2_CH1_Start_Val = TIM2->CCR1;//记录开始得到CNT
			TIM2->CCER |= (0x1<<1);
		}
	}

	if(TIM2->SR & (0x1<<2)){//CH2捕获
		if(TIM2_CH2_Cap_Sta & 0x40){//捕获到一个下降沿
			TIM2_CH2_End_Val = TIM2->CCR2;
			N = TIM2_CH2_Cap_Sta & 0x3f;//溢出的次数
			TIM2_CH2_Counter = N*(arr+1);
			TIM2_CH2_Counter += TIM2_CH2_End_Val;
			TIM2_CH2_Counter -= TIM2_CH2_Start_Val;
			TIM2_CH2_Cap_Sta = 0;
			TIM2->CCER &= ~(0x1<<5);//CC2P上升沿
			
		}else{
			TIM2_CH2_End_Val &= ~(0xffff);//将结束值清零
			TIM2_CH2_Cap_Sta = 0x40;//标记捕获到上升沿
			TIM2_CH2_Start_Val = TIM2->CCR2;
			TIM2->CCER |= (0x1<<5);//cc2p下降沿
		}
	}
	TIM2->SR &= ~(0xffff);//清除中断标志位
}




void TIM4_IRQHandler(void){
	uint16_t arr = TIM4->ARR;
	uint32_t N;
	if(TIM4->SR & (0x01<<0)){//更新事件
		if(TIM4_CH3_Cap_Sta & 0x40){//捕获到高电平了
			if((TIM4_CH3_Cap_Sta & 0x3f) == 0x3f){
				TIM4_CH3_Cap_Sta = 0;//状态归位
				TIM4_CH3_Start_Val = 0;//捕获时间舍弃
			}else{
				TIM4_CH3_Cap_Sta++;
			}
		}
		if(TIM4_CH4_Cap_Sta & 0x40){//捕获到高电平了
			if((TIM4_CH4_Cap_Sta & 0x3f) == 0x3f){
				TIM4_CH4_Cap_Sta = 0;//状态归位
				TIM4_CH4_Start_Val = 0;//捕获时间舍弃
			}else{
				TIM4_CH4_Cap_Sta++;
			}
		}
		if(TIM4_CH1_Cap_Sta & 0x40){//捕获到高电平了
			if((TIM4_CH1_Cap_Sta & 0x3f) == 0x3f){
				TIM4_CH1_Cap_Sta = 0;//状态归位
				TIM4_CH1_Start_Val = 0;//捕获时间舍弃
			}else{
				TIM4_CH1_Cap_Sta++;
			}
		}
		if(TIM4_CH2_Cap_Sta & 0x40){//捕获到高电平了
			if((TIM4_CH2_Cap_Sta & 0x3f) == 0x3f){
				TIM4_CH2_Cap_Sta = 0;//状态归位
				TIM4_CH2_Start_Val = 0;//捕获时间舍弃
			}else{
				TIM4_CH2_Cap_Sta++;
			}
		}
	}
	
	if(TIM4->SR & (0x1<<1)){//CH1捕获
		if(TIM4_CH1_Cap_Sta & 0x40){//捕获到一个下降沿
			TIM4_CH1_End_Val = TIM4->CCR1;
			N = TIM4_CH1_Cap_Sta & 0x3f;//溢出的次数
			TIM4_CH1_Counter = N*(arr+1);
			TIM4_CH1_Counter += TIM4_CH1_End_Val;
			TIM4_CH1_Counter -= TIM4_CH1_Start_Val;
			TIM4_CH1_Cap_Sta = 0;
			TIM4->CCER &= ~(0x1<<1);//CC2P上升沿
		}else{
			TIM4_CH1_End_Val &= ~(0xffff);//将结束值清零
			TIM4_CH1_Cap_Sta = 0x40;//标记捕获到上升沿
			TIM4_CH1_Start_Val = TIM4->CCR1;
			TIM4->CCER |= (0x1<<1);//cc3p下降沿
		}
	}
	if(TIM4->SR & (0x1<<2)){//CH2捕获
		if(TIM4_CH2_Cap_Sta & 0x40){//捕获到一个下降沿
			TIM4_CH2_End_Val = TIM4->CCR2;
			N = TIM4_CH2_Cap_Sta & 0x3f;//溢出的次数
			TIM4_CH2_Counter = N*(arr+1);
			TIM4_CH2_Counter += TIM4_CH2_End_Val;
			TIM4_CH2_Counter -= TIM4_CH2_Start_Val;
			TIM4_CH2_Cap_Sta = 0;
			TIM4->CCER &= ~(0x1<<5);//CC2P上升沿
		}else{
			TIM4_CH2_End_Val &= ~(0xffff);//将结束值清零
			TIM4_CH2_Cap_Sta = 0x40;//标记捕获到上升沿
			TIM4_CH2_Start_Val = TIM4->CCR2;
			TIM4->CCER |= (0x1<<5);//cc2p下降沿
		}
	}

	if(TIM4->SR & (0x1<<3)){//CH3捕获
		if(TIM4_CH3_Cap_Sta & 0x40){//捕获到一个下降沿
			TIM4_CH3_End_Val = TIM4->CCR3;
			N = TIM4_CH3_Cap_Sta & 0x3f;//溢出的次数
			TIM4_CH3_Counter = N*(arr+1);
			TIM4_CH3_Counter += TIM4_CH3_End_Val;
			TIM4_CH3_Counter -= TIM4_CH3_Start_Val;
			TIM4_CH3_Cap_Sta = 0;
			TIM4->CCER &= ~(0x1<<9);//CC3P上升沿
		}else{
			TIM4_CH3_End_Val &= ~(0xffff);//将结束值清零
			TIM4_CH3_Cap_Sta = 0x40;//标记捕获到上升沿
			TIM4_CH3_Start_Val = TIM4->CCR3;
			TIM4->CCER |= (0x1<<9);//cc3p下降沿
		}
	}
	if(TIM4->SR & (0x1<<4)){//CH4捕获
		if(TIM4_CH4_Cap_Sta & 0x40){//捕获到一个下降沿
			TIM4_CH4_End_Val = TIM4->CCR4;
			N = TIM4_CH4_Cap_Sta & 0x3f;//溢出的次数
			TIM4_CH4_Counter = N*(arr+1);
			TIM4_CH4_Counter += TIM4_CH4_End_Val;
			TIM4_CH4_Counter -= TIM4_CH4_Start_Val;
			TIM4_CH4_Cap_Sta = 0;
			TIM4->CCER &= ~(0x1<<13);//CC4P上升沿
		}else{//捕获到上升沿
			TIM4_CH4_End_Val &= ~(0xffff);//将结束值清零
			TIM4_CH4_Cap_Sta = 0x40;//标记捕获到上升沿
			TIM4_CH4_Start_Val = TIM4->CCR4;//获取CNT值
			TIM4->CCER |= (0x1<<13);//cc4p下降沿
		}
	}
	TIM4->SR &= ~(0xffff);
}



void print_capture_pwm(uint16_t arr){
	// printf("TIM2_CH1_CAP:%d\r\n",TIM2_CH1_Counter);
	// printf("TIM2_CH2_CAP:%d\r\n",TIM2_CH2_Counter);
	// printf("TIM4_CH1_CAP:%d\r\n",TIM4_CH1_Counter);
	// printf("TIM4_CH2_CAP:%d\r\n",TIM4_CH2_Counter);
	// printf("TIM4_CH3_CAP:%d\r\n",TIM4_CH3_Counter);
	// printf("TIM4_CH4_CAP:%d\r\n",TIM4_CH4_Counter);
	// printf("\r\n");
	// MotorPWMSet(TIM2_CH1_Counter,TIM2_CH2_Counter,TIM4_CH3_Counter,TIM4_CH4_Counter,TIM4_CH1_Counter,TIM4_CH2_Counter);
	//进行初步的融合数据：将TIM4_CH3作为油门thrust,TIM4_CH4作为偏航yaw,TIM2_CH1作为横滚roll,TIM2_CH2作为俯仰pitch
	/*
		M1 = 0 - pitch + yaw + thrust
		M2 = roll + 0 - yaw + thrust
		M3 = 0 + pitch + yaw + thrust
		M4 = - roll + 0 - yaw + thrust
	*/
	int M1 = 				0-TIM2_CH2_Counter+TIM4_CH4_Counter+TIM4_CH3_Counter;
	int M2 = TIM2_CH1_Counter+           	 0-TIM4_CH4_Counter+TIM4_CH3_Counter;
	int M3 = 				0+TIM2_CH2_Counter-TIM4_CH4_Counter+TIM4_CH3_Counter;
	int M4 = -TIM2_CH1_Counter+           	 0+TIM4_CH4_Counter+TIM4_CH3_Counter;
	// MOTOR_PWM_SET(M1,M2,TIM4_CH1_Counter,TIM4_CH2_Counter,M3,M4);
	MotorPWMSet(M1,M2,TIM4_CH1_Counter,TIM4_CH2_Counter,M3,M4);
	// MotorPWMSet(TIM2_CH1_Counter,TIM2_CH2_Counter,TIM4_CH1_Counter,TIM4_CH2_Counter,TIM4_CH3_Counter,TIM4_CH4_Counter);
	TIM2_CH1_Counter = 0;
	TIM2_CH2_Counter = 0;
	TIM4_CH1_Counter = 0;
	TIM4_CH2_Counter = 0;
	TIM4_CH3_Counter = 0;
	TIM4_CH4_Counter = 0;
}








