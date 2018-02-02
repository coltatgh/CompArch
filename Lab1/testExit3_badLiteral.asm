.orig x3000
ADD R0, R0, x10 ;should exit(3)
ADD R0, R0, #-17 ; should exit(3)
ADD R0, R0, #16 ; should exit(3)
ADD R0, R0, x-11 ; should exit(3)
.end