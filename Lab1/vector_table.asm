.ORIG x0200
	.FILL xFFFF	; no interrupt w/ Vector 0
	.FILL x1200	; Timer Interrupt
	.FILL x1600	; Protection Exception
	.FILL x1A00 ; Unaligned Exception
	.FILL x1C00 ; Opcode Exception
.END
