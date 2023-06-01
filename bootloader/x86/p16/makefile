all: boot.bin

boot.bin: boot.asm
	nasm -o boot.bin boot.asm

run:
	qemu-system-x86_64 -M pc-0.15 -m 4096 -fda boot.bin	

clean:
	rm -f boot.bin
	rm -f boot.o
	rm -f boot.elf
