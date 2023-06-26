SRCS:=$(wildcard *.c)
ASMS:=$(wildcard *.asm)
SRC_OBJS:=$(SRCS:%.c=%.o)
ASM_OBJS:=$(ASMS:.asm=.o)
OBJS:=$(SRCS:%.c=%.o)
OBJS+=$(ASMS:.asm=.o)

X86_16_BASE:=../p16
X86_32_BASE:=../p32

INC:=-I/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/include -I/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/block
OBJ_DIR:=temp
GCC:=x86_64-elf-gcc
STRICT:=
CFLAG:= -m64 -fPIC -ffreestanding -g -Wall -Wextra -nostdlib -nostartfiles $(STRICT) $(INC)  
LD:=x86_64-elf-ld
LDFLAG:= -O2 -nostdlib -T link.ld -melf_x86_64

$(info $(OBJS))

QEMU:=qemu-system-x86_64
QEMU_Q35_FLAG:=-m 4096 -M q35 -d int -no-reboot -no-shutdown -boot d -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 #-hda disk.img 
QEMU_I440FX_FLAG:=-m 2048 -M pc-0.15 -d int -no-reboot -no-shutdown -boot d -hda disk.img -monitor stdio
QEMU_HOST_FLAG:=-m 4096 -M virt -d int -no-reboot -no-shutdown -cpu host -enable-kvm -monitor stdio

BUILD_IMG:=kernel64.bin
IMG:=kernel.bin
DBG_IMG:=kernel.elf

YASM:=nasm
ifdef D
YAFLAG:=-f elf64 -g3 -F dwarf -DDEBUG 
else
YAFLAG:=-f elf64 -g3 -F dwarf 
endif

all: pre ${DBG_IMG} post 

pre:
	mkdir -p $(OBJ_DIR)
	make -C $(X86_32_BASE)

%.o: %.asm	
	$(YASM) $(YAFLAG) $< -o $@

%.o: %.c
	$(GCC) $(CFLAG) -MMD -MP -c $< -o $@ 

${DBG_IMG}: $(OBJS)
	$(LD) $(LDFLAG) $^ -o $@	
	objcopy -O binary -S $@ ${BUILD_IMG}

q35:
	$(QEMU) $(QEMU_Q35_FLAG) -fda $(OBJ_DIR)/${IMG}

i440:
	$(QEMU) $(QEMU_I440FX_FLAG) -fda $(OBJ_DIR)/${IMG}

host:
	$(QEMU) $(QEMU_HOST_FLAG) -fda $(OBJ_DIR)/${IMG}	

post:
	mv *.o $(OBJ_DIR)
	mv *.bin $(OBJ_DIR)
	mv *.elf $(OBJ_DIR)
	
	# create a disk drive image
	cp $(X86_32_BASE)/temp/kernel.bin $(OBJ_DIR)/kernel32.bin
	dd if=$(OBJ_DIR)/kernel32.bin of=disk.img
	truncate -s 264704 disk.img # 262704 = 512 + 2048 + 262144
	dd if=$(OBJ_DIR)/${BUILD_IMG} >> disk.img
	truncate -s 268435456 disk.img # Increment disk size to 256MB

	# concate 16-bit & 32-bit & 64-bit image ,in the order, to create the final image 
	cat $(OBJ_DIR)/kernel32.bin $(OBJ_DIR)/${BUILD_IMG} > $(OBJ_DIR)/$(IMG)
	chmod 775 $(OBJ_DIR)/$(IMG)

q35-debug:
	$(QEMU) $(QEMU_Q35_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s		

host-debug:
	$(QEMU) $(QEMU_HOST_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s		

i440-debug:
	$(QEMU) $(QEMU_I440FX_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s

gdb:
	gdb $(OBJ_DIR)/${DBG_IMG} -ex 'target remote localhost:1234' -ex 'layout src' -ex 'layout regs' -ex 'break lbl.asm:36' -ex 'continue' #-ex 'watch *0xb8070' 

clean:
	rm -rf $(OBJ_DIR)	
	rm -f *.o *.d *.bin *.elf

-include $(SRCS:.c=.d)
