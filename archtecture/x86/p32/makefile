SRCS:=$(wildcard *.c)
ASMS:=$(wildcard *.asm)
SRC_OBJS:=$(SRCS:%.c=%.o)
ASM_OBJS:=$(ASMS:.asm=.o)
OBJS:=$(SRCS:%.c=%.o)
OBJS+=$(ASMS:.asm=.o)

X86_16_BASE:=../p16
X86_64_BASE:=../p64

INC:=-I/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/include -I/home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/block
OBJ_DIR:=temp
GCC:=i686-elf-gcc
STRICT:= 
CFLAG:= -ffreestanding -g -Wall -Wextra -nostdlib -nostartfiles $(STRICT) $(INC) #-m64
LD:=i686-elf-gcc

ifdef G
LDFLAG:= -O2 -nostdlib -T link-grub.ld
else
LDFLAG:= -O2 -nostdlib -T link.ld
endif

QEMU:=qemu-system-i386
QEMU_Q35_FLAG:=-m 4096 -M q35  -no-reboot -no-shutdown -boot d -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -smp 2 #-hda disk.img 
QEMU_Q35_DEBUG_FLAG:= -m 4096 -M q35  -no-reboot -no-shutdown -boot d -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -smp 2 -d int
QEMU_I440FX_FLAG:=-m 2048 -M pc-0.15 -d int -no-reboot -no-shutdown -boot d -hda disk.img -monitor stdio
QEMU_HOST_FLAG:=-m 4096 -M virt -d int -no-reboot -no-shutdown -cpu host -enable-kvm -monitor stdio

BUILD_IMG:=kernel32.bin
IMG:=kernel.bin
ELF_IMG:=kernel.elf

YASM:=nasm
ifdef G
YAFLAG:=-f elf32 -g3 -F dwarf -DGRUB
else
YAFLAG:=-f elf32 -g3 -F dwarf
endif

all: pre ${ELF_IMG} post image

pre:
	mkdir -p $(OBJ_DIR)
ifndef G
ifeq (${SUB}, 1)
	$(info "continue to build 16-bit")
	$(shell ../p16/setup.sh "build")
else
	$(info "only to build 32-bit")
endif
endif

%.o: %.asm	
	$(YASM) $(YAFLAG) $< -o $@

%.o: %.c
	$(GCC) $(CFLAG) -MMD -MP -c $< -o $@ 

${ELF_IMG}: $(OBJS)
	$(LD) $(LDFLAG) $^ -o $@	
	objcopy -O binary -S $@ ${BUILD_IMG}

i440:
	$(QEMU) $(QEMU_I440FX_FLAG) -fda $(OBJ_DIR)/${IMG}

qf:
	$(QEMU) $(QEMU_Q35_FLAG) -fda $(OBJ_DIR)/bl.bin -boot c

qc:
	$(QEMU) $(QEMU_Q35_FLAG) -cdrom $(OBJ_DIR)/kernel32.iso

qh:
	$(QEMU) -m 4096 -M q35 -d int -no-reboot -no-shutdown -hda disk.img

host:
	$(QEMU) $(QEMU_HOST_FLAG) -fda $(OBJ_DIR)/${IMG}	

post:
	mv *.o $(OBJ_DIR)
	mv *.bin $(OBJ_DIR)
	mv *.elf $(OBJ_DIR)
	
image:
ifndef G
# If before it is failed to build 16-bit kernel, from now re-build	
ifeq (${SUB}, 0)
	$(shell ../p16/setup.sh "build")
	
	# concate 16bit image + 32bit image
	cp $(X86_16_BASE)/temp/bl.bin $(OBJ_DIR)
	cat $(OBJ_DIR)/bl.bin $(OBJ_DIR)/${BUILD_IMG} > $(OBJ_DIR)/$(IMG)
	chmod 775 $(OBJ_DIR)/$(IMG)
endif
else
	cp $(OBJ_DIR)/${BUILD_IMG} $(OBJ_DIR)/$(IMG)
	chmod 775 $(OBJ_DIR)/$(IMG)
endif
	

disk:
	cp $(X86_64_BASE)/temp/kernel64.bin $(OBJ_DIR)
	dd if=$(OBJ_DIR)/$(IMG) of=disk.img
	truncate -s 264704 disk.img # 262704 = 512 + 2048 + 262144
	dd if=$(OBJ_DIR)/kernel64.bin >> disk.img
	truncate -s 268435456 disk.img # Increment disk size to 256MB 

cd:
	mkdir -p ./iso/boot/grub
	cp $(OBJ_DIR)/${BUILD_IMG} ./iso
	cp grub.cfg ./iso/boot/grub
	grub-mkrescue -o kernel32.iso iso
	chmod 755 kernel32.iso
	mv kernel32.iso $(OBJ_DIR)/

qf-debug:
	$(QEMU) $(QEMU_Q35_DEBUG_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s		

qh-debug:
	$(QEMU) -m 4096 -M q35 -d int -no-reboot -no-shutdown -hda disk.img -s -S

qc-debug:
	$(QEMU) $(QEMU_Q35_DEBUG_FLAG) -cdrom $(OBJ_DIR)/kernel32.iso -s -S

host-debug:
	$(QEMU) $(QEMU_HOST_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s		

i440-debug:
	$(QEMU) $(QEMU_I440FX_FLAG) -fda $(OBJ_DIR)/${IMG} -S -s

gdb:
	gdb $(OBJ_DIR)/${ELF_IMG} -ex 'target remote localhost:1234' -ex 'layout src' -ex 'layout regs' -ex 'break main.c:main' -ex 'continue' #-ex 'watch *0x300000'

clean:
	rm -rf $(OBJ_DIR) ./iso
	rm -f *.o *.d *.bin *.elf  *.img
	
-include $(SRCS:.c=.d)
