		
		
		THUMB
		PRESERVE8


		AREA |.text|, CODE, READONLY,ALIGN=3;开始代码段


;这里我们使用了浮点数，所以需要使能FPU处理单元
;这个单元在System_Init模块中使能了
;SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); 
;/* set CP10 and CP11 Full Access */


;这个函数需要传入三个参数float* x,float* y,float* z;
;这三个分别存储从HMC5883读出的数据
;这三个数据存入r0,r1,r2
RAW_ACC_READ_MPU6050 PROC
		EXPORT RAW_ACC_READ_MPU6050
		IMPORT Peripheral_ReadBytes
		
		;这里我们除了需要传入参数的r0-r2,还需要使用r4-r6寄存器，所以将其压栈
		PUSH          {r4-r6,lr}
		
		;VPUSH这是针对于浮点数相关的操作指令，也是进行压栈操作，d8-d9代表双精度浮点数double
		;V字开头是什么关于浮点类型数据的操作
		VPUSH      {d8-d9} 
		
		SUB           sp,sp,#0x18
		MOV           r4,r0
		MOV           r5,r1
		MOV           r6,r2
		
		;Peripheral_ReadBytes(MPU6050_ADDRESS,MPU6050_ACCEL_XOUT_H,BUF,6);
		MOVS          r3,#0x06
		ADD           r2,sp,#0x04
		MOVS          r1,#0x3B
		MOVS          r0,#0xD0
		
		BL.W          Peripheral_ReadBytes

		;tempX=(((int16_t)BUF[0])<<8)|((int16_t)BUF[1]); 
		LDRB          r0,[sp,#0x05]
		LDRB          r1,[sp,#0x04]
		
		;在这个指令中，r0 寄存器的值会与 r1 寄存器左移 8 位后的值进行逻辑或操作，
		;然后将结果存储回 r0 寄存器。
		ORR           r0,r0,r1,LSL #8
		SXTH          r0,r0
		VMOV          s16,r0
		
		;tempY=(((int16_t)BUF[2])<<8)|((int16_t)BUF[3]); 
		LDRB          r0,[sp,#0x07]
		LDRB          r1,[sp,#0x06]
		ORR           r0,r0,r1,LSL #8
		SXTH          r0,r0;SXTH {Rd,} Rm {,ROR #n} Sign extend a halfword
		VMOV          s17,r0
		
		;tempZ=(((int16_t)BUF[4])<<8)|((int16_t)BUF[5]); 

		LDRB          r0,[sp,#0x09]
		LDRB          r1,[sp,#0x08]
		ORR           r0,r0,r1,LSL #8
		SXTH          r0,r0
		VMOV          s18,r0
		
		ENDP
		

RAW_READ_MPU6050 PROC
		EXPORT RAW_READ_MPU6050
		IMPORT Peripheral_ReadBytes
		
		PUSH          {r4-r7,lr}
		SUB           sp,sp,#0x14
		MOV           r4,r0
		MOV           r5,r1
		MOV           r6,r2
		MOV           r7,r3
        ;Peripheral_ReadBytes(MPU6050_ADDRESS,readType,BUF,6);

		UXTH          r1,r7
		;将寄存器 Rm 中的 16 位半字进行无符号扩展，
		;并将结果存储到目标寄存器 Rd 中。无符号扩展将高位都填充为 0。
		
		MOVS          r3,#0x06
		MOV           r2,sp
		MOVS          r0,#0xD0
		BL.W          Peripheral_ReadBytes 
		
		;*x=(((int16_t)BUF[0])<<8)|((int16_t)BUF[1]); 
		LDRB          r0,[sp,#0x01]
		LDRB          r1,[sp,#0x00]
		ORR           r0,r0,r1,LSL #8
		SXTH          r0,r0
		;将寄存器 Rm 中的 16 位半字进行有符号扩展，并将结果存储到目标寄存器 Rd 中。
		;有符号扩展将使用半字的最高位进行填充，以保持符号性。
		
		STRH          r0,[r4,#0x00]
		;存储半字到内存中
		
		;*y=(((int16_t)BUF[2])<<8)|((int16_t)BUF[3]); 
		LDRB          r0,[sp,#0x03]
		LDRB          r1,[sp,#0x02]
		ORR           r0,r0,r1,LSL #8
		SXTH          r0,r0; Sign extends a 16-bit value to a 32-bit value.
		STRH          r0,[r5,#0x00]
		
		;*z=(((int16_t)BUF[4])<<8)|((int16_t)BUF[5]); 

		LDRB          r0,[sp,#0x05]
		LDRB          r1,[sp,#0x04]
		ORR           r0,r0,r1,LSL #8
		SXTH          r0,r0
		STRH          r0,[r6,#0x00] 
		ADD           sp,sp,#0x14
		POP           {r4-r7,pc}
		
		ENDP
		
		END
		
		