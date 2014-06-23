#
#  Copyright (c) 2013-2015, ARM Limited. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = ArmJuno
  PLATFORM_GUID                  = ca0722fd-7d3d-45ea-948c-d62b2199807d
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/ArmJuno
  SUPPORTED_ARCHITECTURES        = AARCH64|ARM
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = ArmPlatformPkg/ArmJunoPkg/ArmJuno.fdf

# On RTSM, most peripherals are VExpress Motherboard peripherals
!include ArmPlatformPkg/ArmVExpressPkg/ArmVExpress.dsc.inc

[LibraryClasses.common]
  ArmPlatformLib|ArmPlatformPkg/ArmJunoPkg/Library/ArmJunoLib/ArmJunoLib.inf
  ArmSmcLib|ArmPkg/Library/ArmSmcLib/ArmSmcLib.inf

  ArmPlatformSysConfigLib|ArmPlatformPkg/ArmVExpressPkg/Library/ArmVExpressSysConfigLib/ArmVExpressSysConfigLib.inf
  NorFlashPlatformLib|ArmPlatformPkg/ArmJunoPkg/Library/NorFlashJunoLib/NorFlashJunoLib.inf
  EfiResetSystemLib|ArmPkg/Library/ArmPsciResetSystemLib/ArmPsciResetSystemLib.inf

  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf

  ArmMpServicesAcpiPsciLib|ArmPkg/Library/ArmMpServicesAcpiPsciLib/ArmMpServicesAcpiPsciLib.inf

  # USB Requirements
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf

!ifdef $(JUNO_MODEL)
  LcdPlatformLib|ArmPlatformPkg/ArmJunoPkg/Library/HdLcdArmJunoLib/HdLcdArmJunoLib.inf
!endif

[LibraryClasses.ARM]
  ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7Lib.inf

[LibraryClasses.AARCH64]
  ArmLib|ArmPkg/Library/ArmLib/AArch64/AArch64Lib.inf

[LibraryClasses.common.SEC]
  PrePiLib|EmbeddedPkg/Library/PrePiLib/PrePiLib.inf
  ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  LzmaDecompressLib|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  MemoryAllocationLib|EmbeddedPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  HobLib|EmbeddedPkg/Library/PrePiHobLib/PrePiHobLib.inf
  PrePiHobListPointerLib|ArmPlatformPkg/Library/PrePiHobListPointerLib/PrePiHobListPointerLib.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  PlatformPeiLib|ArmPlatformPkg/PlatformPei/PlatformPeiLib.inf

[LibraryClasses.common.SEC, LibraryClasses.common.PEIM]
  MemoryInitPeiLib|ArmPlatformPkg/MemoryInitPei/MemoryInitPeiLib.inf

[LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.UEFI_APPLICATION, LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.DXE_DRIVER]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf

[BuildOptions]
  RVCT:*_*_ARM_PLATFORM_FLAGS = --cpu Cortex-A15
  GCC:*_*_ARM_PLATFORM_FLAGS  = -mcpu=cortex-a15

!ifdef $(JUNO_EMULATOR)
  *_*_*_PLATFORM_FLAGS = -DJUNO_EMULATOR=1
  *_*_*_PP_FLAGS       = -DJUNO_EMULATOR=1
!endif

!ifdef $(JUNO_MODEL)
  *_*_*_PLATFORM_FLAGS = -DJUNO_MODEL=1
  *_*_*_PP_FLAGS       = -DJUNO_MODEL=1
!endif

  *_*_*_PLATFORM_FLAGS = -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include -I$(WORKSPACE)/ArmPlatformPkg/ArmJunoPkg/Include

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryInitializeInSec|TRUE

  ## If TRUE, Graphics Output Protocol will be installed on virtual handle created by ConsplitterDxe.
  #  It could be set FALSE to save size.
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdTurnOffUsbLegacySupport|TRUE

[PcdsFixedAtBuild.common]
  gArmPlatformTokenSpaceGuid.PcdFirmwareVendor|"ARM Juno"
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPrompt|"ArmJuno"

  #
  # NV Storage PCDs. Use base of 0x08000000 for NOR0
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase|0x0BFC0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0x0BFD0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0x0BFE0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareSize|0x00010000

  # System Memory (2GB - 16MB of Trusted DRAM at the top of the 32bit address space)
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x7F000000

  # Juno Dual-Cluster profile
  gArmPlatformTokenSpaceGuid.PcdCoreCount|6
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2

  gArmTokenSpaceGuid.PcdVFPEnabled|1

  #
  # ARM PrimeCell
  #

  ## PL011 - Serial Terminal
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x7FF80000
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200
!ifdef $(JUNO_EMULATOR)
  gArmPlatformTokenSpaceGuid.PL011UartInteger|1
  gArmPlatformTokenSpaceGuid.PL011UartFractional|0
!else
  gArmPlatformTokenSpaceGuid.PL011UartInteger|4
  gArmPlatformTokenSpaceGuid.PL011UartFractional|0
!endif

  ## PL031 RealTimeClock
  gArmPlatformTokenSpaceGuid.PcdPL031RtcBase|0x1C170000

!ifdef $(JUNO_MODEL)
  ## HdLcd
  gArmPlatformTokenSpaceGuid.PcdArmHdLcdBase|0x7FF60000

  ## PL180 MMC/SD card controller
  gArmPlatformTokenSpaceGuid.PcdPL180SysMciRegAddress|0x1C010048
  gArmPlatformTokenSpaceGuid.PcdPL180MciBaseAddress|0x1C050000
!endif

  # LAN9118 Ethernet Driver
  gEmbeddedTokenSpaceGuid.PcdLan9118DxeBaseAddress|0x18000000
  gEmbeddedTokenSpaceGuid.PcdLan9118DefaultMacAddress|0x1215161822242628

  #
  # ARM General Interrupt Controller
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x2C010000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x2C02F000

  #
  # PLDA PCI Root Complex
  #
  gArmPlatformTokenSpaceGuid.PcdPciBusMax|255
  gArmPlatformTokenSpaceGuid.PcdPciIoBase|0x5f800000
  gArmPlatformTokenSpaceGuid.PcdPciIoSize|0x00800000
  gArmPlatformTokenSpaceGuid.PcdPciMmio32Base|0x50000000
  gArmPlatformTokenSpaceGuid.PcdPciMmio32Size|0x08000000
  gArmPlatformTokenSpaceGuid.PcdPciMmio64Base|0x4000000000
  gArmPlatformTokenSpaceGuid.PcdPciMmio64Size|0x100000000

  # List of Device Paths that support BootMonFs
  gArmPlatformTokenSpaceGuid.PcdBootMonFsSupportedDevicePaths|L"VenHw(E7223039-5836-41E1-B542-D7EC736C5E59)"

  #
  # ARM OS Loader
  #
!ifdef $(JUNO_EMULATOR)
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDescription|L"Linux from DRAM"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDevicePath|L"Fv(B73FE497-B92E-416E-8326-45AD0D270092)/LinuxLoader.efi"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootArgument|L"VenHw(30F57E4A-69CE-4FB7-B7A1-4C7CE49D57A6)/MemoryMapped(0x0,0x80080000,0x81000000) -c \"console=ttyAMA0,115200 earlycon=pl011,0x7ff80000 root=/dev/sda1 rootwait verbose debug\""

  # On the Emulator we directly boot the first entry
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|0
!else
!ifdef $(JUNO_MODEL)
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDescription|L"Linux from SemiHosting"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDevicePath|L"Fv(B73FE497-B92E-416E-8326-45AD0D270092)/LinuxLoader.efi"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootArgument|L"VenHw(C5B9C74A-6D72-4719-99AB-C59F199091EB)/Image -f VenHw(C5B9C74A-6D72-4719-99AB-C59F199091EB)/initrd -c \"console=ttyAMA0,115200 earlycon=pl011,0x7ff80000 root=/dev/sda1 rootwait verbose debug\""

  # We reduce the timeout to 2 seconds to reduce booting time on the model with cache state modelled
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|2
!else
  # Support the Linux EFI stub by default
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDescription|L"EFI Linux from NOR Flash"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDevicePath|L"VenHw(E7223039-5836-41E1-B542-D7EC736C5E59)/Image"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootArgument|L"console=ttyAMA0,115200 earlycon=pl011,0x7ff80000 root=/dev/sda1 rootwait verbose debug"
!endif
!endif

  # Use the serial console (ConIn & ConOut) and the Graphic driver (ConOut)
  gArmPlatformTokenSpaceGuid.PcdDefaultConOutPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi();VenHw(CE660500-824D-11E0-AC72-0002A5D5C51B)"
  gArmPlatformTokenSpaceGuid.PcdDefaultConInPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi()"

  #
  # ARM Architectural Timer Frequency
  #
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|50000000
  gEmbeddedTokenSpaceGuid.PcdMetronomeTickPeriod|1000

[PcdsPatchableInModule]
  # Console Resolution (Full HD)
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1920
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1080

[PcdsDynamicDefault.common]
  #
  # The size of a dynamic PCD of the (VOID*) type can not be increased at run
  # time from its size at build time. Set the "PcdFdtDevicePaths" PCD to a 128
  # character "empty" string, to allow to be able to set FDT text device paths
  # up to 128 characters long.
  #
  gEmbeddedTokenSpaceGuid.PcdFdtDevicePaths|L"                                                                                                                                "

  # Not all Juno platforms support PCI. This dynamic PCD disables or enable
  # PCI support.
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciDisableBusEnumeration|TRUE

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]
  #
  # PEI Phase modules
  #
  ArmPlatformPkg/PrePi/PeiMPCore.inf {
    <LibraryClasses>
      ArmPlatformGlobalVariableLib|ArmPlatformPkg/Library/ArmPlatformGlobalVariableLib/PrePi/PrePiArmPlatformGlobalVariableLib.inf
  }

  #
  # DXE
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
  }

  #
  # Architectural Protocols
  #
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  EmbeddedPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf

  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  EmbeddedPkg/SerialDxe/SerialDxe.inf

!ifdef $(JUNO_EMULATOR)
  MdeModulePkg/Universal/Variable/EmuRuntimeDxe/EmuVariableRuntimeDxe.inf
!else
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
!endif

  #
  # ACPI Support
  #
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  ArmPlatformPkg/ArmJunoPkg/AcpiTables/AcpiTables.inf

  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPlatformPkg/Drivers/NorFlashDxe/NorFlashDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  ArmPkg/Drivers/GenericWatchdogDxe/GenericWatchdogDxe.inf

  #
  # Semi-hosting filesystem
  #
  ArmPkg/Filesystem/SemihostFs/SemihostFs.inf

!ifdef $(JUNO_MODEL)
  #
  # Graphic Output Protocol
  #
  ArmPlatformPkg/Drivers/LcdGraphicsOutputDxe/HdLcdGraphicsOutputDxe.inf

  #
  # Multimedia Card Interface
  #
  EmbeddedPkg/Universal/MmcDxe/MmcDxe.inf
  ArmPlatformPkg/Drivers/PL180MciDxe/PL180MciDxe.inf
!endif

  #
  # FAT filesystem + GPT/MBR partitioning
  #
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf

  # Required by PCI
  UefiCpuPkg/CpuIo2Dxe/CpuIo2Dxe.inf

  #
  # PCI Support
  #
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  ArmPlatformPkg/ArmJunoPkg/Drivers/PciHostBridgeDxe/PciHostBridgeDxe.inf

  #
  # SATA Controller
  #
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  EmbeddedPkg/Drivers/SataSiI3132Dxe/SataSiI3132Dxe.inf

  #
  # Networking stack
  #
  EmbeddedPkg/Drivers/Lan9118Dxe/Lan9118Dxe.inf

  #
  # Usb Support
  #
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf

  #
  # Juno platform driver
  #
  ArmPlatformPkg/ArmJunoPkg/Drivers/ArmJunoDxe/ArmJunoDxe.inf

  #
  # Bds
  #
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  ArmPlatformPkg/Bds/Bds.inf

[Components.ARM]
  ArmPkg/Extra/AArch64ToAArch32Shim/AArch64ToAArch32Shim.inf
