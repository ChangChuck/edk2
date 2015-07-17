=== Requirements ===

*. The AArch64 GCC toolchain. Examples:
 - http://releases.linaro.org/15.02/components/toolchain/binaries/aarch64-linux-gnu/gcc-linaro-4.9-2015.02-3-x86_64_aarch64-linux-gnu.tar.xz

* Connect a serial console to the top UART0 port (the top UART port on the back panel). The UART settings are 115200 bauds, 8 bits data, no parity, 1 bit for stop.

=== Recovery ===

== Restoring the default Juno Softwre Stack ==

Default factory settings & images can be restored with the latest recovery image from http://community.arm.com/servlet/JiveServlet/download/10177-1-18236/board_recovery_image_0.11.3.zip
Copy the content of the archive on your Juno USB mass storage device.

== Restoring the default UEFI configuration ==

If you wish to restore UEFI to a clean default configuration:

1.  Start the board into the "ARM V2M-Juno Boot loader". Either:
- Turn on the power.
- If the board was already powered on, press the black HW RESET button.

The bootloader is accessible on the UART0 port (the top UART port on the back panel).

2.  Run the followings commands:

```
Cmd> flash
Flash> areas
Base      Area Size Blocks Block Size
----      --------- ------ ----------
0x08000000    65280K    255      256K
0x0BFC0000      256K      4        64K
Flash> eraserange 0x0BFC0000
Erasing Flash
```

=== Build the UEFI Firmware ===

1. Configure your environment:

    export GCC49_AARCH64_PREFIX=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu-
    export IASL_PREFIX=<path-to-acpica>/generate/unix/bin/
    export JUNO_FIP=<juno-usb-mass-storage>/SOFTWARE/fip.bin

Ensure the `fip_create` tool is in your PATH environment variable.

2. Build the UEFI Firmware

    make -f ArmPlatformPkg/ArmJunoPkg/Makefile

Note: the build system should update the FIP image located on the Juno Board.
      Do not forget to unmount the Juno board before restarting it.

To build RELEASE version of UEFI firmware:

    make -f ArmPlatformPkg/ArmJunoPkg/Makefile EDK2_BUILD=RELEASE

To clean EDK2 source tree:

    make -f ArmPlatformPkg/ArmJunoPkg/Makefile clean

=== Build the Firmware Image Package (FIP) tool ===

The FIP tool is part of the ARM Trusted Firmware repository.

1. Clone the repository

    git clone https://github.com/ARM-software/arm-trusted-firmware.git

2. Build the FIP tool

    cd arm-trusted-firmware
    make fiptool

The `fip_create` tool is built in `<arm-trusted-firmware-path>/tools/fip_create/fip_create`

=== Build the IASL compiler ===

Note that this tool requires the bison and flex packages to be present on the host build machine.

1. Clone and Build IASL compiler

    git clone https://github.com/acpica/acpica.git

2. Build it

    cd acpica
    make

=== SCT result for this plaform ===

Some SCT tests are expected to fail.

# USBSupportTest\USBIOProtocolTest: If you run from a USB mass-storage, you will get this issue due to the limitation explain in Mantis 1096 (0001096: SCT USB Test will hang system if SCT test is running from USB key: https://mantis.uefi.org/mantis/view.php?id=1096)

See ArmPlatformPkg/ArmVExpressPkg/README.md for more test result information.