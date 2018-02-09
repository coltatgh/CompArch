7		.ORIG x3000 ; Start Address is x3000
TEST	ADD R2,R2,R2
		ADD R7,R7,#0
		ADD R1,R5,x0F	; r1 = xF 		okay
		ADD R1 R1 #-10	; r1 = 5 		okay
		ADD R6 R6 x-9	; r6 = xfff7 	okay
		AND R1,R1,R2	; r1 = 0 		okay
		BR TEST			; pc = 0x3000	okay	inf loop loool
		AND R3,R4,#5	
		JMP R2 			
		RET
		JSR TEST			
		JSRR R6	
		LDB R4,R2,#10
		LDW R4,R2,#10
		LSHF R2,R3,x3
		NOT R1,R2	
		RTI
		RSHFL R2,R3,#3
		RSHFA R2,R3,#7
		STB R4 R2 #10
		STW R4,R2,#10
		TRAP x25
		.FILL x-1
		XOR R3,R1,R2
		XOR R3,R1,#12
		BR TEST
		BRNZP TEST
		BRP TEST
		.END