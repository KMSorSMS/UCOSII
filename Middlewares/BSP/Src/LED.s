RCC_BASE EQU 0x40023800;配置RCC寄存器,时钟
RCC_CR EQU RCC_BASE+0x00;配置RCC控制寄存器
RCC_PLLCFGR EQU RCC_BASE+0x04;配置RCC PLL配置寄存器
RCC_CFGR EQU RCC_BASE+0x08;配置RCC时钟配置寄存器,这是用来选择系统时钟的

RCC_AHB1ENR EQU RCC_BASE+0x30;配置RCC使能寄存器
RCC_AHB1LPENR EQU RCC_BASE+0x50;配置RCC低功耗使能寄存器

GPIOA_BASE EQU 0x40020000;定义GPIOA的基地址
GPIOA_OTYPER EQU GPIOA_BASE+0x04;定义GPIOA的输出类型寄存器的值
GPIOA_OSPEEDR EQU GPIOA_BASE+0x08;定义GPIOA的输出速度寄存器的值
GPIOA_PUPDR EQU GPIOA_BASE+0x0C;定义GPIOA的上下拉寄存器的值
GPIOA_ODR EQU GPIOA_BASE+0x14;定义GPIOA的输出数据寄存器的值

STK_BASE EQU 0xE000E010;定义SysTick的基地址
STK_CTRL EQU STK_BASE+0x00;定义SysTick的控制寄存器
STK_LOAD EQU STK_BASE+0x04;定义SysTick的重装载寄存器
STK_VAL  EQU STK_BASE+0x08;定义SysTick的当前值寄存器
	
FLASH_ACR_LATENCY EQU  0x40023C00
				
				
				PRESERVE8
                THUMB

                AREA	COMMON_DATA, CODE, READWRITE
DELAY_NTIME		DCD		0X000000000
				EXPORT	DELAY_NTIME

                    
               
                    
                
                
                AREA    |.text|, CODE, READONLY,ALIGN=3;开始代码段    


					
;;时钟中断处理函数
;SysTick_Handler PROC
;                EXPORT SysTick_Handler              
;                PUSH {R0,R1,LR}
;                LDR R0,=DELAY_NTIME
;                LDR R1,[R0]
;                SUB R1,R1,#1
;                STR R1,[R0]
;                POP {R0,R1,PC}
;                ENDP
					
;配置为外部时钟源，即是HSE，8MHz，配置PLL倍频系数为8，即是PLLM=8，PLLN=336，PLLP=4，计算公式为：VCO=HSE(我选择的是外部时钟源,8MHz)*PLLN/PLLM=8*336/8=336MHz，SYSCLK=VCO/PLLP=336/4=84MHz
;并且设置AHB时钟预分频系数为1(HPRE)，HCLK=SYSCLK/HPRE=84MHz/1=84MHz，配置APB1时钟预分频系数为2，PCLK1=HCLK/APB1=84MHz/2=42MHz，配置APB2时钟预分频系数为1，PCLK2=HCLK/APB2=84MHz/1=84MHz
Clock_Init		PROC;时钟初始化
				EXPORT Clock_Init
                PUSH {R0,R1,LR};R0,R1,LR中的值放入堆栈
				
				;把外部时钟的PLL关掉才能修改倍频,先更换系统时钟为HSI
				LDR R0,=RCC_CFGR
				LDR R1,[R0]
				BIC R1,#0x3
				STR R1,[R0]
				
                LDR R0,=RCC_CR;这里的地址是RCC_CR的地址
                LDR R1,[R0];将RCC_CR的值装载到R1中
				BIC R1,#0x3<<24;关闭PLL
				STR R1,[R0];

				
                ;在使能PLL之前，必须先设置PLL倍频系数
                ;设置PLL倍频系数为8，即是PLLM=8，PLLN=336，PLLP=4，计算公式为：VCO=HSE(我选择的是外部时钟源,8MHz)*PLLN/PLLM=8*336/8=336MHz，SYSCLK=VCO/PLLP=336/4=84MHz
                LDR R0,=RCC_PLLCFGR;这里的地址是RCC_PLLCFGR的地址
                LDR R1,[R0];将RCC_PLLCFGR的值装载到R1中
                ;我们设置PLLSRC在第22位，PLLM在第5-0位，PLLN在第14-6位，PLLP在第17-16位，PLLSRC=1 PLLP=01，即是4分频，PLLM=1000,即是8分频，PLLN=101010000,即是336倍频
                ;首先将这些位都清零，这些位都置为1作为BIC的第三个操作数，为：00000000010000110111111111111111，16进制为：0x0043 7FFF
                ;BIC R1,R1,#0x437FFF;将RCC_PLLCFGR的第5-0位，第14-6位，第17-16位清零,这样写不对，立即数只能由8位数字左移或者右移得到，这里的这个立即数没办法这样得到，得拆开
                BIC R1,R1,#0xFF
                BIC R1,R1,#0x7F00
                BIC R1,R1,#0x430000
				BIC R1,R1,#0xF<<28
                ;ORR R1,R1,#0x00415408;将RCC_PLLCFGR的第5-0位，第14-6位，第17-16位设置为我们想要的值:00000000010000010101010000001000
                ORR R1,R1,#0x8
                ORR R1,R1,#0x5400
                ORR R1,R1,#0x410000
				STR R1,[R0];将修改后的值写回到地址RCC_PLLCFGR
                
                ;接着设置AHB时钟预分频系数为1(HPRE)，HCLK=SYSCLK/HPRE=84MHz/1=84MHz，配置APB1时钟预分频系数为2，PCLK1=HCLK/APB1=84MHz/2=42MHz，配置APB2时钟预分频系数为1，PCLK2=HCLK/APB2=84MHz/1=84MHz
                ;并设置sw为PLL，即是选择PLL作为系统时钟
                LDR R0,=RCC_CFGR;这里的地址是RCC_CFGR的地址
                LDR R1,[R0];将RCC_CFGR的值装载到R1中
                ;BIC R1,R1,#0x0000FCF3;将RCC_CFGR的第1-0位，第7-4位，第15-10位清零
                BIC R1,R1,#0xF3
                BIC R1,R1,#0xFC00
                ;ORR R1,R1,#0x00001002;将RCC_CFGR的第1-0位，第7-4位，第15-10位
                ORR R1,R1,#0x2
                ORR R1,R1,#0x1000
                STR R1,[R0];将修改后的值写回到地址RCC_CFGR

				;将ACR寄存器置为5
				LDR R0,=FLASH_ACR_LATENCY
				MOV R1,#5
				STR R1,[R0];
				
				;最后使能外部时钟源，使能PLL
                LDR R0,=RCC_CR;这里的地址是RCC_CR的地址
                LDR R1,[R0];将RCC_CR的值装载到R1中
                ORR R1,R1,#0x10000;将RCC_CR的第16位设置为1,即是外部时钟源，HSE
                ORR R1,R1,#0x1000000;将RCC_CR的第24位设置为1,即是使能PLL
                STR R1,[R0];将修改后的值写回到地址RCC_CR
				
				POP {R0,R1,PC}
				ENDP



LED_Init		PROC;LED初始化
				EXPORT LED_Init
					
                PUSH {R0,R1,LR};R0,R1,LR中的值放入堆栈
                ;使能GPIOA的时钟
                LDR R0,=RCC_AHB1ENR      ;LDR是把地址装载到寄存器中(比如R0)。
                LDR R1, [R0]          ;这里把AHB1地址中的内容装载到R1中
                ORR R1, R1, #1        ; 将寄存器R1的第1位设置为1
                STR R1, [R0]          ;将修改后的值写回到地址0x40023800+0x50
                ;设置GPIOA的时钟为高功耗模式，也就是睡眠模式下GPIOA的时钟仍然是开启的
                ;LDR R0,=RCC_AHB1LPENR
                ;LDR R1, [R0]
                ;ORR R1, R1, #1
                ;STR R1, [R0]
                ;设置GPIOA的时钟为低功耗模式，也就是睡眠模式下GPIOA的时钟是关闭的
                LDR R0,=RCC_AHB1LPENR
                LDR R1, [R0]
                BIC R1, R1, #1
                STR R1, [R0]
                ;设置GPIOA pin5的为输出模式,操作寄存器MODER
                LDR R0,=GPIOA_BASE;这里的地址是GPIOA的基地址，同时也是MODER寄存器的地址（因为它的偏移量为0）
                LDR R1,[R0]    ;将GPIOA的MODER寄存器的值装载到R1中
                BIC R1,R1,#0xC00;将MODER寄存器的第10、11位清零
                ORR R1,R1,#0x400;将MODER寄存器的第10位设置为1,即是输出模式
                STR R1,[R0]    ;将修改后的值写回到地址GPIOA_BASE
;				;校验
;				LDR R0,=GPIOA_BASE;这里的地址是GPIOA的基地址，同时也是MODER寄存器的地址（因为它的偏移量为0）
;                LDR R1,[R0]    ;将GPIOA的MODER寄存器的值装载到R1中
                ;进一步设置GPIOA pin5的输出模式为推挽输出,操作寄存器OTYPER
                LDR R0,=GPIOA_OTYPER;这里的地址是GPIOA的OTYPER寄存器的地址（因为它的偏移量为0x04）
                LDR R1,[R0];将GPIOA的OTYPER寄存器的值装载到R1中
                BIC R1,R1,#0x6;将OTYPER寄存器的第1位设置为0,即是推挽输出
                STR R1,[R0];将修改后的值写回到地址GPIO_OTYPER
                ;进一步设置GPIOA pin5的输出模式为高速输出,操作寄存器OSPEEDR
                LDR R0,=GPIOA_OSPEEDR;这里的地址是GPIOA的OSPEEDR寄存器的地址（因为它的偏移量为0x08）
                LDR R1,[R0];将GPIOA的OSPEEDR寄存器的值装载到R1中
                BIC R1,R1,#0xC00;将OSPEEDR寄存器的第10、11位清零
                ORR R1,R1,#0x800;将OSPEEDR寄存器的第11位设置为1,即是高速输出
                STR R1,[R0];将修改后的值写回到地址GPIO_OSPEEDR
                ;进一步设置GPIOA pin5的输出模式为下拉模式,操作寄存器PUPDR
                LDR R0,=GPIOA_PUPDR;这里的地址是GPIOA的PUPDR寄存器的地址（因为它的偏移量为0x0C）
                LDR R1,[R0];将GPIOA的PUPDR寄存器的值装载到R1中
                BIC R1,R1,#0xC00;将PUPDR寄存器的第10、11位清零
                ORR R1,R1,#0x800;将PUPDR寄存器的第11位设置为1,即是下拉模式,也就是10
                STR R1,[R0];将修改后的值写回到地址GPIO_PUPDR
                POP {R0,R1,PC};将栈中之前存的R0，R1，LR的值返还给R0，R1，PC
				ENDP
             
LED_ON			PROC
                EXPORT LED_ON
				PUSH {R0,R1, LR}    
                
                ;设置GPIOA pin5的输出数据为高电平,操作寄存器ODR
                LDR R0,=GPIOA_ODR;这里的地址是GPIOA的ODR寄存器的地址（因为它的偏移量为0x14）
                LDR R1,[R0];将GPIOA的ODR寄存器的值装载到R1中
                ORR R1,R1,#0x20;将ODR寄存器的第5位设置为1,即是高电平
                STR R1,[R0];将修改后的值写回到地址GPIO_ODR

                POP {R0,R1,PC}
				ENDP
					
LED_OFF			PROC
                EXPORT LED_OFF
				PUSH {R0,R1, LR}    
				
                ;设置GPIOA pin5的输出数据为低电平,操作寄存器ODR
                LDR R0,=GPIOA_ODR;这里的地址是GPIOA的ODR寄存器的地址（因为它的偏移量为0x14）
                LDR R1,[R0];将GPIOA的ODR寄存器的值装载到R1中
                BIC R1,R1,#0x20;将ODR寄存器的第5位设置为0,即是低电平
                STR R1,[R0];将修改后的值写回到地址GPIO_ODR
             
                POP {R0,R1,PC}             
				ENDP
					



;初始化SysTick,这里并没有开启SysTick，只是设置了重装载值和时钟源
SYSTICK_INIT    PROC
	
                EXPORT SYSTICK_INIT
				PUSH {R0,R1,LR}
                ;SYSTICK 使用外部时钟源，即是AHB/8=84/8=10.5MHz，开启中断
                LDR R0,=STK_CTRL
                LDR R1,[R0]
                BIC R1,R1,#0x4;设置来自HSE/8
                ORR R1,R1,#0x2;开启中断
                STR R1,[R0]
                ;设置重装载值为0xA，即是10.5MHz/1000000=10.500Hz，1微秒中断一次
                LDR R0,=STK_LOAD
                LDR R1,=0xA
                STR R1,[R0]
                POP {R0,R1,PC}
                ENDP
					
					

;使用SysTick实现的延时函数
;利用SysTick实现毫秒级延时
;R0作为参数，表示延时时长
SYS_DELAY_MS    PROC
                EXPORT SYS_DELAY_MS
                PUSH {R0,R1,R2, LR}
                ;设置延时时间
                LDR R1, =DELAY_NTIME
                STR R0, [R1]
                ;计数器清零
                LDR R1, =STK_VAL
                MOV R2, #0
                STR R2, [R1]
                ;启动SysTick
                LDR R1, =STK_CTRL
                LDR R2, [R1]
                ORR R2, R2, #0x1;设置使能位
                STR R2, [R1]
                ;等待延时完成
                LDR R1,=DELAY_NTIME
				
SYS_DLY_NOTOK   LDR R0,[R1];读取延时时间
                CMP R0,#0
                BNE SYS_DLY_NOTOK
                ;计数器清零
                LDR R1, =STK_VAL
                MOV R2, #0
                STR R2, [R1]
                ;关闭SysTick
                LDR R1, =STK_CTRL
                LDR R2, [R1]
                BIC R2, R2, #0x1;清除使能位
                STR R2, [R1]
                POP {R0,R1,R2,PC}
                ENDP
             END
