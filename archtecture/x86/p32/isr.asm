%ifndef _ISR_NASM_
%define _ISR_NASM_

[BITS 32]

extern isr_division_error_handler, isr_common_handler, isr_non_maskable_interrupt_handler, isr_general_protection_fault_handler, isr_page_fault_handler, isr_system_timer_handler, isr_ps2_keyboard_interrupt_handler, syscall_dispatch_handler

global isr_division_error, isr_debug, isr_non_maskable_interrupt, isr_breakpoint, isr_overflow, isr_bound_range_exceeded, isr_invalid_opcode, isr_device_not_available, isr_double_fault, isr_invalid_tss, isr_segment_not_present, isr_stack_segment_fault, isr_general_protection_fault, isr_page_fault, isr_x87_floating_point_exception, isr_alignment_check, isr_machine_check, isr_simd_floating_point_exception, isr_virtualization_exception, isr_system_timer_interrupt, isr_ps2_keyboard_interrupt, isr_syscall

align 4
isr_division_error:
	cli
	pushad
	mov ebp, esp
	cld	

	push 0	
	push 0
	call isr_division_error_handler
	add esp, 8

	popad
	sti
	iret

isr_debug:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 1
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_non_maskable_interrupt:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 2
	call isr_non_maskable_interrupt_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_breakpoint:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 3
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_overflow:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 4
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_bound_range_exceeded:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 5
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_invalid_opcode:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 6
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_device_not_available:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 7
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_double_fault:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 8
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_invalid_tss:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 10
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_segment_not_present:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 11
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector
	
	popad
	sti
	iret

isr_stack_segment_fault:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 12
	call isr_common_handler
	
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_general_protection_fault:
	cli
	cld	
	push 13	  ; push irq
	pushad	  ; push GPRs

	push gs	  
	push fs
	push es
	push ds
	
	call isr_general_protection_fault_handler

	pop ds
	pop es
	pop fs
	pop gs

	popad
	
	; You should be careful the location of this code. this code will be executed after GPR`s and Segments registers was poped.
	add esp, 8 ; pop error code, interrupt vector
	
	sti
	iret

isr_page_fault:
	cli
	cld	

	push 14
	pushad

	push gs
	push fs
	push es
	push ds
	
	call isr_page_fault_handler

	pop ds
	pop es
	pop fs
	pop gs

	popad
	
	; You should be careful the location of this code. this code will be executed after GPR`s and Segments registers was poped.
	add esp, 8 ; pop error code, interrupt vector
	
	sti		   ; re-enable interrupt
	iret

isr_x87_floating_point_exception:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 16
	call isr_common_handler
	add esp, 8 ; error code + interrupt vector

	popad
	sti
	iret

isr_alignment_check:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 17
	call isr_common_handler
	add esp, 8

	popad
	sti
	iret

isr_machine_check:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 18
	call isr_common_handler
	add esp, 8

	popad
	sti
	iret

isr_simd_floating_point_exception:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 19
	call isr_common_handler
	add esp, 8

	popad
	sti
	iret

isr_virtualization_exception:
	cli
	pushad
	mov ebp, esp
	cld	
	
	push 0	
	push 20
	call isr_common_handler
	add esp, 8

	popad
	sti
	iret

;;;;;;; interrupt ;;;;;;;;;
isr_system_timer_interrupt:
	pushad
	mov ebp, esp
	cld	
	
	push 32
	call isr_system_timer_handler
	add esp, 4

	popad
	iret

isr_ps2_keyboard_interrupt:
	pushad
	mov ebp, esp
	cld	
	
	push 33
	call isr_ps2_keyboard_interrupt_handler
	add esp, 4

	popad
	iret

isr_syscall:
	pushad
	mov ebp, esp
	cld	
	
	push 80
	call isr_ps2_keyboard_interrupt_handler
	add esp, 4

	popad
	iret

%endif
