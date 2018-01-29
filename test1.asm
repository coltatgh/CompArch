         .ORIG x4000

ROOT_PTR .FILL NODE1

NODE1    .FILL NODE2
         .FILL NODE3
         .FILL #10000

NODE2    .FILL NODE4
         .FILL NODE5
         .FILL #12000

NODE3    .FILL NODE6
         .FILL NODE7
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

