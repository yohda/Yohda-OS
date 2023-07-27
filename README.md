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
    * UEFI
    * GRUB
       * Multiboot 
* **Architecture**
    * Higher-half kernel

* **Memory Map**
    * BIOS
    * GRUB
    * UEFI

* **Disk Drive**
    * ATA / PATA / IDE
    * SATA / AHCI

* **Process Management**
    * Thread
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
- mmap
- Copy-On-Write
- Semaphore
