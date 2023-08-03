In Yohda OS, There are two boot-loaders: first stage boot-loader(fbl.asm), secondary stage boot-loader(sbl.asm).

The FBL is responsible of loading the SBL to `0x7E00` with `4 sectors`. Basically, The size of SBL is limited in 2048 bytes.

Why 4 sectors? There are two reasons.
First of all, Actually, SBL does not have many things. it has a few jobs, following:

> 1. Enable A20
> 2. Set-up the Protected mode GDT
> 3. Read the Kernel image from disk drive to load it into memory.

So, 4 sectors are enough to fullfill above jobs.

Second, Yohda OS would like to support the compatibility backward with old disk drive, such as, floppy. But, there are some disk drive that read only 4 sectors per one read. 

After loading the binary of sbl into `0x7E00`, finally the fbl give a control to sbl with some disk paremeters to tell at where kenel image was stored and disk informations in current system.

```
_sbl:
    push word DISK_SBL_SECS+1   ; total sectors read by fbl
    push word [drive_number]    ; pass drive number
    push word [vga_rows]        ; pass command line number for sbl
    push word [secs]            ; pass sectors per track
    push word [heads]           ; pass heads
    push word [cylins]          ; pass cylinders
    push word [sbl_start_sec]   ; pass start sector
    push word [sbl_start_head]  ; pass start head
    push word [sbl_start_cylin] ; pass satrt cylinder
```


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
