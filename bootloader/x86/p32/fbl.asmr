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

extern _sbl_start
global vga_rows

SECTION .text      
jmp 0x0000:_fbl_start

_fbl_start:
	cli						; Disable interrupt	

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
	 
    push BOOT_MSG               
    call vga_text_print         

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
	push ACT_PART_MSG                
    call vga_text_print         
	
	push es
	push di

	; To-do

	pop di
	pop es	

_disk_get_infos:
	pushad
	
	mov ah, BIOS_READ_DISK_INFO	; BIOS INT 13h F8h:Read Drive Parameteres
	int 0x13	
	jc _error					; if carry set, error

	mov [secs], cx				; calculate sectors per a track
	and word [secs], 0x003F		; cx[5:0] - sectors per track 	
	
	popad

_disk_read:
	pushad
	
	mov ax, 0x07E0
	mov es, ax
	xor bx, bx
	
	mov ah, BIOS_READ_SECS	; BIOS INT 13h F2h:Read Sectors from drive
	dec word [secs]		; because start sector number 2
	mov	al, [secs]	
	mov ch, 0		; start to cylinder
	mov cl,	2		; start to sector
	mov dh, 0 		; start to head

	int 0x13		; request BIOS INT13h F2h
	jc _error		; if carry set, error

	popad
	
_sbl:
	push SEC_BOOT_MSG               
    call vga_text_print         

	push word [vga_rows] 	; pass command line number for sbl

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

_error:
	jmp $ 

; disk
secs: 	dw 0

; print
vga_rows	: 	dw 0
SEC_BOOT_MSG:	db 'Ready for jumping secondary bootloader', 0 	
BOOT_MSG:    	db 'YohdaOS First Boot Loader Start', 0 
ACT_PART_MSG:	db 'There exist a active partition', 0
ERR_MSG:		db 'Happned the error. So, stop the process...', 0

times (446 - ($-$$)) db 0
times 16 db 0
times 16 db 0
times 16 db 0
times 16 db 0

dw 0xAA55     
%endif
