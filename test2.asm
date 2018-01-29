         .ORIG x4000

ROOT_PTR .FILL NODE1

         .BLKW 10

NODE1    .FILL NODE2
         .FILL x0000
         .FILL FARRAH
         .FILL #10000

         .BLKW 10

NODE2    .FILL NODE3
         .FILL x0000
         .FILL NATALIA
         .FILL #11000

         .BLKW 10

NODE3    .FILL NODE4
         .FILL x0000
         .FILL ERIC
         .FILL #12000

         .BLKW 10

NODE4    .FILL NODE5
         .FILL x0000
         .FILL MICHAEL
         .FILL #13000

         .BLKW 10

NODE5    .FILL NODE6
         .FILL x0000
         .FILL MORGEN
         .FILL #14000

         .BLKW 10

NODE6    .FILL NODE7
         .FILL x0000
         .FILL MILO
         .FILL #15000

         .BLKW 10

NODE7    .FILL NODE8
         .FILL x0000
         .FILL ELIOT
         .FILL #16000

         .BLKW 10

NODE8    .FILL NODE9
         .FILL x0000
         .FILL KELSO
         .FILL #17000

         .BLKW 10

NODE9    .FILL NODE10
         .FILL x0000
         .FILL SEAN
         .FILL #18000

         .BLKW 10

NODE10   .FILL x0000
         .FILL x0000
         .FILL TODD
         .FILL #19000

         .BLKW 10

FARRAH   .STRINGZ "Farrah"
NATALIA  .STRINGZ "Natalia"
ERIC     .STRINGZ "Eric"
MICHAEL  .STRINGZ "Michael"
MORGEN   .STRINGZ "Morgen"
MILO     .STRINGZ "Milo"
ELIOT    .STRINGZ "Eliot"
KELSO    .STRINGZ "Kelso"
SEAN     .STRINGZ "Sean"
TODD     .STRINGZ "Todd"

         .END

