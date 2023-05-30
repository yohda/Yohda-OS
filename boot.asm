; boot.asm
	;cli

bits 16

_start:
	cli
	hlt
	
	times 510-($-$$) db 0
	dw 0xAA55
