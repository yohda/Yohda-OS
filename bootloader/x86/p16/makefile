SRCS=$(wildcard *.asm)
OBJS=$(SRCS:%.asm=%.o)
OBJ_DIR=temp

QE32=qemu-system-i386
QE32_FLAG=-M pc-0.15 -m 4096 -fda
YASM=nasm
YAFLAG=-f elf -g3 -F dwarf 

all: pre bl.elf post

pre:
	mkdir -p $(OBJ_DIR)

%.o: %.asm	
	$(YASM) $(YAFLAG) $^ -o $@

bl.elf: $(OBJS)
	ld -Tlink.ld -melf_i386 $^ -o $@	
	objcopy -O binary -S $@ bl.bin

run:
	$(QE32) $(QE32_FLAG) $(OBJ_DIR)/bl.bin	

post:
	mv *.o *.bin *.elf $(OBJ_DIR)

debug:
	$(QE32) $(QE32_FLAG) $(OBJ_DIR)/bl.bin -S -s		

gdb:
	gdb $(OBJ_DIR)/bl.elf -ex 'target remote localhost:1234' -ex 'set architecture i8086' -ex 'layout src' -ex 'layout regs' -ex 'break *0x7C00' -ex 'continue'

clean:
	rm -rf $(OBJ_DIR)	
