[INSTRSET "i486p"]
[BITS 32]
        ;应用程序要破坏操作系统，只能通过系统调用API的方法来实现
        MOV     EDX,123456789
        INT     0x40
        MOV     EDX,4
        INT     0x40


        