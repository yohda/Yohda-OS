%ifndef _PBL_NASM_
%define _PBL_NASM_

[BITS 32]

extern main

MB_TAG_TYPE_ADDRESS equ 0x2
MB_TAG_TYPE_ENTRY	equ 0x3

%ifdef MB
section .multiboot_header write
MULTIBOOT2_MAGIC equ 0xE85250D6
MULTIBOOT2_ARCH_I386 equ 0
MULTIBOOT2_ARCH_MIPS equ 4

;align 8
multiboot_header_start:
magic_fields_start:
	magic:			dd MULTIBOOT2_MAGIC 	; MAGIC NUMBER
	archtecture : 	dd MULTIBOOT2_ARCH_I386 ; i386 = 0 , MIPS = 4
	header_length:	dd multiboot_header_end - multiboot_header_start ; header length 
	checksum:		dd -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH_I386 + (multiboot_header_end - multiboot_header_start)) 
magic_fields_end:
address_tag_start:
	dw MB_TAG_TYPE_ADDRESS								; type
	dw 1												; flag
	dd address_tag_end - address_tag_start				; size
	dd multiboot_header_start							; header_addr
	;dd _start 											; load_addr
	dd multiboot_header_start
	dd 0												; load_end_addr
	dd 0												; bss_end_addr
address_tag_end:
entry_address_tag_start:
	dw MB_TAG_TYPE_ENTRY								; type
	dw 1												; flag
	dd entry_address_tag_end - entry_address_tag_start	; size
	dd _start											; entry_address
entry_address_tag_end:
multiboot_header_end:
%endif

section .text
global _start

_start: 
	cli
	
	xor ecx, ecx	; Get the count of sector to read
	pop ecx

	; After entering into protected mode, you just should load data segment into each data segments.
	;mov ax, 0x10 

	;mov ds, ax
	;mov es, ax
	;mov fs, ax
	;mov gs, ax

	; When you assign the stack segment, you should consider the interrupt. So, after set the ss, you just immediately have to assign the esp and ebp.
	;mov ss, ax
	;mov ebp, 0x80000	
	;mov esp, 0x80000

	push ecx
	call main
	jmp $				; Nevere come here

%endif
