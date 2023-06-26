CC = x86_64-linux-gnu
CFLAGS = -Wall -O2 -g -c -m64 -ffreestanding #-nostdinc -nostdlib -nostartfiles
INC = /home/yohda/workspace/SW/bare-metal/yohdaOS/chap16/02.Kernel64/src/include

OBJ_DIR=temp
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o) 

#$(info $(SRCS))
#$(info $(OBJS))

all: pre $(OBJS) post

pre:
	mkdir -p $(OBJ_DIR) 

%.o: %.c
	$(CC)-gcc -I$(INC) $(CFLAGS) $^ 

post:
	cp *.o $(OBJ_DIR)
	cp ./$(OBJ_DIR)/*.o ../../$(OBJ_DIR)

clean: 
	rm -f *.o	
	rm -rf $(OBJ_DIR)

