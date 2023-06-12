%ifndef _ISR_NASM_
%define _ISR_NASM_

[BITS 32]

extern isr_division_error_handler, isr_common_handler, isr_non_maskable_interrupt_handler, isr_general_protection_fault_handler, isr_page_fault_handler

global isr_division_error, isr_debug, isr_non_maskable_interrupt, isr_breakpoint, isr_overflow, isr_bound_range_exceeded, isr_invalid_opcode, isr_device_not_available, isr_double_fault, isr_invalid_tss, isr_segment_not_present, isr_stack_segment_fault, isr_general_protection_fault, isr_page_fault, isr_x87_floating_point_exception, isr_alignment_check, isr_machine_check, isr_simd_floating_point_exception, isr_virtualization_exception, isr_control_protection_exception, isr_hypervisor_injection_exception, isr_vmm_communication_exception, isr_security_exception, isr_triple_fault 

align 4

isr_division_error:
	pushad
	mov ebp, esp
	cld	
	
	push 0
	call isr_division_error_handler
	add esp, 4

	popad
	iret

isr_debug:
	pushad
	mov ebp, esp
	cld	
	
	push 1
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_non_maskable_interrupt:
	pushad
	mov ebp, esp
	cld	
	
	push 2
	call isr_non_maskable_interrupt_handler
	add esp, 4

	popad
	iret

isr_breakpoint:
	pushad
	mov ebp, esp
	cld	
	
	push 3
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_overflow:
	pushad
	mov ebp, esp
	cld	
	
	push 4
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_bound_range_exceeded:
	pushad
	mov ebp, esp
	cld	
	
	push 5
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_invalid_opcode:
	pushad
	mov ebp, esp
	cld	
	
	push 6
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_device_not_available:
	pushad
	mov ebp, esp
	cld	
	
	push 7
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_double_fault:
	pushad
	mov ebp, esp
	cld	
	
	push 8
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_invalid_tss:
	pushad
	mov ebp, esp
	cld	
	
	push 10
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_segment_not_present:
	pushad
	mov ebp, esp
	cld	
	
	push 11
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_stack_segment_fault:
	pushad
	mov ebp, esp
	cld	
	
	push 12
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_general_protection_fault:
	pushad
	mov ebp, esp
	cld	
	
	push 13
	call isr_general_protection_fault_handler
	add esp, 4

	popad
	iret

isr_page_fault:
	pushad
	mov ebp, esp
	cld	
	
	push 14
	call isr_page_fault_handler
	add esp, 4

	popad
	iret

isr_x87_floating_point_exception:
	pushad
	mov ebp, esp
	cld	
	
	push 16
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_alignment_check:
	pushad
	mov ebp, esp
	cld	
	
	push 17
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_machine_check:
	pushad
	mov ebp, esp
	cld	
	
	push 18
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_simd_floating_point_exception:
	pushad
	mov ebp, esp
	cld	
	
	push 19
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_virtualization_exception:
	pushad
	mov ebp, esp
	cld	
	
	push 20
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_control_protection_exception:
	pushad
	mov ebp, esp
	cld	
	
	push 21
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_hypervisor_injection_exception:
	pushad
	mov ebp, esp
	cld	
	
	push 28
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_vmm_communication_exception:
	pushad
	mov ebp, esp
	cld	
	
	push 29
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_security_exception:
	pushad
	mov ebp, esp
	cld	
	
	push 30
	call isr_common_handler
	add esp, 4

	popad
	iret

isr_triple_fault:
	pushad
	mov ebp, esp
	cld	
	
	push 32
	call isr_common_handler
	add esp, 4

	popad
	iret

%endif
