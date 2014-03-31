/** @file
*
*  Copyright (c) 2013, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <PiDxe.h>

#include <Library/ArmPlatformSysConfigLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/LcdPlatformLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/Cpu.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/EdidActive.h>

#include <ArmPlatform.h>

typedef struct {
  UINT32                     Mode;
  UINT32                     HorizontalResolution;
  UINT32                     VerticalResolution;
  LCD_BPP                    Bpp;
  UINT32                     OscFreq;

  // These are used by HDLCD
  UINT32                     HSync;
  UINT32                     HBackPorch;
  UINT32                     HFrontPorch;
  UINT32                     VSync;
  UINT32                     VBackPorch;
  UINT32                     VFrontPorch;
} LCD_RESOLUTION;


STATIC CONST LCD_RESOLUTION mResolutions[] = {
  { // Mode 0 : VGA : 640 x 480 x 24 bpp
    VGA, VGA_H_RES_PIXELS, VGA_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, VGA_OSC_FREQUENCY,
    VGA_H_SYNC, VGA_H_BACK_PORCH, VGA_H_FRONT_PORCH,
    VGA_V_SYNC, VGA_V_BACK_PORCH, VGA_V_FRONT_PORCH
  },
  { // Mode 1 : UXGA : 1600 x 1200 x 24 bpp
    UXGA, UXGA_H_RES_PIXELS, UXGA_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, (UXGA_OSC_FREQUENCY / 2),
    UXGA_H_SYNC, UXGA_H_BACK_PORCH, UXGA_H_FRONT_PORCH,
    UXGA_V_SYNC, UXGA_V_BACK_PORCH, UXGA_V_FRONT_PORCH
  },
  { // Mode 2 : HD : 1920 x 1080 x 24 bpp
    HD, HD_H_RES_PIXELS, HD_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, (HD_OSC_FREQUENCY / 2),
    HD_H_SYNC, HD_H_BACK_PORCH, HD_H_FRONT_PORCH,
    HD_V_SYNC, HD_V_BACK_PORCH, HD_V_FRONT_PORCH
  }
};

EFI_STATUS
LcdPlatformInitializeDisplay (
  IN EFI_HANDLE   Handle
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
LcdPlatformGetVram (
  OUT EFI_PHYSICAL_ADDRESS      *VramBaseAddress,
  OUT UINTN                     *VramSize
  )
{
  EFI_STATUS              Status;
  EFI_CPU_ARCH_PROTOCOL   *Cpu;

  // Ensure VramBaseAddress and VramSize are not NULL
  ASSERT (VramBaseAddress && VramSize);

  // Set the vram size
  *VramSize = LCD_VRAM_SIZE;

  // Allocate the VRAM from the DRAM so that nobody else uses it.
  Status = gBS->AllocatePages (AllocateAnyPages, EfiBootServicesData, EFI_SIZE_TO_PAGES (*VramSize), VramBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Ensure the Cpu architectural protocol is already installed
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&Cpu);
  if (EFI_ERROR (Status)) {
    gBS->FreePool (VramBaseAddress);
    return Status;
  }

  // Mark the VRAM as un-cacheable. The VRAM is inside the DRAM, which is cacheable.
  Status = Cpu->SetMemoryAttributes (Cpu, *VramBaseAddress, *VramSize, EFI_MEMORY_UC);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    gBS->FreePool (VramBaseAddress);
    return Status;
  }

  return EFI_SUCCESS;
}

UINT32
LcdPlatformGetMaxMode (
  VOID
  )
{
  //
  // The following line will report correctly the total number of graphics modes
  // supported by the graphics driver:
  //
  return (sizeof (mResolutions) / sizeof (LCD_RESOLUTION));
}

EFI_STATUS
LcdPlatformSetMode (
  IN UINT32                         ModeNumber
  )
{
  if (ModeNumber >= LcdPlatformGetMaxMode ()) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
LcdPlatformQueryMode (
  IN  UINT32                                ModeNumber,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info
  )
{
  if (ModeNumber >= LcdPlatformGetMaxMode ()) {
    return EFI_INVALID_PARAMETER;
  }

  // Check 'Info' is not NULL
  ASSERT (Info != NULL);

  Info->Version = 0;
  Info->HorizontalResolution = mResolutions[ModeNumber].HorizontalResolution;
  Info->VerticalResolution = mResolutions[ModeNumber].VerticalResolution;
  Info->PixelsPerScanLine = mResolutions[ModeNumber].HorizontalResolution;

  switch (mResolutions[ModeNumber].Bpp) {
    case LCD_BITS_PER_PIXEL_24:
      Info->PixelFormat                   = PixelRedGreenBlueReserved8BitPerColor;
      Info->PixelInformation.RedMask      = LCD_24BPP_RED_MASK;
      Info->PixelInformation.GreenMask    = LCD_24BPP_GREEN_MASK;
      Info->PixelInformation.BlueMask     = LCD_24BPP_BLUE_MASK;
      Info->PixelInformation.ReservedMask = LCD_24BPP_RESERVED_MASK;
      break;

    case LCD_BITS_PER_PIXEL_16_555:
    case LCD_BITS_PER_PIXEL_16_565:
    case LCD_BITS_PER_PIXEL_12_444:
    case LCD_BITS_PER_PIXEL_8:
    case LCD_BITS_PER_PIXEL_4:
    case LCD_BITS_PER_PIXEL_2:
    case LCD_BITS_PER_PIXEL_1:
    default:
      // These are not supported
      ASSERT (FALSE);
      return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
LcdPlatformGetTimings (
  IN  UINT32        ModeNumber,
  OUT UINT32*       HRes,
  OUT UINT32*       HSync,
  OUT UINT32*       HBackPorch,
  OUT UINT32*       HFrontPorch,
  OUT UINT32*       VRes,
  OUT UINT32*       VSync,
  OUT UINT32*       VBackPorch,
  OUT UINT32*       VFrontPorch
  )
{
  if (ModeNumber >= LcdPlatformGetMaxMode ()) {
    return EFI_INVALID_PARAMETER;
  }

  // Check pointers are not NULL
  ASSERT (HRes && HSync && HBackPorch && HFrontPorch && VRes && VSync && VBackPorch && VFrontPorch);

  *HRes           = mResolutions[ModeNumber].HorizontalResolution;
  *HSync          = mResolutions[ModeNumber].HSync;
  *HBackPorch     = mResolutions[ModeNumber].HBackPorch;
  *HFrontPorch    = mResolutions[ModeNumber].HFrontPorch;
  *VRes           = mResolutions[ModeNumber].VerticalResolution;
  *VSync          = mResolutions[ModeNumber].VSync;
  *VBackPorch     = mResolutions[ModeNumber].VBackPorch;
  *VFrontPorch    = mResolutions[ModeNumber].VFrontPorch;

  return EFI_SUCCESS;
}

EFI_STATUS
LcdPlatformGetBpp (
  IN  UINT32        ModeNumber,
  OUT LCD_BPP*      Bpp
  )
{
  if (ModeNumber >= LcdPlatformGetMaxMode ()) {
    return EFI_INVALID_PARAMETER;
  }

  // Check Bpp is not NULL
  ASSERT (Bpp != NULL);

  *Bpp = mResolutions[ModeNumber].Bpp;

  return EFI_SUCCESS;
}
