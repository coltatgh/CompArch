         .ORIG x4000

NODE1    BR NODE2
         BR NODE3
         .FILL #10000

NODE2    BR NODE4
         BR NODE5
         .FILL #12000

NODE3    BR NODE6
         BR NODE7
         .FILL #14000

NODE4    .FILL x0000
         .FILL x0000
         .FILL #12345

NODE5    .FILL x0000
         .FILL x0000
         .FILL #32123

NODE6    .FILL x0000
         .FILL x0000
         .FILL #30000

NODE7    .FILL x0000
         .FILL x0000
         .FILL #20000

         .END

