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

#include "ArmMpServicesInternal.h"

VOID
EFIAPI
CompletionNotifyFunction (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
}

// Procedure wrapper
VOID
ProcedureWrapper (
  VOID
  )
{
  UINTN Index;
  UINTN ProcessorNumber;
  UINTN MpId;
  UINTN ClusterId;
  UINTN CoreId;
  BOOLEAN Completed;

  ProcessorNumber = 0;
  MpId            = ArmReadMpidr ();
  ClusterId       = GET_CLUSTER_ID (MpId);
  CoreId          = GET_CORE_ID (MpId);

  // Get the Processor Number
  for (Index = 0; Index < mArmProcessorTable->NumberOfEntries; Index++) {
    if ((mArmProcessorTable->ArmCpus[Index].ClusterId == ClusterId) &&
        (mArmProcessorTable->ArmCpus[Index].CoreId == CoreId))
    {
      ProcessorNumber = Index;
    }
  }

  if (mMpProcessorInfo[ProcessorNumber].Procedure != NULL) {
    mMpProcessorInfo[ProcessorNumber].Procedure (mMpProcessorInfo[ProcessorNumber].ProcedureArgument);
  }

  // Once completed we reset the Procedure related entries in the mailbox
  mMpProcessorInfo[ProcessorNumber].Procedure = NULL;
  mMpProcessorInfo[ProcessorNumber].ProcedureArgument = NULL;

  // The attributes has been written into Strongly-Ordered memory (so they are not pending
  // in any cache line), but we have to ensure the primary CPU uses these values
  InvalidateDataCacheRange (&mMpProcessorInfo[ProcessorNumber], sizeof(PROCESSOR_INFO));

  // Signal the completion of the procedure
  if (mMpProcessorInfo[ProcessorNumber].CompletionEvent != NULL) {
    // Check if there are still some running procedures
    Completed = TRUE;
    for (Index = 0; Index < mArmProcessorTable->NumberOfEntries; Index++) {
      if (IsSecondaryCoreBusy (Index)) {
        Completed = FALSE;
        break;
      }
    }

    if (Completed) {
      gBS->SignalEvent (mMpProcessorInfo[ProcessorNumber].CompletionEvent);
    }
  } else {
    ASSERT (0); // CompletionEvent should never be NULL
  }

  // Jump back to the Mailbox
  ((VOID (*)(VOID))((UINTN)mMpProcessorInfo[ProcessorNumber].Mailbox))();
}

EFI_STATUS
WaitForSecondaryToBeEnabled (
  IN ARM_CORE_INFO *ArmCpu
  )
{
  ARM_SMC_ARGS SmcArgs;
  UINTN        Retry;

  Retry = 50;
  do {
    SmcArgs.Arg0 = ARM_SMC_ID_PSCI_AFFINITY_INFO_AARCH64;
    SmcArgs.Arg1 = ARM_SMC_PSCI_TARGET_CPU64(0, 0, ArmCpu->ClusterId, ArmCpu->CoreId);
    SmcArgs.Arg2 = ARM_SMC_ID_PSCI_AFFINITY_LEVEL_0;
    ArmCallSmc (&SmcArgs);
    if (SmcArgs.Arg0 == ARM_SMC_ID_PSCI_AFFINITY_INFO_ON_PENDING) {
      gBS->Stall (1000); // Wait for 1ms before trying again
      Retry--;
    } else {
      break;
    }
  } while (Retry != 0);

  if ((Retry == 0) || (SmcArgs.Arg0 != ARM_SMC_ID_PSCI_AFFINITY_INFO_ON)) {
    return EFI_INVALID_PARAMETER;
  } else {
    return EFI_SUCCESS;
  }
}

BOOLEAN
IsSecondaryCoreBusy (
  IN UINTN ProcessorNumber
  )
{
  UINT64 JumpAddress;

  // If the CPU has not been enabled then it is not busy
  if (mMpProcessorInfo[ProcessorNumber].Mailbox == 0) {
    return FALSE;
  }

  // Check if the core is not already running some code
  JumpAddress = MmioRead64 ((UINTN)mMpProcessorInfo[ProcessorNumber].Mailbox + ACPI_ARM_MP_MAILBOX_JUMP_ADDR_OFFSET);

  return (JumpAddress != 0x0);
}

EFI_STATUS
SetProcedureToSecondaryCore (
  IN UINTN                     ProcessorNumber,
  IN EFI_AP_PROCEDURE          Procedure,
  IN VOID                      *ProcedureArgument      OPTIONAL,
  IN EFI_EVENT                 WaitEvent               OPTIONAL
  )
{
  ARM_CORE_INFO *ArmCpu;

  // Set the Procedure and Argument for the targeted CPU
  mMpProcessorInfo[ProcessorNumber].Procedure = Procedure;
  mMpProcessorInfo[ProcessorNumber].ProcedureArgument = ProcedureArgument;

  // We need to ensure mMpProcessorInfo[ProcessorNumber] has been flushed to the memory as the secondary
  // cores are running with disabled MMU
  WriteBackDataCacheRange ((VOID*)(UINTN)&mMpProcessorInfo[ProcessorNumber], sizeof (PROCESSOR_INFO));

  ArmCpu = &mArmProcessorTable->ArmCpus[ProcessorNumber];

  // Write the Jump Address and ProcessorId
  MmioWrite64 ((UINTN)mMpProcessorInfo[ProcessorNumber].Mailbox + ACPI_ARM_MP_MAILBOX_JUMP_ADDR_OFFSET, (UINT64)(UINTN)ProcedureWrapper);
  MmioWrite32 ((UINTN)mMpProcessorInfo[ProcessorNumber].Mailbox + ACPI_ARM_MP_MAILBOX_CPU_ID_OFFSET,
      ACPI_ARM_MP_MAILBOX_CPU_ID(ArmCpu->ClusterId, ArmCpu->CoreId));
  ArmDataSyncronizationBarrier ();

  return EFI_SUCCESS;
}
