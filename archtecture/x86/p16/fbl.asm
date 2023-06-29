%ifndef _FBL_ASM_
%define _FBL_ASM_
[BITS 16] 

; VGA 
VGA_TEST_BASE equ 0xB800
VGA_LINE_BYTES equ 160

; Memory
STACK_TOP equ 0x7C00 

; BIOS INT
BIOS_READ_SECS		equ 0x02
BIOS_READ_DISK_INFO	equ 0x08

; Disk
DISK_SBL_SECS equ 4

extern _sbl_start
global vga_rows

SECTION .text      
jmp 0x0000:_fbl_start

_fbl_start:
	;cli						; Disable interrupt	

	mov byte [drive_number], dl	; set drive number

    xor ax, ax			
    mov ds, ax				; set DS to zero
    mov ax, VGA_TEST_BASE 	 
    mov es, ax      		; set ES to VGA base address

	; ss, bp and sp work in and out with all together. So, as sonn as you set SS, then right now set bp and sp. there is dangerous component due to interrupt.
    xor ax, ax 				 
    mov ss, ax  			   
    mov bp, STACK_TOP 		
    mov sp, bp  

    xor si, si                            

_vga_test_init:                   
    mov word [es:si], 0x0F00       
                                    
    add si, 2              
    cmp si, VGA_LINE_BYTES*25     
                            
    jne _vga_test_init      

_part_check:
	push es
	push di
	
	mov ax, 0x07C0
	mov es, ax
	mov di, 0x1BE				; 0x7BBE, 0x7DBE, 0x7EBE, 0x7FBE = partition table

	.part_loop:
		mov al, byte [es:di] 
		cmp al, 0x08			; 0x08 = active partition

		je _part_read  	

		add di, 0x10			; Inspect next partition entry 

		cmp di, 0x1FE			; if last partitio entry, break
		jne .part_loop
	
		pop di
		pop es
	
		jmp _disk_get_infos

_part_read:
	push es
	push di

	; To-do

	pop di
	pop es	

_disk_get_infos:
	pusha
	
	mov ah, BIOS_READ_DISK_INFO	; BIOS INT 13h F8h:Read Drive Parameteres
	int 0x13	
	jc _error					; if carry set, error

	mov [secs], cx				; cx[5:0] - sectors per track
	and word [secs], 0x003F		; calculate sectors per a track	

	mov [cylins], cx			; cx[15:6] - number of cylinders
	and word [cylins], 0xFFC0	; calculate cylinders
	shr word [cylins], 6
	inc word [cylins]			; index of cylinders start = number - 1
		
	mov byte [heads], dh 		; dh - number of head 
	inc word [heads] 			; index of head starts = number - 1

	mov ax, [secs]
	cmp ax, 8
	jl _disk_not_supported
	
	popa

	;jmp _disk_read

_disk_get_ext_infos:
	pusha

	mov ah, 0x41	; BIOS INT=13h AH=41h - Check if extedned function is supported.
	mov dl, byte [drive_number]	
	mov bx, 0x55aa

	int 0x13
	jc _disk_read 	; If carry flag is set, extended function is not supported.
	
	mov ah, 0x48	; BIOS INT13h AH=48h - extended read drive parameters
	mov dl, byte [drive_number]	

	xor si, si
	mov cx, 0x0900
	mov ds, cx
	
	int 0x13
	jc _disk_read
	
		
	popa

_disk_read:
	pusha
	
	xor bx, bx
	mov ds, bx
	mov ax, 0x07E0
	mov es, ax

	mov al, 4	
	mov ah, BIOS_READ_SECS	; BIOS INT 13h F2h:Read Sectors from drive
	mov ch, 0				; start to cylinder
	mov cl,	2				; start to sector
	mov dh, 0 				; start to head
	mov dl, byte [drive_number]	; floppy0 = 0x00, hdd0 = 0x80
	
	mov word [sbl_start_sec], 6 
	mov word [sbl_start_head], 0
	mov word [sbl_start_cylin], 0

	int 0x13				; request BIOS INT13h F2h
	jc _error				; if carry set, error

	popa
	
_sbl:
	push word DISK_SBL_SECS+1	; total sectors to read in fbl
	push word [drive_number]	; pass drive number 
	push word [vga_rows] 		; pass command line number for sbl
	push word [secs]			; pass sectors per track
	push word [heads]			; pass heads
	push word [cylins]			; pass cylinders
	push word [sbl_start_sec] 	; pass start sector
	push word [sbl_start_head]  ; pass start head
	push word [sbl_start_cylin] ; pass satrt cylinder

	jmp 0x0000:_sbl_start 
	
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

	ret 2

_disk_not_supported:
	push DISK_ERR_MSG               
    call vga_text_print

_error:
	jmp $ 

; disk
drive_number 	: dw 0
secs			: dw 0
heads			: dw 0
cylins			: dw 0
sbl_start_sec 	: dw 0
sbl_start_head 	: dw 0
sbl_start_cylin : dw 0

; print
vga_rows	: 	dw 0
DISK_ERR_MSG:	db 'err0', 0

times (446 - ($-$$)) db 0
times 16 db 0
times 16 db 0
times 16 db 0
times 16 db 0

dw 0xAA55     
%endif
