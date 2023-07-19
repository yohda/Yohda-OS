%ifndef _TASK_ASM_
%define _TASK_ASM_

section .text
global context_switch, swtich_user_to_kern, switch_kern_to_user

context_switch:
	mov eax, [esp+4] ; old context
	mov ecx, [esp+8] ; new context

	push edi
	push esi
	push ebx
	push ebp		

	mov [eax], esp
	mov esp, ecx
	
	pop ebp
	pop ebx
	pop esi
	pop edi	

	ret

switch_kern_to_user:
	

switch_user_to_kern:


%endif<F5>
