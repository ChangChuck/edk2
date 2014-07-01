/** @file

  Copyright (c) 2014, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "AArch64ToAArch32Internal.h"

extern UINT64 GuestResumeAddresses[];

/**
 * Function used to see if the trapped instruction is the one we want to patch
 *
 * @param Registers                 Registers at the moment the instruction has been trapped.
 * @param ExceptionSyndromRegister  Value of the Syndrome register
 *
 */
VOID
AArch64ToAArch32Trap (
  IN AARCH64_GLOBAL_REGISTERS *Registers,
  IN UINT32                    ExceptionSyndromRegister
  )
{
  UINTN ClusterId;
  UINTN CoreId;

  // Detect if the trap is from a SMC
  if (AARCH64_ESR_EC (ExceptionSyndromRegister) == AARCH64_ESR_EC_SMC32) {
    //
    // Detect if it is ARM_SMC_ID_PSCI_CPU_ON_AARCH32 otherwise we do not need to change the arguments
    //
    if (Registers->X0 == ARM_SMC_ID_PSCI_CPU_ON_AARCH32) {
      // Saved the AArch32 resume address - 4 cores per cluster has been considered to avoid
      // platform specific code.
      ClusterId = ARM_SMC_PSCI_TARGET_GET_AFF1(Registers->X1);
      CoreId    = ARM_SMC_PSCI_TARGET_GET_AFF0(Registers->X1);
      GuestResumeAddresses[(ClusterId * 4) + CoreId] = Registers->X2;

      // We correct the SMC Function ID to use the AArch64 one
      Registers->X0 = ARM_SMC_ID_PSCI_CPU_ON_AARCH64;
      //Note: The Target CPU (ie: Registers->Arg1) does not need to be changed between AArch32 to AArch64
      Registers->X2 = (UINT64)AArch64ToAArch32ShimInit;
    }

    // Send SMC - The returned values are directly written back into the registers
    ArmCallSmc ((ARM_SMC_ARGS*)Registers);
  } else {
    // We should never get there
    while (1);
  }
}
