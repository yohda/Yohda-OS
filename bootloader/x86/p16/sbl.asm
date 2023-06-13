%ifndef _SBL_NASM_
%define _SBL_NASM_

[BITS 16]

entry:

%ifndef DEBUG
extern _pbl_start
%endif

; VGA 
VGA_TEST_BASE equ 0xB800
VGA_LINE_BYTES equ 160

SBL_ALLOC_SECS equ 32

SECTION .text
jmp 0x0000:_sbl_start

global _sbl_start

_sbl_start:
	pop ax
	pop bx

	mov word [vga_rows], ax

	push MSG_SEC_BOOT
	call vga_text_print
	add sp, 2

_a20_sec:
.a20_chk:
	push es
	
	mov ax, 0x0000 
	mov es, ax
	mov bx, [es:0x7DFE]

	not ax
	mov es, ax
	mov cx, [es:0x7E0E]

	pop es
	
	cmp bx, cx
	jne _gdt_sec 

.a20_enable:
	push A20_MSG               
    call vga_text_print         
    add sp, 2

_gdt_sec:
	push GDT_MSG               
    call vga_text_print         
    add sp, 2

.gdt_load:
	xor eax, eax
	mov ax, ds
	shl eax, 4
	add eax, _gdt_tbl
	mov [_gdtr+2], eax
	mov eax, _gdtr
	sub eax, _gdt_tbl
	mov [_gdtr], ax
	lgdt [_gdtr]

_pmode:	
	mov eax, cr0
	or al, 1
	mov cr0, eax	
	
[BITS 32]
%ifdef DEBUG
	jmp $
%else
	jmp 0x08:_pbl_start
%endif
	
[BITS 16]

vga_text_print:
	push bp
	mov bp, sp

	push ax
	push bx
	push si	
	push di	
	push es

	mov ax, VGA_TEST_BASE 	
	mov es,	ax

	mov ax, VGA_LINE_BYTES	
	mul word [vga_rows]	
	mov di, ax	
	inc word [vga_rows]
	
	mov si, [bp+4]	

.print:
	mov bl, byte [si]
	
	cmp bl, 0
	je .print_end
	
	mov byte [es:di], bl 
	
	add si, 1
	add di, 2	

	jmp .print

.print_end:
	pop es
	pop di
	pop si
	pop bx
	pop ax
	
	pop bp

	ret

_gdt_tbl:
	; NULL Segment
	dw 0x0000
	dw 0x0000
	dw 0x0000
	dw 0x0000

	; Code Segment
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 10011010b
	db 11001111b
	db 0x00

	; Data Segment
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 10010010b
	db 11001111b
	db 0x00

_gdtr:
	dw 0 
	dd 0

vga_rows	: 	dw 0
MSG_SEC_BOOT: 	db 'YohdaOS Secondary Boot Loader Start', 0
A20_MSG:		db 'For entering to protected mode, preparing for the Gate-A20', 0
GDT_MSG:		db 'Start preparing for GDT of protected mode', 0

size equ $ - entry
times (1024 - size) db 0x00
;times 512 - ($-$$) db 0x00
;times 512*34 db 0x4F 
%endif 
