		.ORIG x3000 ; Start Address is x3000
		LEA R0, VALS
		LDW R1, R0, #0
		LDW R2, R0, #1
		LDW R3, R0, #2
		LDW R4, R0, #3
		AND R5, R1, R3;   = 0xA5A5
		AND R5, R5, #0;   = 0
		AND R5, R2, R3;   = 0x5A5A
		AND R5, R5, R1;   = 0x0000
		XOR R6, R1, R2;   = 0xFFFF
		XOR R6, R6, R1;   = 0x5A5A
		XOR R6, R6, #0;   = 0x5A5A
		XOR R7, R3, R4;   = 0x9999
		XOR R7, R7, x0F; = 
		NOT R7, R7;       = 
		HALT

VALS 	.FILL xA5A5 ; R1
		.FILL x5A5A ; R2
		.FILL xFFFF ; R3
		.FILL x6666 ; R4
		.END