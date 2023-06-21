%ifndef _CPUID_ASM_
%define _CPUID_ASM_

section .text

CPUID_STD_BASE_CMD equ 0x00000000
CPUID_EXT_BASE_CMD equ 0x80000000
CPUID_EXT_LM equ 1<<29

global cpuid_check_valid

cpuid_check_valid:
	push ebp		; cpuid_check_valid stat
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

	.end:
		mov dword [CPUID_IS_VALID], eax		
	
	pop ecx
	pop eax	

_cpuid_check_std_func:
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

_cpuid_x64_is_supported:
	push eax

	mov eax, CPUID_EXT_BASE_CMD
	cpuid
	
	cmp eax, CPUID_EXT_BASE_CMD
	jbe .cpuid_no_x64
		and eax, 0xFF
		mov dword [CPUID_EXT_FUNCS], eax

		mov eax, 0x80000001					; call extended function(0x8000_0001) to get whether or not is supported long mode
		cpuid

		and edx, CPUID_EXT_LM				; long mode bit 29
		cmp edx, 0							; compare the result and 0. If the result compared is zero, long mode is not supported. 
		je .cpuid_no_x64
		mov eax, 1	
		jmp .end

	.cpuid_no_x64:
		mov eax, 0

	.end:
		mov dword [CPUID_X64], eax
		
	pop eax

	pop ebp ; cpuid_check_valid stat 
	ret

section .data
global CPUID_IS_VALID, CPUID_X64, CPUID_EXT_FUNCS, CPUID_STD_FUNCS

CPUID_IS_VALID:			dd 0
CPUID_X64:				dd 0
CPUID_EXT_FUNCS:		dd 0
CPUID_STD_FUNCS: 		dd 0

%endif
