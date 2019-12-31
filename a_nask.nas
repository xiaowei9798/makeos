[FORMAT "WCOFF"] ;生成对象文件模式
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"] ;源文件信息

        GLOBAL  _api_putchar
        GLOBAL  _api_end
        GLOBAL  _api_putstr0
        GLOBAL  _api_openwin
        GLOBAL  _api_putstrwin
        GLOBAL  _api_boxfilwin

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

_api_openwin:
        ;    在windous里面的规则是，EAX,ECX,EDX,这三个寄存器由调用者保存，故不需要保存这些寄存器的值
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

_api_putstrwin:
        PUSH    EDI
        PUSH    ESI
        PUSH    EBP
        PUSH    EBX
        MOV     EDX,6
        MOV     EBX,[ESP+20]
        MOV     ESI,[ESP+24]
        MOV     EDI,[ESP+28]
        MOV     EAX,[ESP+32]
        MOV     ECX,[ESP+36]
        MOV     EBP,[ESP+40]
        INT     0x40
        POP     EBX
        POP     EBP
        POP     ESI
        POP     EDI
        RET

_api_boxfilwin:
        PUSH    EDI
        PUSH    ESI
        PUSH    EBP
        PUSH    EBX
        MOV     EDX,7
        MOV     EBX,[ESP+20]
        MOV     EAX,[ESP+24]
        MOV     ECX,[ESP+28]
        MOV     ESI,[ESP+32]
        MOV     EDI,[ESP+36]
        MOV     EBP,[ESP+40]
        INT     0x40
        POP     EBX
        POP     EBP
        POP     ESI
        POP     EDI
        RET 


