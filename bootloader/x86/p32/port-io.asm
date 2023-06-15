%ifndef _PORT_IO_
%define _PORT_IO_

SECTION .text
global inb, inw, ind, outb, outw, outd

; All funcions in I/O Port of input are same with parameters
; First argument - I/O port address
inb:
	push ebp
	mov ebp, esp
	push edx

	mov dx, word [ebp+8]	
	in al, dx
	
	pop edx
	pop ebp
	ret 

inw:
	push ebp
	mov ebp, esp
	push edx

	mov dx, word [ebp+8]
	in ax, dx

	pop edx
	pop ebp
	ret 

ind:
	push ebp
	mov ebp, esp
	push edx

	mov dx, word [ebp+8]
	in eax, dx
	
	pop edx
	pop ebp
	ret 

; All funcions in I/O Port of output are same with parameters
; First argument - I/O port address
; Second argument - value to write 
outb:
	push ebp
	mov ebp, esp
	push edx

	mov dx, word [ebp+8]	
	mov eax, [ebp+12]

	out dx, al
	
	pop edx
	pop ebp
	ret
	
outw:
	push ebp
	mov ebp, esp
	push edx

	mov dx, [ebp+8]
	mov eax, [ebp+12]

	out dx, ax

	pop edx
	pop ebp
	ret 

outd:
	push ebp
	mov ebp, esp
	push edx

	mov dx, word [ebp+8]
	mov eax, [ebp+12]	

	out dx, eax	

	pop edx
	pop ebp
	ret

%endif
