SRCS:=$(wildcard *.c)
ASMS:=$(wildcard *.asm)
SRC_OBJS:=$(SRCS:%.c=%.o)
ASM_OBJS:=$(ASMS:.asm=.o)
OBJS:=$(SRCS:%.c=%.o)
OBJS+=$(ASMS:.asm=.o)

X86_16_BASE:=/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/bootloader/x86/p16

INC:=-I/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/include -I/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/block
OBJ_DIR:=temp
GCC:=i686-elf-gcc
CFLAG:=-ffreestanding -g -Wall -Wextra -nostdlib -nostartfiles $(INC)
LD:=i686-elf-gcc
LDFLAG:= -ffreestanding -O2 -nostdlib -T link.ld 

$(info $(OBJS))

QE32:=qemu-system-i386
QE32_Q35_FLAG:=-m 4096 -M q35 -d int -no-reboot -no-shutdown -boot d -hda disk.img -monitor stdio 
QE32_I440FX_FLAG:=-m 2048 -M pc-0.15 -d int -no-reboot -no-shutdown -boot d -hda disk.img -monitor stdio
QE32_HOST_FLAG:=-m 4096 -M virt -d int -no-reboot -no-shutdown -cpu host -enable-kvm -monitor stdio

BUILD_IMG:=kernel32.bin
IMG:=kernel.bin
DBG_IMG:=kernel.elf

YASM:=nasm
ifdef D
YAFLAG:=-f elf -g3 -F dwarf -DDEBUG
else
YAFLAG:=-f elf -g3 -F dwarf 
endif

all: pre ${DBG_IMG} post 

pre:
	mkdir -p $(OBJ_DIR)
	make -C ../p16

%.o: %.asm	
	$(YASM) $(YAFLAG) $< -o $@

%.o: %.c
	$(GCC) $(CFLAG) -MMD -MP -c $< -o $@ 

${DBG_IMG}: $(OBJS)
	$(LD) $(LDFLAG) $^ -o $@	
	objcopy -O binary -S $@ ${BUILD_IMG}

q35:
	$(QE32) $(QE32_Q35_FLAG) -fda $(OBJ_DIR)/${IMG}

i440:
	$(QE32) $(QE32_I440FX_FLAG) -fda $(OBJ_DIR)/${IMG}

host:
	$(QE32) $(QE32_HOST_FLAG) -fda $(OBJ_DIR)/${IMG}	

post:
	mv *.o $(OBJ_DIR)
	mv *.bin $(OBJ_DIR)
	mv *.elf $(OBJ_DIR)
	cp $(X86_16_BASE)/temp/bl.bin $(OBJ_DIR)
	cat $(OBJ_DIR)/bl.bin $(OBJ_DIR)/${BUILD_IMG} > $(OBJ_DIR)/$(IMG)
	chmod 775 $(OBJ_DIR)/$(IMG)

q35-debug:
	$(QE32) $(QE32_Q35_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s		

host-debug:
	$(QE32) $(QE32_HOST_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s		

i440-debug:
	$(QE32) $(QE32_I440FX_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s

gdb:
	gdb $(OBJ_DIR)/${DBG_IMG} -ex 'target remote localhost:1234' -ex 'layout src' -ex 'layout regs' -ex 'break main.c:18' -ex 'continue'

clean:
	rm -rf $(OBJ_DIR)	
	rm -f *.o *.d *.bin *.elf

-include $(SRCS:.c=.d)
