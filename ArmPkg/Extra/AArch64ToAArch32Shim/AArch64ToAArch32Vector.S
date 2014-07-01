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
#include <Chipset/AArch64.h>
#include <AsmMacroIoLibV8.h>

GCC_ASM_EXPORT(ExceptionVectorStart)
GCC_ASM_IMPORT(AArch64ToAArch32Trap)

.text

//
// This code gets copied to the ARM vector table
// VectorTableStart - VectorTableEnd gets copied
//
.align 11
ASM_PFX(ExceptionVectorStart):

//
// Current EL with SP0 : 0x0 - 0x180
//
ASM_PFX(SynchronousExceptionSP0):
  b   ASM_PFX(SynchronousExceptionEntry)

.align 7
ASM_PFX(IrqSP0):
  b   ASM_PFX(IrqEntry)

.align 7
ASM_PFX(FiqSP0):
  b   ASM_PFX(FiqEntry)

.align 7
ASM_PFX(SErrorSP0):
  b   ASM_PFX(SErrorEntry)

//
// Current EL with SPx: 0x200 - 0x380
//
.align 7
ASM_PFX(SynchronousExceptionSPx):
  b   ASM_PFX(SynchronousExceptionEntry)

.align 7
ASM_PFX(IrqSPx):
  b   ASM_PFX(IrqEntry)

.align 7
ASM_PFX(FiqSPx):
  b   ASM_PFX(FiqEntry)

.align 7
ASM_PFX(SErrorSPx):
  b   ASM_PFX(SErrorEntry)

//
// Lower EL using AArch64 : 0x400 - 0x580
//
.align 7
ASM_PFX(SynchronousExceptionA64):
  b   ASM_PFX(SynchronousExceptionEntry)

.align 7
ASM_PFX(IrqA64):
  b   ASM_PFX(IrqEntry)

.align 7
ASM_PFX(FiqA64):
  b   ASM_PFX(FiqEntry)

.align 7
ASM_PFX(SErrorA64):
  b   ASM_PFX(SErrorEntry)

//
// Lower EL using AArch32 : 0x0 - 0x180
//
.align 7
ASM_PFX(SynchronousExceptionA32):
  b   ASM_PFX(SynchronousExceptionEntry)

.align 7
ASM_PFX(IrqA32):
  b   ASM_PFX(IrqEntry)

.align 7
ASM_PFX(FiqA32):
  b   ASM_PFX(FiqEntry)

.align 7
ASM_PFX(SErrorA32):
  b   ASM_PFX(SErrorEntry)

#define REG_ONE(REG1, OFFSET, CONTEXT_SIZE)         stur REG1, [sp, #(OFFSET-CONTEXT_SIZE)]
#define REG_PAIR(REG1, REG2, OFFSET, CONTEXT_SIZE)  stp  REG1, REG2, [sp, #(OFFSET-CONTEXT_SIZE)]
#define REG_ONE(REG1, OFFSET, CONTEXT_SIZE)         stur REG1, [sp, #(OFFSET-CONTEXT_SIZE)]

/*
  UINT64  X0;     0x000
  UINT64  X1;     0x008
  UINT64  X2;     0x010
  UINT64  X3;     0x018
  UINT64  X4;     0x020
  UINT64  X5;     0x028
  UINT64  X6;     0x030
  UINT64  X7;     0x038
  UINT64  X8;     0x040
  UINT64  X9;     0x048
  UINT64  X10;    0x050
  UINT64  X11;    0x058
  UINT64  X12;    0x060
  UINT64  X13;    0x068
  UINT64  X14;    0x070
  UINT64  X15;    0x078
  UINT64  X16;    0x080
  UINT64  X17;    0x088
  UINT64  X18;    0x090
  UINT64  X19;    0x098
  UINT64  X20;    0x0a0
  UINT64  X21;    0x0a8
  UINT64  X22;    0x0b0
  UINT64  X23;    0x0b8
  UINT64  X24;    0x0c0
  UINT64  X25;    0x0c8
  UINT64  X26;    0x0d0
  UINT64  X27;    0x0d8
  UINT64  X28;    0x0e0
  UINT64  FP;     0x0e8   // x29 - Frame Pointer
  UINT64  LR;     0x0f0   // x30 - Link Register
  UINT64  SP;     0x0f8   // x31 - Stack Pointer
*/

#define GP_CONTEXT_SIZE    (32 *  8)

// Some of the GP registers - we do not need to save all the registers as we work in assembler
#define ALL_GP_REGS                                     \
        REG_PAIR (x0,  x1,  0x000, GP_CONTEXT_SIZE);    \
        REG_PAIR (x2,  x3,  0x010, GP_CONTEXT_SIZE);    \
        REG_PAIR (x4,  x5,  0x020, GP_CONTEXT_SIZE);    \
        REG_PAIR (x6,  x7,  0x030, GP_CONTEXT_SIZE);    \
        REG_PAIR (x8,  x9,  0x040, GP_CONTEXT_SIZE);    \
        REG_PAIR (x10, x11, 0x050, GP_CONTEXT_SIZE);    \
        REG_PAIR (x12, x13, 0x060, GP_CONTEXT_SIZE);    \
        REG_PAIR (x14, x15, 0x070, GP_CONTEXT_SIZE);    \
        REG_PAIR (x16, x17, 0x080, GP_CONTEXT_SIZE);    \
        REG_PAIR (x18, x19, 0x090, GP_CONTEXT_SIZE);    \
        REG_PAIR (x20, x21, 0x0a0, GP_CONTEXT_SIZE);    \
        REG_PAIR (x22, x23, 0x0b0, GP_CONTEXT_SIZE);    \
        REG_PAIR (x24, x25, 0x0c0, GP_CONTEXT_SIZE);    \
        REG_PAIR (x26, x27, 0x0d0, GP_CONTEXT_SIZE);    \
        REG_PAIR (x28, x29, 0x0e0, GP_CONTEXT_SIZE);    \
        REG_ONE  (x30,      0x0f0, GP_CONTEXT_SIZE);

// In order to save the SP we need to put it somwhere else first.
// STR only works with XZR/WZR directly
#define SAVE_SP \
        mov x1, sp; \
        REG_ONE (x1,        0x0f8, GP_CONTEXT_SIZE);

ASM_PFX(SynchronousExceptionEntry):
  // Save Global Registers & Stack pointer
  ALL_GP_REGS
  SAVE_SP

  // Point to top of struct after all regs saved
  sub   sp, sp, #GP_CONTEXT_SIZE

  // Set x0 to point to the top of our struct on the Stack
  mov   x0, sp

  // EL2 Exception syndrome register
  mrs   x1, esr_el2

  // VOID
  // AArch64ToAArch32Trap (
  //   IN AARCH64_GLOBAL_REGISTERS *Registers,
  //   IN UINT32                    ExceptionSyndromRegister
  //   )
  bl  ASM_PFX(AArch64ToAArch32Trap)

#undef REG_PAIR
#define REG_PAIR(REG1, REG2, OFFSET, CONTEXT_SIZE)  ldp  REG1, REG2, [sp, #(OFFSET-CONTEXT_SIZE)]
#define REG_ONE(REG1, OFFSET, CONTEXT_SIZE)         ldur REG1, [sp, #(OFFSET-CONTEXT_SIZE)]

  // Adjust ELR to execute the next instruction. We only trap SMC because there are always 4 bytes,
  // we do not need to check if we come from AArch32 Thumb/AArch32/AArch64.
  mrs   x0, elr_el2
  add   x0, x0, #4
  msr   elr_el2, x0

  // Adjust SP to be where we started from when we came into the handler.
  // The handler can not change the SP.
  add   sp, sp, #GP_CONTEXT_SIZE

  // Restore Global Registers
  ALL_GP_REGS

  eret

ASM_PFX(IrqEntry):
  // We do not trap IRQs
  b ASM_PFX(IrqEntry)

ASM_PFX(FiqEntry):
  // We do not trap FIQs
  b ASM_PFX(FiqEntry)

ASM_PFX(SErrorEntry):
  // We do not trap SError
  b ASM_PFX(SErrorEntry)
