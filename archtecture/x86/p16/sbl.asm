%ifndef _SBL_NASM_
%define _SBL_NASM_

[BITS 16]

entry:

; Multiboot2 BOOT Information
MM_MAGIC equ 0x36d76289
MBI_BUF_SIZE equ 24 ; Multiboot2 Memory Map MIN size 24B, But, BIOS Memory map MIN size = 20B. But, last 4 bytes are reserved. So, set to 24 bytes
MBI_BASE equ 0x9000
MBI_OST_MM equ 8 	; type#6
; In yohdaOS, first boot information is `Memory map`. So, this offset is in 8 bytes(total_size + reserved)
; If you want to add a new boot information, added new offset of boot information in below.
; For example, if you want to add the `boot loader name(type#2), refer to below codes.
; MBI_OST_BNAME equ MBI_OST_MM + 24(size of Mempry map information in bytes is 20B. But, boot information is alinged 8 bytes. Then, added the 4 bytes padding after that)


; Memory Map(Type#6)
MBI_MM_EBASE	equ 0x9200

; Memory Map
MM_SIG equ 0x534d4150
BIOS_MM_MIN_BUF_SIZE equ 20

; 32-bt Kernel meta data
PMODE_ENTRY_POINT equ 0x100000
BIOS_READ_SECS	equ 0x02

; VGA 
VGA_TEST_BASE 	equ 0xB800
VGA_LINE_BYTES	equ 160

SECTION .text
jmp 0x0000:_sbl_start

global _sbl_start

; Okay, here is now secondary boot having two something to do.
; First, Change the operating mode from real mode to unreal mode. after that, from unreal mode to Protected mode.
; Second, Load the 32bit kernel into above 1MB.
_sbl_start:
	pop word [start_cylin]
	pop word [start_head]
	pop word [start_sec]
	pop word [cylins]
	pop word [heads]
	pop word [secs]
	pop word [vga_rows]
	pop word [drive_number]
	pop word [total_read_sec]

	mov word [0x34], __isr_gp ; #GP
	mov word [0x36], 0x00
;
;#define PIC1_CMD                    0x20
;#define PIC_READ_IRR                0x0a    
;
;	mov dx, 0x20
;	mov al, 0x0a
;	out dx, al
;
;	in al, dx

	mov [total_size], word 8 ; 8 = basic tags structure

	push MSG_SEC_BOOT
	call vga_text_print
	add sp, 2

; BIOS 15h AX=0xE820
_mb_detect_mm:
	clc	; clear carry flag
	pusha
	
	mov [MBI_BASE+MBI_OST_MM], word 6				 ; type
	mov [MBI_BASE+MBI_OST_MM+4], word 16			 ; size
	mov [MBI_BASE+MBI_OST_MM+8], word 24			 ; entry size
	mov [MBI_BASE+MBI_OST_MM+12], word 0	 		 ; entry version
	;mov [MBI_BASE+MBI_OST_MM+16], word MBI_MM_EBASE  ; base address
	
	xor ebx, ebx
	xor ecx, ecx
	mov es, ebx
	mov di, MBI_BASE+MBI_OST_MM+16
	.detect_mm_loop_start:
		xor eax, eax
		xor ecx, ecx
		
		mov edx, MM_SIG
		mov eax, 0xe820	
		mov ecx, MBI_BUF_SIZE

		int 0x15
		jc _error

		cmp eax, MM_SIG 	
		jne _error

		cmp ecx, BIOS_MM_MIN_BUF_SIZE
		jl _error	

		add word [MBI_BASE+MBI_OST_MM+4], MBI_BUF_SIZE	; size	
		cmp ebx, 0 	; If return value is zero, the value is last descriptor.
		je .detect_mm_loop_end

		add di, MBI_BUF_SIZE
		jmp .detect_mm_loop_start	

	.detect_mm_loop_end:	
	mov ax, word [MBI_BASE+MBI_OST_MM+4]	
	add word [total_size], ax ; added memory map size to total_size
	
	popa

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; If you want to add a new boot inforamtion, added that here!!! Never define the other place!!!
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_mb_end:
	pusha

	mov bx, [total_size]
	mov [MBI_BASE], bx 
	
	; Added End Tag
	mov [MBI_BASE+bx], word 0
	mov [MBI_BASE+bx+4], word 8	

	add bx, 8 ; 8 = terminating 8 bytes
	mov [total_size], bx 
	mov [MBI_BASE], bx 

	popa

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
	;jne _load_kernel 
	jne _gdt_sec

; When reached at this line, A20 is disabled and you must enable this before entering the protected mode. 
.a20_enable:
	push A20_MSG               
    call vga_text_print         
    add sp, 2

; From here, preapre for GDT used in protected mode
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

; From here, switch from real mode to unreal mode
_pre_unreal:    
    mov eax, cr0
    or al, 1
    mov cr0, eax    

_do_unreal:
    mov ax, GDT32_DATA        
    mov ds, ax              ; Load 32bit Date Segment Discriptor

    and al, 0xFE        
    mov cr0, eax            ; Disable Protected Mode 

_unreal_mode:
    xor ax, ax 
    mov ds, ax
    mov es, ax
    
    mov ecx, 0x100008
    mov dword [ds:ecx], 0x99aabbcc

; Fron now on, Load the 32bit kernel into 1MB. 
_load_kernel:
	pusha

	mov ax, 0x1000  					; 32bit kernel base address = 0x100000 
	mov es, ax
	xor bx, bx
		
	mov word [total_read_sec], 1 + 4 	; 1 - Size of PBL , 4 - Size of SBL
	
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
		inc word [total_read_sec] ; increment the count to read sector
		dec word [kern32_secs]    ; decrement the count of kernel32 sectors

   		mov eax, cr0
    	or al, 1
    	mov cr0, eax    

   		mov ax, GDT32_DATA        
    	mov ds, ax              ; Load 32bit Date Segment Discriptor

   		and al, 0xFE        
    	mov cr0, eax            ; Disable Protected Mode 

    	xor ax, ax 
    	mov ds, ax
    	mov es, ax
 
		push es
		xor dx, dx
		mov es, dx
	
		mov edi, [kern32_entry_point]
		mov esi, 0x10000
		mov ecx, 0x200 ; This macro was inserted from building 
		rep movsb

		mov eax, 0x100000
		mov dword [eax], 0xffeedd22

		pop es

		mov dword [kern32_entry_point], edi  
		
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

		; check if disk loop exit
		cmp word [kern32_secs], 0
		je _disk_loop_exit

		jl _disk_loop

		jmp _error

	_disk_loop_exit:
	popa

; From here, start to get into protected mode.
_pre_pmode:	
	mov eax, cr0
	or al, 1
	mov cr0, eax	

	jmp GDT32_CODE:_penter

[bits 32]
_penter:
	; In this area, skip to set ss because for setting properly the stack position, we must to know system memery-map. So, the reposibility of setting stack delegates the boot-lodaer in protected mode.
	mov ax, GDT32_DATA		
	mov ds, ax				; Load 32bit Date Segment Discriptor
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov edi, PMODE_ENTRY_POINT
	mov esi, 0x10000
	;mov ecx, KERN32_SIZE ; This macro was inserted from building 
	rep movsb 

	; for compatibility of multiboot2
	mov eax, MM_MAGIC   ; Yohda OS
	mov ebx, 0x9000		; Machien State - Memory map
	mov cx, GDT32_DATA  ;  

	; below codes for protected mode
	push word [total_read_sec]
	jmp PMODE_ENTRY_POINT	; boot loader entry point of protected mode	

[bits 16]
;;; function sections
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

__isr_gp:
	push MSG_SEC_BOOT
	call vga_text_print
	add sp, 2

	jmp $	
	

_gdt_tbl:
; NULL Segment
_gdt_null_desp:
	dw 0x0000
	dw 0x0000
	dw 0x0000
	dw 0x0000

; 32 Code Segment
_gdt32_code_desp:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0b10011010
	db 0b11001111
	db 0x00

; 32 Data Segment
_gdt32_data_desp:
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 0b10010010
	db 0b11001111
	db 0x00

; 64 Code Segment
_gdt16_code_desp:
    dw 0xFFFF       ; limit = 0
    dw 0x0000       ; base1[16] = 0
    db 0x00         ; base2[8] = 0
    db 0b10011010   ; P = 1, DPL[6:5] = 0, S = 1, TYPE[3:0] = 1010 
    db 0b00000000   ; G = 0, D/B = 0(0:16bit, 1:32bit), L = 0, AVL = 0, LIMIT[3:0] = 0 
    db 0x00         ; base3[8] = 0
	
; 64 Data Segment
_gdt16_data_desp:
    dw 0xFFFF       ; limit = 0
    dw 0x0000       ; base1[16] = 0
    db 0x00         ; base2[8] = 0
    db 0b10010010   ; P = 1, DPL[6:5] = 0, S = 1, TYPE[3:0] = 0010 
    db 0b00000000   ; G = 0, D/B = 0(0:16bit, 1:32bit), L = 0, AVL = 0, LIMIT[3:0] = 0 
    db 0x00         ; base3[8] = 0

_gdtr:
	dw 0 
	dd 0

GDT32_CODE equ _gdt32_code_desp - _gdt_tbl
GDT32_DATA equ _gdt32_data_desp - _gdt_tbl
GDT16_CODE equ _gdt16_code_desp - _gdt_tbl
GDT16_DATA equ _gdt16_data_desp - _gdt_tbl

; MM
total_size  : 	dw 0

; disk
kern32_entry_point : dd 0x00100000
kern32_secs : 	dw KERN32_SECS
total_read_sec: dw 0
start_cylin :   dw 0
start_head	:   dw 0
start_sec	:   dw 0
cylins 		:   dw 0
heads 		:	dw 0
secs		: 	dw 0
drive_number: 	dw 0

; vga
vga_rows	: 	dw 0

MSG_SEC_BOOT: 	db 'YohdaOS Secondary Boot Loader Start', 0
A20_MSG:		db 'For entering to protected mode, preparing for the Gate-A20', 0
GDT_MSG:		db 'Start preparing for GDT of protected mode', 0

size equ $ - entry
times ((512*4) - size) nop
%ifndef P32
times KERN32_SECS*512 db 0x44
%endif

%endif 
