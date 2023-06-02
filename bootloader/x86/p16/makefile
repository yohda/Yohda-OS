SRCS=$(wildcard *.asm)
BINS=$(SRCS:%.asm=%.bin)

YASM=nasm
YAFLAG=-f bin 

all: $(BINS) post 

%.bin: %.asm	
	$(YASM) $(YAFLAG) $^ -o $@

post:
	cat pbl.bin sbl.bin > bl.bin

prun:
	qemu-system-x86_64 -M pc-0.15 -m 4096 -fda pbl.bin	

srun:
	qemu-system-x86_64 -M pc-0.15 -m 4096 -fda sbl.bin	

run:
	qemu-system-x86_64 -M pc-0.15 -m 4096 -fda bl.bin	

clean:
	rm -f pbl.bin
	rm -f sbl.bin
	rm -f pbl.elf
	rm -f bl.bin
