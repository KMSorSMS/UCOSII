#include "MotorPWM.h"

/*
此模块用于控制电机的PWM输出，目前使用的是TIM3的CH1~CH4通道
TIM3挂载在APB1上，在我们f401 AHB为84M下，APB1为42Mhz，但是TIM3的时钟为84Mhz
*/

//初始化GPIO口，TIM3，PWM输出，我们使用GPIOC6~GPIOC9对应M1~M4
void MotorPWMInit(){
    //使能GPIOC的时钟,使用寄存器的方法
    RCC->AHB1ENR |= 1<<2;//使能GPIOC时钟
    //使能TIM3的时钟,在APB1上
    RCC->APB1ENR |= 1<<1;//使能TIM3时钟
    //设置GPIOC6~GPIOC9为复用功能，具体为GPIO_AF2_TIM3
    GPIOC->MODER &= ~(0xffu<<12);//清空GPIOC6~GPIOC9的MODER
    GPIOC->MODER |= 0xaa<<12;//设置GPIOC6~GPIOC9为复用功能
    GPIOC->AFR[0] &= ~(0xffu<<24);//清空GPIOC6,GPIOC7的AFR[0]
    GPIOC->AFR[0] |= 0x22<<24;//设置GPIOC6,GPIOC7的AFR[0]为AF2
    GPIOC->AFR[1] &= ~(0xff<<0);//清空GP IOC8,GPIOC9的AFR[1]
    GPIOC->AFR[1] |= 0x22<<0;//设置GPIOC8,GPIOC9的AFR[1]为AF2
    //设置GPIOC6~GPIOC9的输出速度为高速
    GPIOC->OSPEEDR &= ~(0xff<<12);//清空GPIOC6~GPIOC9的OSPEEDR
    GPIOC->OSPEEDR |= 0xff<<12;//设置GPIOC6~GPIOC9的OSPEEDR为超高速Very High Speed
    //设置GPIOC6~GPIOC9的推挽输出
    GPIOC->OTYPER &= ~(0xf<<6);//清空GPIOC6~GPIOC9的OTYPER
    //设置GPIOC6~GPIOC9无上下拉
    GPIOC->PUPDR &= ~(0xff<<12);//清空GPIOC6~GPIOC9的PUPDR
    //设置TIM3的工作模式为向上计数模式
    TIM3->CR1 &= ~(0x1<<4);//清空TIM3的DIR
    //设置TIM3的预分频系数为(84)-1,则TIM3的时钟为84Mhz/(84) = 1Mhz ,pre是16位的，最大值是65535
    TIM3->PSC = 84-1;
    //设置TIM3的自动重装载值为1000-1,则TIM3的计数频率为1Mhz/5000 = 200Hz,也就是5ms一次中断，周期为5ms
    TIM3->ARR = 5000-1;
    //设置TIM3的CH1~CH4的占空比为0，范围是1000~2000-1,对应1~2ms，占空比的范围控制在20%~40%
    TIM3->CCR1 = 1000;
    TIM3->CCR2 = 1000;
    TIM3->CCR3 = 1000;
    TIM3->CCR4 = 1000;
    //设置TIM3的CH1~CH4的工作模式为PWM模式2，也就是计数值高于CCRx时输出为高电平，否则为低电平
    TIM3->CCMR1 &= ~(0x7<<4);//清空TIM3的CH1的OC1M
    TIM3->CCMR1 |= 0x7<<4;//设置TIM3的CH1的OC1M为PWM模式2
    TIM3->CCMR1 &= ~(0x7<<12);//清空TIM3的CH2的OC2M
    TIM3->CCMR1 |= 0x7<<12;//设置TIM3的CH2的OC2M为PWM模式2
    TIM3->CCMR2 &= ~(0x7<<4);//清空TIM3的CH3的OC3M
    TIM3->CCMR2 |= 0x7<<4;//设置TIM3的CH3的OC3M为PWM模式2
    TIM3->CCMR2 &= ~(0x7<<12);//清空TIM3的CH4的OC4M
    TIM3->CCMR2 |= 0x7<<12;//设置TIM3的CH4的OC4M为PWM模式2
    //设置TIM3的CH1~CH4的通道输出极性为低电平有效，并且设置TIM3的CH1~CH4的输出使能
    TIM3->CCER &= ~(0x3<<0);//清空TIM3的CH1的CC1P和CC1E
    TIM3->CCER |= 0x3<<0;//设置TIM3的CH1的CC1P为低电平有效，CC1E为使能
    TIM3->CCER &= ~(0x3<<4);//清空TIM3的CH2的CC2P和CC2E
    TIM3->CCER |= 0x3<<4;//设置TIM3的CH2的CC2P为低电平有效，CC2E为使能
    TIM3->CCER &= ~(0x3<<8);//清空TIM3的CH3的CC3P和CC3E
    TIM3->CCER |= 0x3<<8;//设置TIM3的CH3的CC3P为低电平有效，CC3E为使能
    TIM3->CCER &= ~(0x3<<12);//清空TIM3的CH4的CC4P和CC4E
    TIM3->CCER |= 0x3<<12;//设置TIM3的CH4的CC4P为低电平有效，CC4E为使能
    //设置TIM3的CH1~CH4的预装载使能
    TIM3->CCMR1 |= 1<<3;//设置TIM3的CH1的OC1PE为使能，预装载的作用是在下一个更新事件时，将TIMx_CCRx的值写入TIMx_CCRx寄存器
    TIM3->CCMR1 |= 1<<11;//设置TIM3的CH2的OC2PE为使能
    TIM3->CCMR2 |= 1<<3;//设置TIM3的CH3的OC3PE为使能
    TIM3->CCMR2 |= 1<<11;//设置TIM3的CH4的OC4PE为使能
    //设置TIM3的ARR的预装载使能
    TIM3->CR1 |= 1<<7;//设置TIM3的ARPE为使能
    //完成初始化,使能TIM3,开启计数
    TIM3->CR1 |= 1<<0;//设置TIM3的CEN为使能
	
//	MotorPWMTest(2000);
//遥控左上下是ch3 motor3下最高
//右左右是ch1 motor1右最高
//右上下ch2 motor2下最高
//左左右 ch4 motor4右最高
}
//下面为调试程序，我们通过usart串口传入的数据来控制电机的转速，调整占空比
  void MotorPWMTest(uint16_t M){
    //M为接收到的数据，范围是0~500
    //设置TIM3的CH1~CH4的占空比为M，范围是0~500-1，其大小代表了占空比的大小
    TIM3->CCR1 = M;
    TIM3->CCR2 = M;
    TIM3->CCR3 = M;
    TIM3->CCR4 = M;
}

void MotorPWMSet(uint16_t M1,uint16_t M2,uint16_t M3,uint16_t M4,uint16_t M5,uint16_t M6){
    //M为接收到的数据，范围是0~500
    //设置TIM3的CH1~CH4的占空比为M，范围是0~500-1，其大小代表了占空比的大小
    TIM3->CCR1 = M1;
    TIM3->CCR2 = M2;
    TIM3->CCR3 = M5;
    TIM3->CCR4 = M6;
}


void startPWM() {
	print_capture_pwm(5000 - 1);
	//查看接收缓冲区是否有数据
	//printf("receiver:%d",TIM4_CH3_Counter);
	if (g_usart_rx_sta & 0x8000) {		//接收到了一帧数据
		printf("接收到数据\n");
		printf("数据长度为：%d\n", g_usart_rx_sta & 0x3fff);
		printf("数据内容为：%s\n", g_usart_rx_buf);
		//从里面得到数据，字符串转为uint16_t，作为pwm的值
		uint16_t pwm = 0;
		//字符串转为uint16_t，不调用库函数：atoi
		for (int i = 0; i < (g_usart_rx_sta & 0x3fff); i++) {
			pwm = pwm * 10 + g_usart_rx_buf[i] - '0';
		}
		printf("pwm is %d\n", pwm);
		// MOTOR_PWM_TEST(pwm);				//设置电机PWM
        MotorPWMTest(pwm);
		//清除接收缓冲区，不使用memset
		for (int i = 0; i < (g_usart_rx_sta & 0x3fff); i++) {
			g_usart_rx_buf[i] = 0;
		}
		g_usart_rx_sta = 0;			//清除接收标记
	}
}
