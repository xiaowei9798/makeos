[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api005.nas"]

        GLOBAL  _api_openwin

[SECTION .text]

_api_openwin:
        ;    在windows里面的规则是，EAX,ECX,EDX,这三个寄存器由调用者保存，故不需要保存这些寄存器的值
        PUSH    EDI
        PUSH    ESI
        PUSH    EBX
        MOV     EDX,5
        MOV     EBX,[ESP+16]
        MOV     ESI,[ESP+20]
        MOV     EDI,[ESP+24]
        MOV     EAX,[ESP+28]
        MOV     ECX,[ESP+32]
        INT     0x40
        POP     EBX
        POP     ESI
        POP     EDI
        RET

        