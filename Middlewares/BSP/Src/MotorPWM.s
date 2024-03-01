		
		THUMB
		PRESERVE8


	AREA |.text|, CODE, READONLY,ALIGN=4
	

MOTOR_PWM_TEST PROC
	EXPORT MOTOR_PWM_TEST
	
	PUSH {r1}
	
	;TIM3->CCR1 = M
	MOV r1,#0x0434
	MOVT r1,#0x4000
	
	STR r0,[r1,#0x00]
	
	;TIM3->CCR2 = M
	ADDS r1,r1,#4
	STR r0,[r1,#0x00]
	
	;TIM->CCR3 = M
	ADDS r1,r1,#4
	STR r0,[r1,#0x00]
	
	;TIM->CCR4 = M
	ADDS r1,r1,#4
	STR r0,[r1,#0x00]
	
	POP {r1}
	BX lr
	
	ENDP
		
		
MOTOR_PWM_SET PROC
	EXPORT MOTOR_PWM_SET
	
	;参数M1在r0中，M2在r1中，M5在sp指向的单元的上一个单元中，M6在sp指向的单元中，M3在r2中，M4在r3中
	
	PUSH {r4-r6,lr}
	LDRD r4,r5,[sp,#0x10]
	
	;TIM3->CCR1 = M1
	;TIM3->CCR1地址
	MOV r6,#0x0434
	MOVT r6,#0x4000
	
	STR r0,[r6,#0x00]
	
	;TIM3->CCR2 = M2
	ADDS r6,r6,#4
	STR r1,[r6,#0x00]
	
	;TIM3->CCR3 = M5
	ADDS r6,r6,#4
	STR r4,[r6,#0x00]
	
	;TIM3->CCR4 = M6
	ADDS r6,r6,#4
	STR r5,[r6,#0x00]

	POP {r4-r6,pc}
	
	ENDP
		
	END
