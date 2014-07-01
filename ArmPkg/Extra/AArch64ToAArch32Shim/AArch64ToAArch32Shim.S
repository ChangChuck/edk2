//
// Copyright (c) 2014, ARM Limited. All rights reserved.
//
// This program and the accompanying materials
// are licensed and made available under the terms and conditions of the BSD License
// which accompanies this distribution.  The full text of the license may be found at
// http://opensource.org/licenses/bsd-license.php
//
// THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
// WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//

#include <Base.h>
#include <Library/ArmLib.h>
#include <Chipset/AArch64.h>
#include <AsmMacroIoLibV8.h>

#include "AArch64ToAArch32Internal.h"

.text
.align 2

GCC_ASM_IMPORT(ExceptionVectorStart)

GCC_ASM_EXPORT(AArch64ToAArch32ShimStart)
GCC_ASM_EXPORT(AArch64ToAArch32ShimInit)
GCC_ASM_EXPORT(ArmCallSmc)
GCC_ASM_EXPORT(GuestResumeAddresses)

//
// Entrypoint for the AArch64 to AArch32 shim.
// Only the primary core is expected to run this code.
//
ASM_PFX(AArch64ToAArch32ShimStart):
  // Configure EL2 and Enable AArch32 world
  bl   ASM_PFX(InitEL2)

  // Get the start address of the 32bit UEFI firmware
  ldr  x8, =_PCD_VALUE_PcdFvBaseAddress
  msr  elr_el2, x8

  isb
  eret

//
// Configure EL2 and Enable AArch32 world
//
ASM_PFX(InitEL2):
  // CP15 Phy timers are accessible from EL1
  mrs   x8, cnthctl_el2
  orr   x8, x8, #(CNTHCTL_EL2_EL1PCTEN | CNTHCTL_EL2_EL1PCEN)
  msr   cnthctl_el2, x8

  // Setup SPSR to enter EL1 in SVC mode
  mov  x8, (SPSR_A | SPSR_I | SPSR_F | SPSR_AARCH32 | SPSR_AARCH32_MODE_SVC)
  msr  spsr_el2, x8

  msr  sctlr_el1, xzr

  //
  // Setup stack
  //

  // Get MPIDR
  mrs   x8, mpidr_el1
  // Compute the core position
  mov   x9, #ARM_CLUSTER_MASK
  and   x9, x8, x9
  // Cluster Id (offset 8) multiply by 4 (max core number per cluster)
  lsr   x9, x9, #(8 - 2)
  and   x8, x8, #ARM_CORE_MASK
  add   x10, x8, x9

  // Calculate the top of the stack
  mov  x8, #CORE_STACK_SIZE
  // x10 contains the core position from 0 to N-1 core
  add  x9, x10, #1
  mul  x9, x9, x8

  adr  x8, ASM_PFX(CoreStacks)
  add  sp, x8, x9

  //
  // Get the resume address of the 32bit UEFI firmware
  //

  // x10 contains the core position from 0 to N-1 core. And '8'
  // is the number of bytes for the resume address
  mov  x9, #8
  mul  x9, x9, x10

  adr  x8, ASM_PFX(GuestResumeAddresses)
  add  x8, x8, x9
  // Load the resume address at the appropriate offset.
  // Note: at the initialization the resume address will not be
  //       valid. But the function Arch64ToAArch32ShimInit() will
  //       set the correct start address
  ldr  x8, [x8]
  msr  elr_el2, x8

  //
  // Set the VBAR
  //
  adr  x8, ASM_PFX(ExceptionVectorStart)
  msr  vbar_el2, x8

  //
  // EL1 is 32-bit (Which means EL0 is also 32bit)
  // Physical IRQ while executing in the non-secure state at EL2 or
  // lower are taken to EL2 unless routed by the SCR_EL3.IRQ bit to EL3.
  // Virtual IRQ Interrupt is enabled.
  //
  // Enable SMC trapping
  mov  x8, #ARM_HCR_TSC
  msr  hcr_el2, x8

  ret

//
//  Trigger an SMC call
//
// SMC calls can take up to 7 arguments and return up to 4 return values.
// Therefore, the 4 first arguments of this function are used for both input
// and output values.
//
// Note: The implementation is duplicated as the library declared in the
//       DSC file is built for AArch32.
//
// VOID
// ArmCallSmc (
//   IN OUT ARM_SMC_ARGS *Args
//   );
ASM_PFX(ArmCallSmc):
  // Push x0 on the stack - The stack must always be quad-word aligned
  str   x0, [sp, #-16]!

  // Load the values into the appropriate registers
  ldp   x6, x7, [x0, #48]
  ldp   x4, x5, [x0, #32]
  ldp   x2, x3, [x0, #16]
  ldp   x0, x1, [x0, #0]

  smc   #0

  // Pop the ARM_SMC_ARGS structure address from the stack into x9
  ldr   x9, [sp], #16

  // Store the values of the registers back into the ARM_SMC_ARGS structure
  // A SMC call can return up to 4 values - we do not need to store back x4-x7.
  stp   x2, x3, [x9, #16]
  stp   x0, x1, [x9, #0]

  mov   x0, x9

  ret

//
// Entrypoint for the cores that have just been powered up and made the transition from EL3.
// The core has been powered up by SMC CPU_ON.
//
// @param ContextId                 ContextId value passed to SMC CPU_ON.
//
ASM_PFX(AArch64ToAArch32ShimInit):
  // Configure EL2 and Enable AArch32 world
  bl   ASM_PFX(InitEL2)

  isb
  eret

//
// Stacks
//
//FIXME: When the stack are located into a '.data' section, the relocation for
//       the instruction 'adr x8, ASM_PFX(CoreStacks)' is not correct.
//       There is probably an issue in the ELF -> PECOFF conversion (done by GenFw)
//.data
.align 4
ASM_PFX(CoreStacks):
// This code must be platform independent. And it cannot be mixed with AArch32 code;
// so we cannot use ArmPlatformLib (built as AArch32 in this firmware) in this module.
// For the stack we assume the worst case scenario (ie: 4 cores per cluster)
.space      _PCD_VALUE_PcdClusterCount * 4 * CORE_STACK_SIZE

//
// Guest resume addresses
//
.align 4
ASM_PFX(GuestResumeAddresses):
// This code must be platform independent. And it cannot be mixed with AArch32 code;
// so we cannot use ArmPlatformLib (built as AArch32 in this firmware) in this module.
// For the stack we assume the worst case scenario (ie: 4 cores per cluster).
// The '8' is to allow to have AArch64 resume addresses (8 bytes).
.space      _PCD_VALUE_PcdClusterCount * 4 * 8
