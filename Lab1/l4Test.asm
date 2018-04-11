.ORIG x3000
	;STW R0, R1, #0 ; uncomment to test for protection
	.FILL xA000 	; uncomment to test for invalid opcode

	;LEA R5, THREEK
	;STW R3, R5, #1 ; uncomment to test for misalignment
	ADD R1, R1, #1 ; shouldn't happen b/c was already halted
	;HALT	

	THREEK .FILL x3010
.END