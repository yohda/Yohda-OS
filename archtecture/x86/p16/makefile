ASMS:=$(wildcard *.asm)
OBJS:=$(ASMS:.asm=.o)

OBJ_DIR:=temp
LD:=i686-elf-gcc
LDFLAG:= -ffreestanding -O2 -nostdlib -T link.ld

#$(info $(OBJS))

QE32:=qemu-system-i386
QE32_LEGACY_FLAG:=
QE32_Q35_FLAG:=-m 4096 -M q35 -d int -no-reboot -no-shutdown -D ./log.txt
QE32_HOST_FLAG:=-m 4096 -M virt -d int -no-reboot -no-shutdown -cpu host -enable-kvm

YASM:=nasm
ifdef P32 # for only real mode
YAFLAG:=-f elf -g3 -F dwarf -DP32
else
YAFLAG:=-f elf -g3 -F dwarf
endif

all: pre bl.elf post 

pre:
	mkdir -p $(OBJ_DIR)

%.o: %.asm	
	$(YASM) $(YAFLAG) $< -o $@

bl.elf: $(OBJS)
	$(LD) $(LDFLAG) $^ -o $@	
	objcopy -O binary -S $@ bl.bin

q35:
	$(QE32) $(QE32_FLAG) -fda $(OBJ_DIR)/bl.bin -boot c

qf:
	$(QE32) $(QE32_FLAG) -fda $(OBJ_DIR)/bl.bin -boot c
qc:
	$(QE32) $(QE32_FLAG) -drive file=$(OBJ_DIR)/bl.bin,media=cdrom -boot d #-fda $(OBJ_DIR)/bl.bin -boot c
qh:
	$(QE32) $(QE32_FLAG) -hda $(OBJ_DIR)/bl.img

host:
	$(QE32) $(QE32_HOST_FLAG) -fda $(OBJ_DIR)/bl.bin	

post:
	mv *.o $(OBJ_DIR)
	mv *.bin $(OBJ_DIR)
	mv *.elf $(OBJ_DIR)
	cp $(OBJ_DIR)/bl.bin $(OBJ_DIR)/bl.img 

q35-debug:
	$(QE32) $(QE32_Q35_FLAG) -fda $(OBJ_DIR)/bl.bin -boot c -S -s		

qh-debug:
	$(QE32) $(QE32_FLAG) -hda $(OBJ_DIR)/bl.img -s -S

host-debug:
	$(QE32) $(QE32_HOST_FLAG) -fda $(OBJ_DIR)/bl.bin -S -s		

gdb:
	gdb $(OBJ_DIR)/bl.elf -ex 'target remote localhost:1234' -ex 'set architecture i8086' -ex 'layout src' -ex 'layout regs' -ex 'break sbl.asm:155' -ex 'continue'

clean:
	rm -rf $(OBJ_DIR)	
	rm -f *.o *.d *.bin *.elf
