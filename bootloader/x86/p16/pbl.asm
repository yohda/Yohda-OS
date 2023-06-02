%ifndef DEBUG
org 0x7C00       
bits 16   
%endif

; VGA 
VGA_TEST_BASE equ 0xB800
VGA_LINE_BYTES equ 160

; Disk
DISK_FRT_SECS equ 2
DISK_READ_SECS equ 32

; Memory
MEM_MBR_ADDR equ 0x07C0
STACK_TOP equ MEM_MBR_ADDR
MEM_PAR_OST equ 0x1BE
MEM_SEC_BOOT_ADDR equ 0x07E0 

global vga_rows

SECTION .text      
jmp 0x0000:.start

.start:
	cli	

    xor ax, ax
    mov ds, ax
	mov [vga_rows], ax      
    mov ax, VGA_TEST_BASE 
    mov es, ax      

    mov ax, 0x0000  
    mov ss, ax      
    mov bp, STACK_TOP 
    mov sp, bp  

    xor si, si                            

.vga_test_init:                   
    mov word [es:si], 0x0F00       
                                    
    add si, 2              
    cmp si, VGA_LINE_BYTES*25     
                            
    jne .vga_test_init      
	 
    push BOOT_MSG               
    call vga_text_print         
    add  sp, 2                 

.part_check:
	push es
	push di
	
	mov ax, MEM_MBR_ADDR
	mov es, ax
	mov di, MEM_PAR_OST

.part_loop:
	mov al, byte [es:di] 
	cmp al, 0x08

	je .part_read  	

	add di, 0x10

	cmp di, 0x1FE 
	jne .part_loop
	
	pop di
	pop es
	
	jmp .disk_sec

.part_read:
	push ACT_PART_MSG                
    call vga_text_print         
    add  sp, 2
	
	push es
	push di

	; To-do

	pop di
	pop es	

.disk_sec:
	push es
	
	mov ax, MEM_SEC_BOOT_ADDR
	mov es, ax
	xor bx, bx

	mov ah, 2
	mov al, DISK_READ_SECS
	mov ch, 0
	mov cl, DISK_FRT_SECS
	mov dh, 0
	
	int 0x13	

	pop es
	
	cmp al, DISK_READ_SECS
	je .sbl

	; Something is needed to be writen shiled code here.

.sbl:
	push SEC_BOOT_MSG               
    call vga_text_print         
    add  sp, 2

	push word [vga_rows] 
	jmp 0x07E0:0x0000 
	
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
ACT_PART_MSG:	db 'There exist a active partition', 0
SEC_BOOT_MSG:	db 'Ready for jumping secondary bootloader', 0 	
BOOT_MSG:    	db 'YohdaOS Primary Boot Loader Start', 0 

times (446 - ($-$$)) nop
times 16 db 0
times 16 db 0
times 16 db 0
times 16 db 0

dw 0xAA55     
