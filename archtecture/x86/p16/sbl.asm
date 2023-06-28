%ifndef _SBL_NASM_
%define _SBL_NASM_

[BITS 16]

extern main

entry:

; 32-bt Kernel meta data
PMODE_ENTRY_POINT equ 0x10000
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
	pop word [drive_number]
	
	push MSG_SEC_BOOT
	call vga_text_print
	add sp, 2

_a20_sec:
.a20_chk:
	push es
	
	mov ax, 0x0000 
	mov es, ax
	mov bx, [es:0x7DFE] ; In MBR, 0x7DFE = 0x55, 0x7DFF = 0xAA

	not ax				; ax = 0xFFFF
	mov es, ax			; es = 0xFFFF0
	mov cx, [es:0x7E0E] ; cx 0xFFFF:0x7E0E => 0x107DFE

	pop es
	
	cmp bx, cx
	jne _load_kernel 


; If reaching at this line, A20 is disabled and you must enable this. 
.a20_enable:
	push A20_MSG               
    call vga_text_print         
    add sp, 2

_load_kernel:
	pusha
	
	mov ax, 0x1000 						; kernel base address of protected mode
	mov es, ax
	xor bx, bx

	mov word [total_read_sec], 1 + 4 			; 1 - Size of PBL , 4 - Size of SBL
	
	_disk_loop:
		; read the sectors 
		mov al, 1						; read one sector
		mov ah, BIOS_READ_SECS			; BIOS INT 13h F2h:Read Sectors from drive
		mov ch, byte [start_cylin]		; start to cylinder
		mov cl,	byte [start_sec]		; start to sector
		mov dh, byte [start_head]		; start to head
		mov dl, byte [drive_number]		; set disk drive to 0

		int 0x13						; request BIOS INT13h F2h
		jc _error						; If carry set, error
		
		cmp al, 1						; If successed to read, return the read sectors count to al
		jne _error	

		; count total sectors to read
		inc word [total_read_sec]

		; migration address
		mov ax, 512
		add bx, ax
		jnc _disk_update_chs
	
		; a carry happends		
		xor bx, bx
		mov ax, es
		add ax, 0x1000
		mov es, ax	
		cmp ax, 0x5000					; kernel size of protected model
		je _disk_loop_exit

		; increment the read sector count
		_disk_update_chs:
		inc word [start_sec]
		mov ax, [start_sec]
		mov dx, [secs]
		cmp ax, dx
		jle _disk_loop

		mov word [start_sec], 1

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
	push word [total_read_sec]
	jmp PMODE_ENTRY_POINT	; boot loader entry point of protected mode	

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

	; 32 Code Segment
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 10011010b
	db 11001111b
	db 0x00

	; 32 Data Segment
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 10010010b
	db 11001111b
	db 0x00

	; 64 Code Segment
    dw 0xFFFF       ; limit = 0
    dw 0x0000       ; base1[16] = 0
    db 0x00         ; base2[8] = 0
    db 0b10011010   ; P = 1, DPL[6:5] = 0, S = 1, TYPE[3:0] = 1010 
    db 0b10101111   ; G = 1, D/B = 0, L = 1, AVL = 0, LIMIT[3:0] = 0 
    db 0x00         ; base3[8] = 0
	
	; 64 Data Segment
    dw 0xFFFF       ; limit = 0
    dw 0x0000       ; base1[16] = 0
    db 0x00         ; base2[8] = 0
    db 0b10010010   ; P = 1, DPL[6:5] = 0, S = 1, TYPE[3:0] = 0010 
    db 0b11001111   ; G = 1, D/B = 1, L = 0, AVL = 0, LIMIT[3:0] = 0 
    db 0x00         ; base3[8] = 0

_gdtr:
	dw 0 
	dd 0

total_read_sec: dw 0
start_cylin :   dw 0
start_head	:   dw 0
start_sec	:   dw 0
cylins 		:   dw 0
heads 		:	dw 0
secs		: 	dw 0
vga_rows	: 	dw 0
drive_number: 	dw 0

MSG_SEC_BOOT: 	db 'YohdaOS Secondary Boot Loader Start', 0
A20_MSG:		db 'For entering to protected mode, preparing for the Gate-A20', 0
GDT_MSG:		db 'Start preparing for GDT of protected mode', 0

size equ $ - entry
times ((512*4) - size) nop
%ifndef P32
times (512*128) db 0x44 ; 0x10000 ~ 0x20000 for test for 32-bit
times (512*128) db 0x55 ; 0x20000 ~ 0x30000 for test for 32-bit
times (512*128) db 0x66 ; 0x30000 ~ 0x40000 for test for 32-bit
times (512*128) db 0x77 ; 0x40000 ~ 0x50000 for test for 32-bit
%endif

%endif 