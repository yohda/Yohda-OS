Boot loader
---
In Yohda OS, There are two boot-loaders: first stage boot-loader(fbl.asm), secondary stage boot-loader(sbl.asm).

The FBL is responsible of loading the SBL to `0x7E00` with `4 sectors`. Basically, The size of SBL is limited in 2048 bytes.

Why 4 sectors? There are two reasons.
First of all, Actually, SBL does not have many things. it has a few jobs, following:

> 1. Enable A20
> 2. Set-up the Protected mode GDT
> 3. Read the Kernel image from disk drive to memory.

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

Kernel Header
---
The header size is 128 bytes and header was appended at pre-position of kernel image. So, SBL first read a one sector from disk at `0x7E00`. 
|Name|Offset|Size|Description|
|-----|-------|-------|-----|
|Signature|0x0000|0x08|ASCII `YODA`|
|Kernel Size|0x0008|0x08|A unit is a sector|
|Physical Addresss|0x0010|0x08|Memory Load Physical Address|
|Virtual Addresss|0x0018|0x08|Memory Load Virtual Address|
|Offset|0x0020|0x04|Kernel is located from offset in bytes|
|Reserved|0x0024|0x5C|Reserved|
***

Kernel Emulator
---
For real mode in Yohda OS, there are many features to help you for developemnt.

- In YohdaOS, you have three choices to emulate `YOHDA-OS` on virtual machines such as `QEMU`, `BOCHS` and `Virtualbox`. But, if you would like to start your kernel with `BOCHS`, in the first time, you wiil have to make a disk image for `BOSCH`. But, don't worry about it. This is why you just type `./setup.sh` and print out the informations about disk image.
- For debugging, you have two choices, `QEMU` and `BOCHS`. If you would like to use `QEMU`, there are support three disk image type, `img`, `bin` and `iso`
    * If you type `make qf-debug`, disk image type is `floppy`
    * If you type `make qh-debug`, disk image type is `hard disk`
    * If you type `make qc-debug`, disk image type is `CD-ROM (this is ISO)`


## Supported Functions
* **Mode**
    * Real mode
    * Protected mode
    * Long mode

* **Emulators**
   * QEMU
   * BOCHS
   * Virtualbox
     
* **Boot Loader**
    * BIOS
    * GRUB
       * Multiboot
         
* **Architecture**
    * Higher-half kernel

* **Memory Map**
    * BIOS
    * GRUB

* **Disk Drive**
    * ATA / PATA / IDE
    * SATA / AHCI

* **Process Management**
    * Scheduling

* **Memory Management**
    * Segmentation
    * Paging
    * Buddy Allocator
       * Lazy Buddy   
    * Fast Memory Pool
      
* **File System**
  * FAT
    
* **Graphic**
  * VGA
     * Text Mode
        * Scrolling   

* **Misc**
   * Keyboard 
  
## Supported Modes

| ISA  | Supported |
| ------------- |:-------------:|
| x86      | Y     |
| x86-64      | Y     |
| AArch32 | X |
| AArch64 | X |

## Supported Emulators

YohdaOS supports three emulators : `QEMU`, `BOCHS` and `Virtualbox`


## Supported Boot Image
YohdaOS supports some of disk image format : `*.img`, `*.bin`, `*.iso`


## Supported Bootloaders

YohdaOS supports boot-up via some boot-loaders : `GRUB`

## Supported System Firmware
Under PC environment, YohdaOS works fine with some system firmwares  : `BIOS`, `UEFI`

## To-do implement
- Demand Page
- Swapping
- Multilevel-Feedback Queue Scheduling
- Lock-Free
- Thread
- Mmap
- Copy-On-Write
- Semaphore
- UEFI
- ACPI
