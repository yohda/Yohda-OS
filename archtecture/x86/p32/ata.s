	.file	"ata.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.file 0 "/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/bootloader/x86/p32" "ata.c"
	.globl	ata
	.section	.bss
	.align 4
	.type	ata, @object
	.size	ata, 10
ata:
	.zero	10
	.text
	.globl	ata_sw_rst
	.type	ata_sw_rst, @function
ata_sw_rst:
.LFB0:
	.file 1 "ata.c"
	.loc 1 30 1
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$8, %esp
	.loc 1 31 2
	subl	$8, %esp
	pushl	$4
	pushl	$1014
	call	outb
	addl	$16, %esp
	.loc 1 32 2
	call	ata_is_ready
	.loc 1 34 2
	subl	$8, %esp
	pushl	$64
	pushl	$502
	call	outb
	addl	$16, %esp
	.loc 1 35 1
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	ata_sw_rst, .-ata_sw_rst
	.globl	ata_diag_disk
	.type	ata_diag_disk, @function
ata_diag_disk:
.LFB1:
	.loc 1 39 1
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	.loc 1 40 5
	movb	$-1, -13(%ebp)
	.loc 1 40 15
	movb	$0, -14(%ebp)
	.loc 1 40 23
	movb	$0, -15(%ebp)
.LBB2:
	.loc 1 43 5
	call	ata_is_ready
	.loc 1 43 4
	testl	%eax, %eax
	jns	.L3
	.loc 1 44 10
	movl	$-1, %eax
	jmp	.L4
.L3:
.LBE2:
	.loc 1 46 2
	subl	$8, %esp
	pushl	$144
	pushl	$503
	call	outb
	addl	$16, %esp
	.loc 1 47 7
	movl	$0, -12(%ebp)
	.loc 1 47 2
	jmp	.L5
.L6:
	.loc 1 47 19 discriminator 3
	addl	$1, -12(%ebp)
.L5:
	.loc 1 47 11 discriminator 1
	cmpl	$29999, -12(%ebp)
	jle	.L6
.LBB3:
	.loc 1 49 5
	call	ata_is_ready
	.loc 1 49 4
	testl	%eax, %eax
	jns	.L7
	.loc 1 50 10
	movl	$-1, %eax
	jmp	.L4
.L7:
.LBE3:
	.loc 1 52 8
	subl	$12, %esp
	pushl	$497
	call	inb
	addl	$16, %esp
	movb	%al, -13(%ebp)
	.loc 1 53 9
	movzbl	-13(%ebp), %eax
	andl	$129, %eax
	.loc 1 53 4
	testl	%eax, %eax
	je	.L8
	.loc 1 54 15
	movb	$1, ata
.L8:
	.loc 1 56 8
	subl	$12, %esp
	pushl	$369
	call	inb
	addl	$16, %esp
	movb	%al, -13(%ebp)
	.loc 1 57 9
	movzbl	-13(%ebp), %eax
	andl	$129, %eax
	.loc 1 57 4
	testl	%eax, %eax
	je	.L9
	.loc 1 58 15
	movb	$1, ata+1
.L9:
	.loc 1 60 14
	movzbl	ata, %eax
	.loc 1 60 4
	testb	%al, %al
	jne	.L10
	.loc 1 60 30 discriminator 1
	movzbl	ata+1, %eax
	.loc 1 60 18 discriminator 1
	testb	%al, %al
	jne	.L10
	.loc 1 61 10
	movl	$-1, %eax
	jmp	.L4
.L10:
	.loc 1 63 7
	subl	$12, %esp
	pushl	$500
	call	inb
	addl	$16, %esp
	movb	%al, -14(%ebp)
	.loc 1 64 7
	subl	$12, %esp
	pushl	$501
	call	inb
	addl	$16, %esp
	movb	%al, -15(%ebp)
	.loc 1 66 9
	movl	$-1, %eax
.L4:
	.loc 1 67 1
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	ata_diag_disk, .-ata_diag_disk
	.globl	ata_is_ready
	.type	ata_is_ready, @function
ata_is_ready:
.LFB2:
	.loc 1 70 1
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	.loc 1 71 5
	movb	$0, -9(%ebp)
	.loc 1 72 6
	movl	$0, -16(%ebp)
.L14:
	.loc 1 75 12
	subl	$12, %esp
	pushl	$503
	call	inb
	addl	$16, %esp
	movb	%al, -9(%ebp)
	.loc 1 76 6
	movzbl	-9(%ebp), %eax
	.loc 1 76 5
	testb	%al, %al
	js	.L14
	.loc 1 76 30 discriminator 1
	movzbl	-9(%ebp), %eax
	andl	$64, %eax
	.loc 1 76 26 discriminator 1
	testl	%eax, %eax
	jne	.L14
	.loc 1 77 11
	movl	$1, %eax
	.loc 1 82 1
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size	ata_is_ready, .-ata_is_ready
	.globl	ata_init
	.type	ata_init, @function
ata_init:
.LFB3:
	.loc 1 85 1
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$8, %esp
	.loc 1 87 2
	subl	$4, %esp
	pushl	$10
	pushl	$0
	pushl	$ata
	call	memset
	addl	$16, %esp
	.loc 1 88 2
	call	ata_diag_disk
	.loc 1 89 1
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE3:
	.size	ata_init, .-ata_init
	.type	ata_write, @function
ata_write:
.LFB4:
	.loc 1 92 1
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	.loc 1 94 1
	nop
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE4:
	.size	ata_write, .-ata_write
	.globl	ata_write_cmd
	.type	ata_write_cmd, @function
ata_write_cmd:
.LFB5:
	.loc 1 97 1
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	.loc 1 99 1
	nop
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE5:
	.size	ata_write_cmd, .-ata_write_cmd
	.globl	ata_write_data
	.type	ata_write_data, @function
ata_write_data:
.LFB6:
	.loc 1 102 1
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	.loc 1 104 1
	nop
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE6:
	.size	ata_write_data, .-ata_write_data
	.globl	ata_read
	.type	ata_read, @function
ata_read:
.LFB7:
	.loc 1 107 1
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	.loc 1 109 1
	nop
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE7:
	.size	ata_read, .-ata_read
.Letext0:
	.file 2 "/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/include/type.h"
	.file 3 "ata.h"
	.file 4 "/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/include/string.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x295
	.value	0x5
	.byte	0x1
	.byte	0x4
	.long	.Ldebug_abbrev0
	.uleb128 0xd
	.long	.LASF24
	.byte	0x1d
	.long	.LASF0
	.long	.LASF1
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x8
	.string	"u8"
	.byte	0x5
	.long	0x2f
	.uleb128 0x1
	.byte	0x1
	.byte	0x8
	.long	.LASF2
	.uleb128 0x8
	.string	"u16"
	.byte	0x6
	.long	0x40
	.uleb128 0x1
	.byte	0x2
	.byte	0x7
	.long	.LASF3
	.uleb128 0x1
	.byte	0x4
	.byte	0x7
	.long	.LASF4
	.uleb128 0x1
	.byte	0x8
	.byte	0x7
	.long	.LASF5
	.uleb128 0x1
	.byte	0x1
	.byte	0x6
	.long	.LASF6
	.uleb128 0x1
	.byte	0x2
	.byte	0x5
	.long	.LASF7
	.uleb128 0xe
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x1
	.byte	0x8
	.byte	0x5
	.long	.LASF8
	.uleb128 0x9
	.long	.LASF11
	.byte	0x2
	.byte	0x3
	.byte	0x2d
	.long	0x96
	.uleb128 0x4
	.long	.LASF9
	.byte	0x3
	.byte	0x2e
	.long	0x26
	.byte	0
	.uleb128 0x4
	.long	.LASF10
	.byte	0x3
	.byte	0x2f
	.long	0x26
	.byte	0x1
	.byte	0
	.uleb128 0x9
	.long	.LASF12
	.byte	0xa
	.byte	0x1
	.byte	0x15
	.long	0xbc
	.uleb128 0x4
	.long	.LASF13
	.byte	0x1
	.byte	0x16
	.long	0xbc
	.byte	0
	.uleb128 0xf
	.string	"dev"
	.byte	0x1
	.byte	0x17
	.byte	0x11
	.long	0xd3
	.byte	0x2
	.byte	0
	.uleb128 0xa
	.long	0x26
	.long	0xcc
	.uleb128 0xb
	.long	0xcc
	.byte	0x1
	.byte	0
	.uleb128 0x1
	.byte	0x4
	.byte	0x7
	.long	.LASF14
	.uleb128 0xa
	.long	0x71
	.long	0xe3
	.uleb128 0xb
	.long	0xcc
	.byte	0x3
	.byte	0
	.uleb128 0x10
	.string	"ata"
	.byte	0x1
	.byte	0x1b
	.byte	0x11
	.long	0x96
	.uleb128 0x5
	.byte	0x3
	.long	ata
	.uleb128 0xc
	.long	.LASF15
	.byte	0x4
	.byte	0xc
	.byte	0x7
	.long	0x115
	.long	0x115
	.uleb128 0x2
	.long	0x115
	.uleb128 0x2
	.long	0x63
	.uleb128 0x2
	.long	0x63
	.byte	0
	.uleb128 0x11
	.byte	0x4
	.uleb128 0x12
	.string	"inb"
	.byte	0x1
	.byte	0x7
	.byte	0xb
	.long	0x26
	.long	0x12d
	.uleb128 0x2
	.long	0x36
	.byte	0
	.uleb128 0xc
	.long	.LASF16
	.byte	0x1
	.byte	0x4
	.byte	0xb
	.long	0x26
	.long	0x148
	.uleb128 0x2
	.long	0x36
	.uleb128 0x2
	.long	0x26
	.byte	0
	.uleb128 0x5
	.long	.LASF17
	.byte	0x6a
	.long	0x63
	.long	.LFB7
	.long	.LFE7-.LFB7
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x5
	.long	.LASF18
	.byte	0x65
	.long	0x63
	.long	.LFB6
	.long	.LFE6-.LFB6
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x5
	.long	.LASF19
	.byte	0x60
	.long	0x63
	.long	.LFB5
	.long	.LFE5-.LFB5
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x13
	.long	.LASF25
	.byte	0x1
	.byte	0x5b
	.byte	0xc
	.long	0x63
	.long	.LFB4
	.long	.LFE4-.LFB4
	.uleb128 0x1
	.byte	0x9c
	.long	0x1ad
	.uleb128 0x14
	.string	"u3"
	.byte	0x1
	.byte	0x5b
	.byte	0xc
	.long	0x63
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.byte	0
	.uleb128 0x15
	.long	.LASF20
	.byte	0x1
	.byte	0x54
	.byte	0x5
	.long	0x63
	.long	.LFB3
	.long	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x16
	.long	.LASF22
	.byte	0x1
	.byte	0x45
	.byte	0x5
	.long	0x63
	.long	.LFB2
	.long	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x1f9
	.uleb128 0x17
	.long	.LASF21
	.byte	0x1
	.byte	0x47
	.byte	0x5
	.long	0x26
	.uleb128 0x2
	.byte	0x91
	.sleb128 -17
	.uleb128 0x3
	.string	"i"
	.byte	0x48
	.byte	0x6
	.long	0x63
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x18
	.long	.LASF26
	.byte	0x1
	.byte	0x26
	.byte	0x5
	.long	0x63
	.long	.LFB1
	.long	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x276
	.uleb128 0x3
	.string	"err"
	.byte	0x28
	.byte	0x5
	.long	0x26
	.uleb128 0x2
	.byte	0x91
	.sleb128 -21
	.uleb128 0x3
	.string	"lm"
	.byte	0x28
	.byte	0xf
	.long	0x26
	.uleb128 0x2
	.byte	0x91
	.sleb128 -22
	.uleb128 0x3
	.string	"lh"
	.byte	0x28
	.byte	0x17
	.long	0x26
	.uleb128 0x2
	.byte	0x91
	.sleb128 -23
	.uleb128 0x3
	.string	"i"
	.byte	0x29
	.byte	0x6
	.long	0x63
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x19
	.long	.LBB2
	.long	.LBE2-.LBB2
	.long	0x260
	.uleb128 0x6
	.long	.LASF22
	.long	0x63
	.uleb128 0x7
	.byte	0
	.byte	0
	.uleb128 0x1a
	.long	.LBB3
	.long	.LBE3-.LBB3
	.uleb128 0x6
	.long	.LASF22
	.long	0x63
	.uleb128 0x7
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x1b
	.long	.LASF23
	.byte	0x1
	.byte	0x1d
	.byte	0x5
	.long	0x63
	.long	.LFB0
	.long	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x6
	.long	.LASF22
	.long	0x63
	.uleb128 0x7
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7a
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0x21
	.sleb128 32
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 2
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 2
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 29
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 8
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x1f
	.uleb128 0x1b
	.uleb128 0x1f
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7a
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.value	0
	.value	0
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF11:
	.string	"ata_dev"
.LASF21:
	.string	"status"
.LASF23:
	.string	"ata_sw_rst"
.LASF18:
	.string	"ata_write_data"
.LASF2:
	.string	"unsigned char"
.LASF13:
	.string	"disk"
.LASF14:
	.string	"long unsigned int"
.LASF3:
	.string	"short unsigned int"
.LASF17:
	.string	"ata_read"
.LASF15:
	.string	"memset"
.LASF24:
	.string	"GNU C17 11.3.0 -mtune=generic -march=pentiumpro -g -ffreestanding"
.LASF19:
	.string	"ata_write_cmd"
.LASF16:
	.string	"outb"
.LASF4:
	.string	"unsigned int"
.LASF5:
	.string	"long long unsigned int"
.LASF22:
	.string	"ata_is_ready"
.LASF10:
	.string	"chan"
.LASF25:
	.string	"ata_write"
.LASF8:
	.string	"long long int"
.LASF12:
	.string	"ata_mgmr"
.LASF9:
	.string	"type"
.LASF7:
	.string	"short int"
.LASF6:
	.string	"signed char"
.LASF26:
	.string	"ata_diag_disk"
.LASF20:
	.string	"ata_init"
	.section	.debug_line_str,"MS",@progbits,1
.LASF1:
	.string	"/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/bootloader/x86/p32"
.LASF0:
	.string	"ata.c"
	.ident	"GCC: (GNU) 11.3.0"
