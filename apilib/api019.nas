[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api019.nas"]

        GLOBAL  _api_freetimer

[SECTION .text]

_api_freetimer:
        PUSH    EBX
        MOV     EDX,19
        MOV     EBX,[ESP+8]
        INT     0x40
        POP     EBX
        RET
        
        