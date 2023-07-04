%ifndef _CPUID_ASM_
%define _CPUID_ASM_

section .text

CPUID_STD_BASE_CMD equ 0x00000000
CPUID_STD_PROC_INFO_CMD equ (CPUID_STD_BASE_CMD + 1)
CPUID_STD_PAGE_4MB		equ 1<<3

CPUID_EXT_BASE_CMD equ 0x80000000
CPUOD_EXT_PROC_INFO_CMD equ (CPUID_EXT_BASE_CMD + 1)
CPUID_EXT_LONG_MODE 	equ 1<<29
CPUID_EXT_PAGE_1GB 		equ	1<<26

global cpu_init, cli, sti, mode64

cpu_init:
	push ebp		; cpuid_check_valid start
	mov ebp, esp	

_cpuid_is_supported:
	push eax
	push ecx
	
	pushfd
	pop eax

	mov ecx, eax	
	xor eax, 1<<21		

	push eax
	popfd

	pushfd
	pop eax

	cmp eax, ecx
	je .cpuid_no_cpuid
		mov eax, 1 	
		jmp .end

	.cpuid_no_cpuid:
		mov eax, 0 	
		jmp _end

	.end:
		mov dword [CPUID], eax		
	
	pop ecx
	pop eax	

;;;;;;;;;; Basic CPUID
_cpuid_basic_check_std_func:
	push eax

	mov eax, CPUID_STD_BASE_CMD
	cpuid
	
	cmp eax, CPUID_STD_BASE_CMD
	jbe .cpuid_no_std 		
	
	and eax, 0xFF
	jmp .end
	
	.cpuid_no_std:
		mov eax, 0

	.end:
		mov dword [CPUID_STD_FUNCS], eax
	
	pop eax

_cpuid_basic_4mb_page:
	push eax

	mov eax, CPUID_STD_PROC_INFO_CMD
	cpuid

	and edx, CPUID_STD_PAGE_4MB
	cmp edx, 0 
	je .cpuid_no_4mb_page
	
	mov [CPUID_PAGE_4MB], dword 1
	jmp .end

	.cpuid_no_4mb_page:
	mov [CPUID_PAGE_4MB], dword 0

	.end:		

	pop eax

;;;;;;;;;; Extended CPUID
_cpuid_ext_x64:
	push eax

	mov eax, CPUID_EXT_BASE_CMD
	cpuid
	
	cmp eax, CPUID_EXT_BASE_CMD
	jbe .nok
		and eax, 0xFF
		mov dword [CPUID_EXT_FUNCS], eax

		mov eax, CPUOD_EXT_PROC_INFO_CMD	; call extended function(0x8000_0001) to get whether or not is supported long mode
		cpuid

		and edx, CPUID_EXT_LONG_MODE		; long mode bit 29
		cmp edx, 0							; compare the result and 0. If the result compared is zero, long mode is not supported. 
		je .nok
		mov eax, 1	
		jmp .ok

	.nok:
		mov eax, 0

	.ok:
		mov dword [CPUID_64], eax
		
	pop eax

_cpuid_ext_page_1gb:
	push eax

	mov eax, CPUOD_EXT_PROC_INFO_CMD	; call extended function(0x8000_0001) to get whether or not is supported long mode
	cpuid

	and edx, CPUID_EXT_PAGE_1GB		; page 1GB bit 26
	cmp edx, 0
	je .nok
		mov eax, 1	
		jmp .ok

	.nok:
		mov eax, 0

	.ok:
		mov dword [CPUID_PAGE_1GB], eax
		
	pop eax

_end:
	pop ebp ; cpuid_check_valid end
	ret

;;;;;; interrupt
cli:
	push ebp
	mov ebp, esp
	
	cli

	pop ebp
	ret 

sti:
	push ebp
	mov ebp, esp
	
	sti

	pop ebp
	ret

;;;;;; mode change to 64
mode64:
	cli
	jmp 0x100000


;;;;;; data section
section .data
global CPUID, CPUID_64, CPUID_EXT_FUNCS, CPUID_PAGE_4MB, CPUID_PAGE_1GB, CPUID_STD_FUNCS

CPUID:					dd 0
CPUID_64:				dd 0
CPUID_EXT_FUNCS:		dd 0
CPUID_PAGE_4MB:			dd 0
CPUID_PAGE_1GB:			dd 0
CPUID_STD_FUNCS: 		dd 0

%endif
