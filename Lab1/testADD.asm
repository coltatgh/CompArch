		.ORIG x3000 ; Start Address is x3000
		LEA R0, VALS		; R0 = 0x3012 okay
		LDW R1, R0, #0		; R1 = 0x000a okay
		LDW R2, R0, #1		; R2 = 0xfff6 okay
		ADD R3, R1, R2		; R3 = 0x0000 okay	
		ADD R3, R1, x-5		; R3 = 0x0005 okay
		LDW R4, R0, #2		; R4 = 0x0010 okay
		LDW R5, R0, x3 		; R5 = 0xffe0 okay
		ADD R4, R4, R5		; R4 = 0xfff0 okay
		HALT				; R7 = 0X3012, PC = 0x0000, N = 1 okay
		
VALS 	.FILL #10 	; R1
		.FILL #-10 	; R2
		.FILL #16 	; R4
		.FILL #-32 	; R5 PUT VALUES TO TEST OVERFLOW
		.END