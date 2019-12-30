[FORMAT "WCOFF"] ;生成对象文件模式
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"] ;源文件信息

        GLOBAL  _api_putchar
        GLOBAL  _api_end
        GLOBAL  _api_putstr0

[SECTION .text]

_api_putchar:
        MOV     EDX,1
        MOV     AL,[ESP+4]
        INT     0x40
        RET

_api_end:
        MOV     EDX,4
        INT     0x40

_api_putstr0:
        PUSH    EBX
        MOV     EDX,2
        MOV     EBX,[ESP+8]
        INT     0x40
        POP     EBX
        RET


