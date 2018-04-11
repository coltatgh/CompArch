		.ORIG x3000 ; Start Address is x3000
		JSR BLK1;


VALS 	.FILL x1100 
		.FILL x5544    
		.FILL x3322 

LOC		.FILL x3500
		.FILL x3502
		.FILL x3504

BLK1	LEA R0, VALS;
		LDW R1, R0, #0;
		LEA R2, LOC;
		LDW R3, R2, #0;
		STW R1, R3, #0;
		;Work in progress
		.END



		