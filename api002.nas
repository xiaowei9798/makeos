[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api002.nas"]

        GLOBAL  _api_putstr0

[SECTION .text]

_api_putstr0:
        PUSH    EBX
        MOV     EDX,2
        MOV     EBX,[ESP+8]
        INT     0x40
        POP     EBX
        RET

        