SRCS:=$(wildcard *.c)
ASMS:=$(wildcard *.asm)
OBJS:=$(SRCS:%.c=%.o)
OBJS+=$(ASMS:.asm=.o)
INC:=/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/include
OBJ_DIR:=temp
GCC:=i686-elf-gcc
CFLAG:=-ffreestanding -g -Wall -Wextra -nostdlib -nostartfiles -I$(INC)
LD:=i686-elf-gcc
LDFLAG:= -ffreestanding -O2 -nostdlib -T link.ld

$(info $(OBJS))

QE32:=qemu-system-i386
QE32_LEGACY_FLAG:=
QE32_Q35_FLAG:=-m 4096 -M q35 -d int -no-reboot -no-shutdown -D ./log.txt
QE32_HOST_FLAG:= -m 4096 -M virt -d int -no-reboot -no-shutdown -cpu host -enable-kvm
YASM:=nasm
YAFLAG:=-f elf -g3 -F dwarf 

all: pre bl.elf post 

pre:
	mkdir -p $(OBJ_DIR)

%.o: %.asm	
	$(YASM) $(YAFLAG) $< -o $@

%.o: %.c
	$(GCC) $(CFLAG) -MMD -MP -c $< -o $@ 
	
bl.elf: $(OBJS)
	$(LD) $(LDFLAG) $^ -o $@	
	objcopy -O binary -S $@ bl.bin

q35:
	$(QE32) $(QE32_FLAG) -fda $(OBJ_DIR)/bl.bin	

host:
	$(QE32) $(QE32_HOST_FLAG) -hda $(OBJ_DIR)/bl.bin	

post:
	mv *.o *.bin *.elf $(OBJ_DIR)

q35-debug:
	$(QE32) $(QE32_Q35_FLAG) -fda $(OBJ_DIR)/bl.bin -S -s		

host-debug:
	$(QE32) $(QE32_HOST_FLAG) -fda $(OBJ_DIR)/bl.bin -S -s		

g16:
	gdb $(OBJ_DIR)/bl.elf -ex 'target remote localhost:1234' -ex 'set architecture i8086' -ex 'layout src' -ex 'layout regs' -ex 'break *0x7e00' -ex 'continue'

g32:
	gdb $(OBJ_DIR)/bl.elf -ex 'target remote localhost:1234' -ex 'layout src' -ex 'layout regs' -ex 'break main' -ex 'continue'

clean:
	rm -rf $(OBJ_DIR)	
	rm -f *.o *.d *.bin *.elf
	
-include $(SRCS:.c=.d)
