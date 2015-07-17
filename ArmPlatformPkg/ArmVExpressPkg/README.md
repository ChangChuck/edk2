=== Requirements ===

*. The GCC toolchain for your architecture. Examples:
 - ARM: http://releases.linaro.org/15.02/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2015.02-3-x86_64_arm-linux-gnueabihf.tar.xz
 - AArch64: http://releases.linaro.org/15.02/components/toolchain/binaries/aarch64-linux-gnu/gcc-linaro-4.9-2015.02-3-x86_64_aarch64-linux-gnu.tar.xz

* Connect a serial console to UART0 see http://infocenter.arm.com/help/topic/com.arm.doc.dui0447j/ch01s01s01.html

=== Preparing the ARM Versatile Express Board ===

== Get the latest board files ==

Ensure that you update your Versatile Express board firmware to the latest version. To update your VE board firmware, please follow the instructions below:

    Connect and mount your Versatile Express motherboard USB mass storage device to your PC
    Install the Recovery firmware from the (v5.2 VE DVD)[https://silver.arm.com/download/download.tm?pv=1467447] onto your board.

    Unmount the Versatile Express motherboard
    Reboot the Versatile Express board
    At the “Cmd> “ prompt, type the following commands:

```
Cmd> flash
Cmd> eraseall
Cmd> exit
Cmd> reboot
``` 

== Get the latest Flat Device Tree (FDT) and Linux kernel ==

You can get them from Linaro website. Examples:

```
wget http://releases.linaro.org/15.05/openembedded/vexpress-lsk/hwpack_linaro-lsk-vexpress_20150522-720_armhf_supported.tar.gz
tar xzf hwpack_linaro-lsk-vexpress_20150522-720_armhf_supported.tar.gz
dpkg -x pkgs/linux-image-3.10.74.0-1-linaro-lsk-vexpress_3.10.74.0-1.1ubuntu1~ci+150521101438_armhf.deb .
```

The FDT are located in `lib/firmware/3.10.74.0-1-linaro-lsk-vexpress/device-tree` and the kernel `boot/vmlinuz-3.10.74.0-1-linaro-lsk-vexpress`

== Reseting the UEFI Variables stored in the Non Volatile storage on Versatile Express ==

The UEFI variables are stored in the NOR Flash of the Versatile Express. The UEFI variables are stored on the smaller block size partition of the NOR Flash - NOR Flash access is slow, using smaller block NOR Flash allows to get better performance than using larger block.

To erase the UEFI variables in the Non-Volatile storage, you need to restart the board to use ARM Boot Monitor (the ARM bootloader delivers with the ARM Versatile Express). On some platforms, Boot Monitor is enabled by changing the SCC used to switch NOR Flash mapping to boot either Boot Monitor or UEFI.

At the Bootmon prompt:

> flash
Flash> list areas

Base       Area Size Blocks Block Size
----       --------- ------ ----------
0x40000000    65280K    255       256K
0x43FC0000      256K      4        64K
0x44000000    65280K    255       256K
0x47FC0000      256K      4        64K

Flash> erase range 0x43FC0000
Erasing flash
Flash> erase range 0x47FC0000
Erasing flash
Flash>


=== Arm Versatile Express - Core Tile A9x4 ===

== Build the UEFI Firmware ==

1. Build
    make -f ArmPlatformPkg/Scripts/Makefile EDK2_DSC=ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-CTA9x4.dsc \
      EDK2_TOOLCHAIN=GCC49 GCC49_ARM_PREFIX=<gcc49-toolchain-path>/bin/arm-linux-gnueabihf-

2. Copy UEFI binaries in the Versatile Express motherboard USB mass storage device.

    cp Build/ArmVExpress-CTA9x4-Standalone/DEBUG_GCC49/FV/SEC_ARMVEXPRESS_EFI.fd <vexpress-usb>/SOFTWARE/a9_sec.bin
    cp Build/ArmVExpress-CTA9x4-Standalone/DEBUG_GCC49/FV/ARMVEXPRESS_EFI.fd <vexpress-usb>/SOFTWARE/a9_uefi.bin

3. Change the SCC 0x004 to make the second NOR Flash a Boot Device in `SITE1/HBI0191B/board.txt`. From:

    SCC: 0x004 0x00001F09

To:

    SCC: 0x004 0x10001F09

4. Configure the VExpress board with UEFI. Change the file `SITE1/HBI0191B/images.txt` on the VExpress mass-storage such as (note: the Boot Monitor image bm_v513r.axf might have a different name):

```
TITLE: Versatile Express Images Configuration File

[IMAGES]
TOTALIMAGES: 6                   ;Number of Images (Max : 32)
NOR0UPDATE: AUTO                 ;Image Update:NONE/AUTO/FORCE
NOR0ADDRESS: BOOT                ;Image Flash Address
NOR0FILE: \SOFTWARE\bm_v521l.axf ;Image File Name

NOR1UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR1ADDRESS: 40000000            ;Image Flash Address
NOR1NAME: BOOTSCRIPT             ;Image Name
NOR1FILE: \SOFTWARE\booscr9.txt  ;Image File Name

NOR2UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR2ADDRESS: 40000000            ;Image Flash Address
NOR2FILE: \SOFTWARE\ca9.dtb      ;Image File Name
NOR2LOAD: 68000000               ;Image Load Address
NOR2ENTRY: 00000000              ;Image Entry Point

NOR3UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR3ADDRESS: 41000000            ;Image Flash Address
NOR3FILE: \SOFTWARE\kernel.bin   ;Image File Name
NOR3LOAD: 60008000               ;Image Load Address
NOR3ENTRY: 60008000              ;Image Entry Point

NOR4UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR4ADDRESS: 44000000            ;Image Flash Address
NOR4FILE: \SOFTWARE\a9_sec.bin   ;Image File Name
NOR4LOAD: 0                      ;Image Load Address
NOR4ENTRY: 0                     ;Image Entry Point

NOR5UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR5ADDRESS: 45000000            ;Image Flash Address
NOR5FILE: \SOFTWARE\a9_uefi.bin  ;Image File Name
NOR5LOAD: 45000000               ;Image Load Address
NOR5ENTRY: 45000000              ;Image Entry Point
```

Note: Do not forget to unmount the Versatile Express motherboard USB mass storage device before rebooting

5. And reboot the board

=== Arm Versatile Express - TC2 A15x2-A7x3 ===

== Build the UEFI Firmware ==

1. Build
    make -f ArmPlatformPkg/Scripts/Makefile EDK2_DSC=ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-CTA15-A7.dsc \
      EDK2_TOOLCHAIN=GCC49 GCC49_ARM_PREFIX=<gcc49-toolchain-path>/bin/arm-linux-gnueabihf-

2. Copy UEFI binary in the Versatile Express motherboard USB mass storage device.

    cp Build/ArmVExpress-CTA15-A7/DEBUG_GCC49/FV/ARM_VEXPRESS_CTA15A7_EFI.fd <vexpress-usb>/SOFTWARE/tc2_uefi.bin

Note: Do not forget to unmount the Versatile Express motherboard USB mass storage device before rebooting

3. Change the Boot core in SITE1/HBI0249A/board.txt - BootMon use A15-0 as a primary core while UEFI uses A7-0 Change from:

    SCC: 0x700 0x0032F003           ;CFGRW48 - Boot cluster and CPU (CA15[0])

to:

    SCC: 0x700 0x1032F003           ;CFGRW48 - Boot cluster and CPU (CA7[0])

4. Configure the VExpress board with UEFI. Change the file SITE1/HBI0249A/images.txt on the VExpress mass-storage such as (note: the Boot Monitor image bm_v513r.axf might have a different name):

```
TITLE: Versatile Express Images Configuration File

[IMAGES]
TOTALIMAGES: 5                   ;Number of Images (Max: 32)

NOR0UPDATE: AUTO                 ;Image Update:NONE/AUTO/FORCE
NOR0ADDRESS: BOOT                ;Image Flash Address
NOR0FILE: \SOFTWARE\bm_v521r.axf ;Image File Name

NOR1UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR1ADDRESS: 00000000            ;Image Flash Address
NOR1NAME: BOOTSCRIPT             ;Image Flash Name
NOR1FILE: \SOFTWARE\booscr15.txt ;Image File Name

NOR2UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR2ADDRESS: 00000000            ;Image Flash Address
NOR2FILE: \SOFTWARE\ca15a7.dtb   ;Image File Name
NOR2LOAD: 88000000               ;Image Load Address
NOR2ENTRY: 00000000              ;Image Entry Point

NOR3UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR3ADDRESS: 00000000            ;Image Flash Address
NOR3NAME: kernel                 ;Image Flash Name
NOR3FILE: \SOFTWARE\kernel.bin   ;Image File Name
NOR3LOAD: 80008000               ;Image Load Address
NOR3ENTRY: 80008000              ;Image Entry Point

NOR4UPDATE: AUTO                 ;IMAGE UPDATE:NONE/AUTO/FORCE
NOR4ADDRESS: 0x0d000000          ;Image Flash Address
NOR4FILE: \SOFTWARE\tc2_uefi.bin     ;Image File Name
NOR4LOAD: 0xB0000000
NOR4ENTRY: 0xB0000000
```

5. Start the board and type:

    flash run tc2_uefi

=== SCT result for this plaform ===

# BootableImageSupportTest\SimpleFileSystemProtocolTest: some File System tests are expected to fail for the NOR Flash File System (ArmPlatformPkg/FileSystem/BootMonFs).
This file system does not support directory.