%ifndef _PAGING_NASM_
%define _PAGING_NASM_

[BITS 32]

extern higher_half_start

MB_MAGIC_YOHDA_OS	equ 0x3f221d73
MB_MAGIC_MACH_STATE equ 0x36d76289

%ifdef GRUB
section .multiboot_header write
MB_TAG_TYPE_ADDRESS equ 0x2
MB_TAG_TYPE_ENTRY	equ 0x3

MULTIBOOT2_MAGIC equ 0xE85250D6
MULTIBOOT2_ARCH_I386 equ 0
MULTIBOOT2_ARCH_MIPS equ 4

align 8
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
%endif ; GRUB

section .text.boot exec
global _start

_start: 
	mov ecx, cr4
	or ecx, 1<<4 ; PSE : Need to use 4MB in protected mode
	mov cr4, ecx

	mov ecx, pdes
	mov cr3, ecx	

	mov ecx, cr0
	or ecx, 1<<31
	mov cr0, ecx

	jmp higher_half_start

section .data.boot
align 4096
global pdes ; in later, main will use them
pdes:
	dd 0x00000083 ; Identify Mapped [0x000000:0x400000]
	times 767 dd 0x00000000; 
	dd 0x00000083 ;Identify Mapped [0xC0000000:0xC0400000]
	times 255 dd 0x00000000
;%assign i 0
;%rep 256
;	dd 0x00000083 + i
;%assign i i+0x400000
;%endrep

%endif
