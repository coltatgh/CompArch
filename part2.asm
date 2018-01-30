
		.ORIG 	x3000
; ---Simple Names for Memory Locations----------------------------------
SRC0	.FILL	x4000
SRC1	.FILL	x4001
SRC2	.FILL	x4002
SRC3	.FILL	x4003
CTRL	.FILL	x4004
DST0	.FILL	x4005
DST1	.FILL	x4006
DST2	.FILL	x4007
DST3	.FILL	x4008

; ---Setup -------------------------------------------------------------
		LEA R0, DST0	
		LDW R0, R0, #0	; R0 holds the current DST location
		AND R1, R1, #0	; R1 will be the offset counter
		LEA R2, CTRL
		LDB R2, R2, #0	; R2 holds the control mask

; ---Determine which source to store in current DST --------------------
LOOP	AND R3, R2, #3
		ADD R3, R3, #-1
		BRn ZERO
		BRz ONE
		ADD R3, R3, #-2	; should I reset R3 and subtract 3 for clarity?
		BRn TWO
		BRz THREE

ZERO	LEA R4, SRC0	; Select SRC0
		BR SHFL

ONE		LEA R4, SRC1	; Select SRC1
		BR SHFL

TWO		LEA R4, SRC2	; Select SRC2
		BR SHFL

THREE	LEA R4, SRC3	; Select SRC3
		BR SHFL

; ---Shuffle accordingly -----------------------------------------------
SHFL	LDW R4, R4, #0	; get the address of the selected source
		LDB R5, R4, #0	; get the contents of the selected source
		STB R5, R0, R1	; And then store them in the current DST location

; Check if done
		ADD R6, R1, #-3
		BRzp DONE

; If not, reset for next loop
		RSHFL R2, R2, #2	; move to the next bit pair in the control mask
		ADD R1, R1, #1		; move to the next DST address
		BR LOOP

; ---Finish ------------------------------------------------------------
DONE  	TRAP x25		; Halt machine
		.END