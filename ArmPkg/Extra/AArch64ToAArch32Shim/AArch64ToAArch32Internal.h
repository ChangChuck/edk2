/** @file

  Copyright (c) 2014, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _AARCH64_TO_AARCH32_SHIM_H_
#define _AARCH64_TO_AARCH32_SHIM_H_

#include <Base.h>

#include <Library/ArmSmcLib.h>

#include <Chipset/AArch64.h>
#include <IndustryStandard/ArmStdSmc.h>

#define CORE_STACK_SIZE   0x100

typedef struct {
  UINT64 X0;
  UINT64 X1;
  UINT64 X2;
  UINT64 X3;
  UINT64 X4;
  UINT64 X5;
  UINT64 X6;
  UINT64 X7;
  UINT64 X8;
  UINT64 X9;
  UINT64 X10;
  UINT64 X11;
  UINT64 X12;
  UINT64 X13;
  UINT64 X14;
  UINT64 X15;
  UINT64 X16;
  UINT64 X17;
  UINT64 X18;
  UINT64 X19;
  UINT64 X20;
  UINT64 X21;
  UINT64 X22;
  UINT64 X23;
  UINT64 X24;
  UINT64 X25;
  UINT64 X26;
  UINT64 X27;
  UINT64 X28;
  UINT64 X29;
  UINT64 X30;
} AARCH64_GLOBAL_REGISTERS;

/**
 * Function use to see if the trapped instruction is the one we want to patch
 *
 * @param Registers                 Registers at the moment the instruction has been trapped.
 * @param ExceptionSyndromRegister  Value of the Syndrome register
 *
 */
VOID
AArch64ToAArch32Trap (
  IN AARCH64_GLOBAL_REGISTERS *Registers,
  IN UINT32                    ExceptionSyndromRegister
  );

/**
 * Entrypoint for the cores that have just been powered up and made the transition from EL3.
 * The core has been powered up by SMC CPU_ON.
 *
 * @param ContextId                 ContextId value passed to SMC CPU_ON.
 *
 */
VOID
AArch64ToAArch32ShimInit (
  IN UINTN ContextId
  );

#endif // _AARCH64_TO_AARCH32_SHIM_H_
