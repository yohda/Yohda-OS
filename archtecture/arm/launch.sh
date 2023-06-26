#!/bin/bash

#BIN=build/kernel8.elf
BIN=kernel8.img
MONITOR_PORT=1235

#~/workspace/SW/bare-metal/qemu/qemu-6.2.0/build/qemu-system-aarch64 -M raspi3b -m 1024 -serial null -serial stdio -kernel kernel8.img

~/workspace/SW/bare-metal/qemu/qemu-6.2.0/build/qemu-system-aarch64 \
	-M raspi3b \
	-m 1024 \
	-nographic \
	-no-reboot \
	-kernel ${BIN} \
	-monitor telnet:127.0.0.1:${MONITOR_PORT},server,nowait \
	-serial stdio \
