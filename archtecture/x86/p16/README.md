For real mode in Yohda OS, there are many features to help you for developemnt.

1" For building a real mode kernel, you just type `./setup.sh build`.
This command will build real mode kernel and make the disk image associated with 32-bit kernel image.

In YohdaOS, you have three choices to emulate virtual machines such as `QEMU`, `BOCHS` and `Virtualbox`
But, if you want to start your kernel with `BOCHS`, in the first time, you wiil have to prepare for setting disk image sectors.
But, don't worry about it. This is why you just type `./setup.sh` and print out the informations about disk image.

2" For debugging, you have two choices, `QEMU` and `BOCHS`
If you want to use `QEMU`, there are support three disk image type, `img`, `bin` and `iso`
- If you type `make qf-debug`, disk image type is `floppy`
- If you type `make qh-debug`, disk image type is `hard disk`
- If you type `make qc-debug`, disk image type is `CD-ROM (this is ISO)`
