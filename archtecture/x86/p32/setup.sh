#/bin/sh

FILE_SIZE=$(stat -L -c %s temp/kernel.bin)
make SIZE=${FILE_SIZE}
