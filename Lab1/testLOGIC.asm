		.ORIG x3000 ; Start Address is x3000
		LEA R0, VALS	; x3020
		LDW R1, R0, #0	; xa5a5
		LDW R2, R0, #1	; x5a5a
		LDW R3, R0, #2	; xffff
		LDW R4, R0, #3	; x6666
		AND R5, R1, R3;   = 0xA5A5
		AND R5, R5, #0;   = 0
		AND R5, R2, R3;   = 0x5A5A
		AND R5, R5, R1;   = 0x0000
		XOR R6, R1, R2;   = 0xFFFF
		XOR R6, R6, R1;   = 0x5A5A
		XOR R6, R6, #0;   = 0x5A5A
		XOR R7, R3, R4;   = 0x9999
		XOR R7, R7, x0F;  = 0x9996
		NOT R7, R7;       = 0x6669
		HALT

VALS 	.FILL xA5A5 ; R1
		.FILL x5A5A ; R2
		.FILL xFFFF ; R3
		.FILL x6666 ; R4
		.END
