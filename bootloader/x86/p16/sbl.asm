%ifndef _SBL_NASM_
%define _SBL_NASM_

[BITS 16]

entry:

PBL_ENTRY_POINT equ 0x100000

DISK_SEG_LIMIT  equ 0xFFFF
BIOS_READ_SECS	equ 0x02

; VGA 
VGA_TEST_BASE 	equ 0xB800
VGA_LINE_BYTES	equ 160

SECTION .text
jmp 0x0000:_sbl_start

global _sbl_start

_sbl_start:
	pop word [start_cylin]
	pop word [start_head]
	pop word [start_sec]
	pop word [cylins]
	pop word [heads]
	pop word [secs]
	pop word [vga_rows]

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
	jne _load_kernel 

.a20_enable:
	push A20_MSG               
    call vga_text_print         
    add sp, 2

_load_kernel:
	pusha
	
	mov ax, 0xFFFF
	mov es, ax
	mov bx, 0x10

	_disk_loop:
		; migration address
		mov ax, 512
		mul word [secs]
		mov cx, bx
		add bx, ax
		jc _disk_loop_exit
		
		mov bx, cx

		; read the sectors 
		mov al, [secs]					; sector count to read
		mov ah, BIOS_READ_SECS			; BIOS INT 13h F2h:Read Sectors from drive
		mov ch, byte [start_cylin]		; start to cylinder
		mov cl,	byte [start_sec]		; start to sector
		mov dh, byte [start_head]		; start to head

		int 0x13						; request BIOS INT13h F2h
		jc _error						; if carry set, error
		
		cmp al, [secs]					; if successed, return the read sectors count to al
		jne _error	

		; increment offset(bx)
		mov ax, 512
		mul word [secs]
		add bx, ax

		; calculate next head count	
		mov ax, [heads]
		inc word [start_head]
		cmp [start_head], ax
		jl _disk_loop

		mov word [start_head], 0
	
		; calculate next cylinder count
		mov ax, [cylins]
		inc word [start_cylin]				
		cmp [start_cylin], ax 
		jl _disk_loop

		jmp _error

	_disk_loop_exit:
	popa

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
	
	jmp 0x08:_penter

[BITS 32]
_penter:
	jmp 0x100000;	

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

_error:
	jmp $

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


start_cylin :   dw 0
start_head	:   dw 0
start_sec	:   dw 0
cylins 		:   dw 0
heads 		:	dw 0
secs		: 	dw 0
vga_rows	: 	dw 0
MSG_SEC_BOOT: 	db 'YohdaOS Secondary Boot Loader Start', 0
A20_MSG:		db 'For entering to protected mode, preparing for the Gate-A20', 0
GDT_MSG:		db 'Start preparing for GDT of protected mode', 0

size equ $ - entry
times ((512*35) - size) nop
;times 512 - ($-$$) db 0x00
;times 512*34 db 0x4F 
%endif 
