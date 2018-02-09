		.ORIG x3000 ; Start Address is x3000
		LEA R0, VALS		; R0 = 0x3016	okay
		LDW R1, R0, #0		; R1 = 0x0001	okay
		LDW R2, R0, #1		; R2 = 0xffff	okay
		LDW R3, R0, #2		; R3 = 0xf000	okay
		LDW R4, R0, #3		; R4 = 0x0000	okay
		LSHF R5, R1, #5;   	; R5 = 0x0020	okay
		RSHFL R6, R2, #7	; R6 = 0x01ff	okay
		RSHFA R7, R2, #7	; R7 = 0xffff	okay
		RSHFL R6, R3, #5	; R6 = 0x0780	okay
		RSHFA R7, R3, #5	; R7 = 0xFF80	okay
		HALT

VALS 	.FILL #1      ; R1
		.FILL #-1     ; R2
		.FILL xF000 ; R3
		.END