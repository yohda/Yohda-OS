bits 16

extern main

; VGA 
VGA_TEST_BASE equ 0xB800
VGA_LINE_BYTES equ 160

SECTION .text
jmp 0x0000:.sbl_start

.sbl_start:
	pop ax
	mov word [vga_rows], ax

	push MSG_SEC_BOOT
	call vga_text_print
	add sp, 2

.a20_sec:
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
	jne .gdt_sec 

.a20_enable:
	push A20_MSG               
    call vga_text_print         
    add sp, 2

.gdt_sec:
	push GDT_MSG               
    call vga_text_print         
    add sp, 2

.gdt_tbl:
	; NULL Segment
	dw 0x0000, 0x0000, 0x0000, 0x0000

	; Code Segment
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0b10011010
	db 0b11001111
	db 0x00

	; Data Segment
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0b10010010
	db 0b11001111
	db 0x00

.gdtr:
	dw 0
	dd 0

.gdt_load:
	xor eax, eax
	mov ax, ds
	shl eax, 4
	add eax, .gdt_tbl
	mov [.gdtr+2], eax
	mov eax, .gdtr
	sub eax, .gdt_tbl
	mov [.gdtr], ax
	lgdt [.gdtr]

	jmp $

.pmode:	
	mov eax, cr0
	or al, 1
	mov cr0, eax	
	
	jmp 0x08:.pmain

[BITS 32]

.pmain:
	xor ax, ax

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	call main
	
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

vga_rows	: 	dw 0
MSG_SEC_BOOT: 	db 'YohdaOS Secondary Boot Loader Start', 0
A20_MSG:		db 'For entering to protected mode, preparing for the Gate-A20', 0
GDT_MSG:		db 'Start preparing for GDT of protected mode', 0

times 512 - ($-$$) db 0 
