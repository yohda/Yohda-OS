%ifndef DEBUG
org 0x7C00       
bits 16   
%endif

%ifndef STACK_TOP
STACK_TOP equ 0x07C0
%endif

VGA_TEST_BASE equ 0xB800
VGA_LINE_BYTES equ 160

SECTION .text      
jmp 0x0000:.start

.start:
	cli	

    xor ax, ax 
    mov ds, ax      
    mov ax, VGA_TEST_BASE 
    mov es, ax      

    mov ax, 0x0000  
    mov ss, ax      
    mov bp, 0x07C0;STACK_TOP 
    mov sp, bp  

    xor si, si                            

.vga_test_init:                   
    mov word [es:si], 0x0F00       
                                    
    add si, 2              
    cmp si, VGA_LINE_BYTES*25     
                            
    jne .vga_test_init      
	 
    push BOOT_MSG               
    push 0                      
    call vga_text_print         
    add  sp, 4                  

	jmp $	

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
	mul word [bp+4]	
	mov di, ax	

	mov si, [bp+6]	

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

BOOT_MSG:    db 'yohdaOS 16-bit Boot Loader', 0 

times 510 - ($-$$) db 0
dw 0xAA55     
