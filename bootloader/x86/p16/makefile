SRCS:=$(wildcard *.c)
ASMS:=$(wildcard *.asm)
OBJS:=$(SRCS:%.c=%.o)
OBJS+=$(ASMS:%.asm=%.o)
INC:=/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/include
OBJ_DIR:=temp
GCC:=i686-elf-gcc
CFLAG:=-ffreestanding -Wall -Wextra -nostdlib -nostartfiles -I$(INC)
LD:=i686-elf-gcc
LDFLAG:= -T link.ld -ffreestanding -O2 -nostdlib

$(info $(OBJS))

QE32:=qemu-system-i386
QE32_FLAG:=-M pc-0.15 -m 4096 -fda
YASM:=nasm
YAFLAG:=-f elf -g3 -F dwarf 

all: pre bl.elf post

pre:
	mkdir -p $(OBJ_DIR)

%.o: %.c
	$(GCC) $^ -c -o $@ $(CFLAG)  

%.o: %.asm	
	$(YASM) $(YAFLAG) $^ -o $@

bl.elf: $(OBJS)
	$(LD) $(LDFLAG) $^ -o $@	
	objcopy -O binary -S $@ bl.bin

run:
	$(QE32) $(QE32_FLAG) $(OBJ_DIR)/bl.bin	

post:
	mv *.o *.bin *.elf $(OBJ_DIR)

debug:
	$(QE32) $(QE32_FLAG) $(OBJ_DIR)/bl.bin -S -s		

gdb:
	gdb $(OBJ_DIR)/bl.elf -ex 'target remote localhost:1234' -ex 'set architecture i8086' -ex 'layout src' -ex 'layout regs' -ex 'break *0x7e00' -ex 'continue'

clean:
	rm -rf $(OBJ_DIR)	
