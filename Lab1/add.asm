		.ORIG x3000
			LEA R0, FOURK
			ADD R1, R1, #1
			STB R1, R0, #0
			
			LEA R1, C
			ADD R2, R2, #15
			ADD R2, R2, #5
FOR			ADD R2, R2, #-1		; R2 is iterator
			BRzp ADDADDRESS
			BRn	END

ADDADDRESS	LDB R4, R1, #0
			ADD R3, R3, R4 ; R3 is accumulator
			ADD R1, R1, #1 ; increment address
			BR FOR

END			STW R3, R5, #0 ; uncomment to test for protection
			;STW R3, R5, #1 ; uncomment to test for misalignment
			;.FILL xA000 	; uncomment to test for invalid opcode

			STW R3, R1, #0; store to 0xC014 
			HALT 	; put after the .FILL to test invalid OPCODE

FOURK		.FILL x4000
C			.FILL xC000

		.END