
Readkbd:

        JSR     PrintAx2
.Readkbd1:        
        MOVEM.L D1-D7/A0-A6,-(SP)
        JSR     $8426.L
        MOVEM.L (SP)+,D1-D7/A0-A6
        CMP.B   #$0,D0
        BEQ.S   .Readkbd1

        JSR     PrintAx3

        RTS


Configkbd:
        MOVE.L  #Readkbd,A1         ; Initialize cconin console char in
        MOVE.L  A1,cconin
        RTS



