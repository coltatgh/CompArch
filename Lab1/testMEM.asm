		.ORIG x3000 ; Start Address is x3000
		JSR BLK1	; R7 = 0x3002
		LEA R1, BLK2
		JSRR R1
		BR BLK3

VALS	 	.FILL x1100 
LAST		.FILL x5544    
		.FILL x3322 

LOC		.FILL x3500
		.FILL x3502
		.FILL x3504

BLK1		LEA R0, VALS; 	R0 = 0x3008
		LDW R1, R0, #0;	R1 = 0x1100
		LEA R2, LOC;	R2 = 0x300e
		LDW R3, R2, #0;	R3 = 0x3500
		STB R1, R3, #0;	STB doesn't seem to work correctly, mem @ 3500 meant nothing
		RSHFL R1, R1, #8; R1 = 0x0011 okay
		STB R1, R3, #1;
		RET

BLK2		LDB R1, R0, #4; check my math... want x22
		LDB R4, R0, #5; wanting x33
		LSHF R4, R4, #8
		ADD R1, R1, R4;
		LDW R3, R2, #1;
		STW R1, R3, #0;
		RET

BLK3		LEA R0 LAST; positive address = set p?
		BRnz CRASH
		BRp HOP1

CRASH		.FILL x25

HOP1		AND R4, R4, #0
		BRnp BURN
		LDW R1, R0, #0;
		LDW R3, R2, #1;  get 0x3502 again
		ADD R4, R4, #-1; should be xFFFF
		NOT R4, R4; 	 should be 0 so z is set
		BRz HOP2

BURN 		.FILL x25

HOP2 		ADD R4, R4, x-1; should be -1 so n set
		BRzp DIE
		BRn DONE

DIE		.FILL x25

DONE		STW R1, R3, #1; should target 0x3504
		HALT
		.END

		;should leave memory like:
		;
		; 0x3500 = x00
		; 0x3501 = x11
		; 0x3502 = x22
		; 0x3503 = x33
		; 0x3504 = x44
		; 0x3505 = x55




		
