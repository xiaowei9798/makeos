[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "crack7.nas"]

        GLOBAL  _HariMain

[SECTION .text]

_HariMain:
    ; MOV     AX,1005*8    ;读取1005号代码段
    MOV     AX,4
    MOV     DS,AX
    CMP     DWORD [DS:0x0004],'Hari'

    JNE     fin

    MOV     ECX,[DS:0x0000]    ;取出该应用程序数据段的大小
    MOV     AX,12
    ; MOV     AX,2005*8          ;读取数据段   
    MOV     DS,AX

crackloop:
    ADD     ECX,-1
    MOV     BYTE [DS:ECX],123   ;将数据段的内容全部赋为123
    CMP     ECX,0
    JNE     crackloop

fin:
    MOV     EDX,4      
    INT     0x40


