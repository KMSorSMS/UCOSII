		THUMB
		PRESERVE8


		AREA |.text|, CODE, READONLY,ALIGN=3

		
MY_MS5611_READ_D PROC
		EXPORT MY_MS5611_READ_D
		IMPORT iic_start
		IMPORT iic_send_byte
		IMPORT iic_wait_ack
		IMPORT iic_stop
		IMPORT iic_read_byte
;
		PUSH          {r4-r10,lr}
		
		MOV           r5,r0
		MOV           r4,r1
		BL.W          iic_start 
		
		;iic_send_byte(MS5611_ADDR);                          
		MOVS          r0,#0xEE
		BL.W          iic_send_byte 
		BL.W          iic_wait_ack 
        
		;iic_send_byte(cmd);            
		MOV           r0,r5
		BL.W          iic_send_byte 
		BL.W          iic_wait_ack 
		BL.W          iic_stop 
		
		
		BL.W          iic_start 
		
		;iic_send_byte(MS5611_ADDR);              
		MOVS          r0,#0xEE
		BL.W          iic_send_byte 
		BL.W          iic_wait_ack 
		
		;iic_send_byte(MS5611_ADC_READ);               
		MOVS          r0,#0x00
		BL.W          iic_send_byte
		BL.W          iic_wait_ack 
		BL.W          iic_stop 

		BL.W          iic_start 
		;iic_send_byte(MS5611_ADDR+1);          
		MOVS          r0,#0xEF
		BL.W          iic_send_byte 
		BL.W          iic_wait_ack
		
		;res1 = iic_read_byte(1);                  
		MOVS          r0,#0x01
		BL.W          iic_read_byte
		MOV           r6,r0
		
		;res2 = iic_read_byte(1); 
		MOVS          r0,#0x01
		BL.W          iic_read_byte 
		MOV           r7,r0
   
		;res3 = iic_read_byte(0); 
		MOVS          r0,#0x00
		BL.W          iic_read_byte 
		MOV           r8,r0
		BL.W          iic_stop 
		
		;uint16_t temp=(((uint16_t)res1)<<8)|((uint16_t)res2); 
		ORR           r9,r7,r6,LSL #8
		;*D =(((uint32_t)temp)<<8)|((uint32_t)res3); 
		ORR           r0,r8,r9,LSL #8
		STR           r0,[r4,#0x00]
		POP           {r4-r10,pc}
		ENDP
			
		END