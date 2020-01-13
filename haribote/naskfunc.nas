; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 生成?象文件的模式	
[INSTRSET "i486p"]				; 使用486兼容指令集
[BITS 32]						; 生成32位模式机器?言
[FILE "naskfunc.nas"]			; 源文件名信息

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL	_load_tr
		GLOBAL	_asm_inthandler20, _asm_inthandler21
		GLOBAL	_asm_inthandler27, _asm_inthandler2c
		GLOBAL  _asm_inthandler0d, _asm_inthandler0c
		GLOBAL	_memtest_sub
		GLOBAL	_farjmp
		GLOBAL	_farcall
		GLOBAL	_asm_hrb_api, _start_app
		GLOBAL  _asm_end_app
		EXTERN	_inthandler20, _inthandler21
		EXTERN	_inthandler27, _inthandler2c
		EXTERN  _inthandler0d,_inthandler0c
		EXTERN	_hrb_api

[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS という意味
		POP		EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS という意味
		RET

_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

_load_tr:		; void load_tr(int tr);
		LTR		[ESP+4]			; tr
		RET

_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler0d:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP 	EAX,0
		JNE 	end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD 	ESP,4       ;在INT 0x0d中需要加入?句
		IRETD

_asm_inthandler0c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0c
		CMP 	EAX,0
		JNE 	end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD 	ESP,4       ;在INT 0x0d中需要加入?句
		IRETD

_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; （EBX, ESI, EDI も使いたいので）
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

_farjmp:		; void farjmp(int eip, int cs);
		JMP		FAR	[ESP+4]				; eip, cs
		RET

_farcall:
		CALL	FAR [ESP+4]
		RET
								
_asm_hrb_api:
		STI
		PUSH 	DS
		PUSH 	ES
		PUSHAD  		;用于保存的PUSH
		PUSHAD			;用于向hrb_api??的PUSH
		MOV 	AX,SS
		MOV 	DS,AX	;  将操作系?用段地址存入DS和ES
		MOV 	ES,AX
		CALL 	_hrb_api
		CMP 	EAX,0		; 当EAX不?0?程序?束，?明出?了?常或者EDX==4程序自然?束
		JNE 	end_app
		ADD 	ESP,32
		POPAD
		POP 	ES        ;此?相当于借助IRETD方法，通?事先保存的DS和ES，将其POP出来，然后跳?到?地址
		POP 	DS
		IRETD
end_app:
		; 此?EAX?tss.esp0的地址
		MOV 	ESP,[EAX]
		POPAD
		RET      ;返回cmd_app

_start_app:
		PUSHAD		; 将32位寄存器的?全部保存起来
		MOV		EAX,[ESP+36]	; ?用程序用的EIP
		MOV		ECX,[ESP+40]	; アプリ用のCS
		MOV		EDX,[ESP+44]	; アプリ用のESP
		MOV		EBX,[ESP+48]	; アプリ用のDS/SS
		MOV 	EBP,[ESP+52]    ;tss.esp0的地址
		MOV		[EBP],ESP	    ;保存操作系?用ESP
		MOV 	[EBP+4],SS		;保存操作系?用SS
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
		; 下面?整?，以便用RETF跳?到?用程序
		OR 		ECX,3  			;将?用程序用段号和3?行OR?算，是?用RETF?用?用程序而使用的一个小技巧
		OR 		EBX,3
		PUSH 	EBX
		PUSH 	EDX
		PUSH 	ECX
		PUSH  	EAX
		RETF

_asm_end_app:
		MOV 	ESP,[EAX]
		MOV 	DWORD [EAX+4],0
		POPAD
		RET

