/** @file
*
*  Copyright (c) 2013-2014, ARM Limited. All rights reserved.
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

#ifndef __ARM_MP_SERVICES_INTERNAL_H__
#define __ARM_MP_SERVICES_INTERNAL_H__

#include <PiDxe.h>
#include <Library/ArmMpServicesAcpiPsciLib.h>

#include <Library/CacheMaintenanceLib.h>
#include <Library/ArmGicLib.h>
#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/Cpu.h>
#include <Protocol/MpService.h>

#include <Guid/ArmMpCoreInfo.h>

#include <IndustryStandard/ArmStdSmc.h>
#include <IndustryStandard/AcpiArmMpStartup.h>

// Processor Information
typedef struct {
  EFI_PHYSICAL_ADDRESS Mailbox;
  EFI_AP_PROCEDURE     Procedure;
  VOID                 *ProcedureArgument;
  EFI_EVENT            CompletionEvent;
} PROCESSOR_INFO;

VOID
EFIAPI
CompletionNotifyFunction (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );

VOID
ProcedureWrapper (
  VOID
  );

EFI_STATUS
WaitForSecondaryToBeEnabled (
  IN ARM_CORE_INFO *ArmCpu
  );

BOOLEAN
IsSecondaryCoreBusy (
  IN UINTN ProcessorNumber
  );
EFI_STATUS
SetProcedureToSecondaryCore (
  IN UINTN                     ProcessorNumber,
  IN EFI_AP_PROCEDURE          Procedure,
  IN VOID                      *ProcedureArgument      OPTIONAL,
  IN EFI_EVENT                 WaitEvent               OPTIONAL
  );

extern ARM_PROCESSOR_TABLE *mArmProcessorTable;
extern PROCESSOR_INFO *mMpProcessorInfo;

#endif
