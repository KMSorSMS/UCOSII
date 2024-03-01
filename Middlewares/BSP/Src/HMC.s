HMC_SLAVE_ADDRESS EQU 0X3C;这是HMC写的设备地址
;读地址为0x3c+1

;这里没有使用prinf函数
		
		THUMB
		PRESERVE8


	AREA |.text|, CODE, READONLY,ALIGN=4;开始代码段

		
;这个函数需要传入三个参数int* x,int* y,int* z;
;这三个分别存储从HMC5883读出的数据
;这三个数据存入r0,r1,r2
MULTIPLE_READ_HMC5883 PROC;从HMC读取多个字节
		EXPORT MULTIPLE_READ_HMC5883
		IMPORT Peripheral_ReadBytes
			
		;这里进入函数进行参数压栈，将r4-r6进行压栈，将r1-r3赋给r4-r6
		PUSH {r4-r6,lr}
		SUB sp,sp,#0x18
		MOV r4,r0
		MOV r5,r1
		MOV r6,r2
		
		;这里调用Peripheral_ReadBytes，它需要传入
		;HMC_Slave_Address,0x03,BUF,HMC_DATA_SIZE这四个数据
		MOVS  r3,#0x06 ;这个是HMC_DATA_SIZE的大小
		ADD   r2,sp,#0x04 ;这个是传入BUF参数
		MOVS  r1,#0x03 ;这个传入立即数3
		MOVS  r0,#0x3C ;这个传入参数HMC_SLAVE_ADDRESS
		BL  Peripheral_ReadBytes
		
		
		;*x = (int16_t)(BUF[0] << 8) | (BUF[1])
		LDRB  r0,[sp,#0x04];只加载一个字节到中
		LSLS  r0,r0,#8;左移r0 8位
		SXTH  r0,r0;将半字扩展为字
		LDRB  r1,[sp,#0x05];加载一个字节到r1中
		ORRS  r0,r0,r1 ;使用异或
		STRH  r0,[r4,#0x00];存储半字到内存中
		
		;*z = (int16_t)(BUF[2] << 8) | (BUF[3]); 
		LDRB          r0,[sp,#0x06]
		LSLS          r0,r0,#8
		SXTH          r0,r0
		LDRB          r1,[sp,#0x07]
		ORRS          r0,r0,r1
		STRH          r0,[r6,#0x00]
		
		;*y = (int16_t)(BUF[4] << 8) | (BUF[5]); 
		LDRB          r0,[sp,#0x08]
		LSLS          r0,r0,#8
		SXTH          r0,r0
		LDRB          r1,[sp,#0x09]
		ORRS          r0,r0,r1
		STRH          r0,[r5,#0x00]
		
		ADD           sp,sp,#0x18
		POP           {r4-r6,pc}
		ENDP
		

;向指定地址发送数据
;需要传入两个参数，第一个参数data，和第二个参数addr
HMC5883_SEND_BYTE PROC
		EXPORT HMC5883_SEND_BYTE
		IMPORT iic_start
		IMPORT iic_stop
		IMPORT iic_send_byte
		IMPORT iic_wait_ack
		
		;这里压入了r4-r6,同时将r5，r4存储了r0,r1,r0-r3自由使用
		PUSH {r4-r6,lr}
		MOV r5,r0
		MOV r4,r1
		
		BL iic_start
		
		MOVS r0,#0x3C
		BL iic_send_byte
		BL iic_wait_ack
		
		;iic_send_byte(addr);
		MOV r0,r4
		BL iic_send_byte
		BL iic_wait_ack
		
		;iic_send_byte(data);
		MOV r0,r5
		BL iic_send_byte
		BL iic_wait_ack
		
		BL iic_stop
		
		POP {r4-r6,pc}
		ENDP



HMC5883L_INIT PROC
	EXPORT HMC5883L_INIT
	PUSH {r0-r4,lr}
	
	;HMC5883_SendByte(0x78, 0x00);
	MOVS r1,#0x00
	MOVS r0,#0x78
	BL HMC5883_SEND_BYTE
	
	;HMC5883_SendByte(0x40, 0x01);
	MOVS r1,#0x01
	MOVS r0,#0x40
	BL HMC5883_SEND_BYTE
	
	;HMC5883_SendByte(0x00, 0x02); 
	MOVS r1,#0x02
	MOVS r0,#0x00
	BL HMC5883_SEND_BYTE
	
	POP {r0-r4,pc}
	ENDP
	END
