//
//  Copyright (c) 2014, ARM Limited. All rights reserved.
//
//  This program and the accompanying materials
//  are licensed and made available under the terms and conditions of the BSD License
//  which accompanies this distribution.  The full text of the license may be found at
//  http://opensource.org/licenses/bsd-license.php
//
//  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
//  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//
//

#include <AsmMacroIoLibV8.h>
#include <AutoGen.h>

#include <IndustryStandard/AcpiArmMpStartup.h>

#include <Library/ArmLib.h>
#include <Library/ArmGicLib.h>

.text
.align 3

GCC_ASM_EXPORT(MailboxCodeStart)
GCC_ASM_EXPORT(MailboxCodeEnd)

GCC_ASM_EXPORT(MailboxCodeStack)
GCC_ASM_EXPORT(MailboxCodeProcessorIdOffset)
GCC_ASM_EXPORT(MailboxCodeJumpAddressOffset)

#
# Note: We should not invoked any helper functions in this code. The code must
#       be self-contained because it will copy into the MpMailbox
#
ASM_PFX(MailboxCodeStart):
  # Build Processor Id ((ClusterId << 16) | CoreId)
  mrs   x0, mpidr_el1
  mov	x1, #ARM_CLUSTER_MASK
  and	x1, x0, x1
  lsl	x1, x1, #8
  and	x0, x0, #ARM_CORE_MASK
  orr	x0, x0, x1

  # Set stack: The stack is located at the top of the firmware region
  adr	x1, MailboxCodeStack
  ldr	x1, [x1]
  mov	sp, x1

  # Get GIC CPU Interface
  adr	x1, GicInterruptInterfaceBase
  ldr	x3, [x1]

  #
  # Mailbox Initialization
  #

  # Initialize Processor Id of the mailbox (to '~0')
  adr	x1, MailboxCodeProcessorIdOffset
  ldr	x1, [x1]
  movn	w2, #0
  str	w2, [x1]

  # Initialize Jump Address (to '0')
  adr	x2, MailboxCodeJumpAddressOffset
  ldr	x2, [x2]
  str	xzr, [x2]

  #
  # Parking protocol
  #
  # x0: Expected Processor Id
  # x1: Processor Id Mailbox Address
  # x2: Jump Address Mailbox Address
  # x3: Gic Cpu Interface
MailboxParkingProtocol:
  wfi
  dsb   sy

  # Read Processor Id
  ldr	w4, [x1]
  # Read Jump Address and check it is not zero
  ldr	x5, [x2]

AcknowledgeInterrupt:
  # Acknowledge Interrupt - Read the Interrupt Acknowledge Register
  ldr	w6, [x3, ARM_GIC_ICCIAR]

  # Check it is not a spurious interrupt
  #TODO: We need to support GICv2 & GICv3
  #LoadConstantToReg (ARM_GIC_ICCIAR_ACKINTID, x7)
  LoadConstantToReg (0x3FF, x7)
  and	w6, w6, w7
  cmp	w6, #1020
  b.hi	CheckProcessorIdValid

  # End Of Interrupt
  str	w5, [x3, ARM_GIC_ICCEIOR]

CheckProcessorIdValid:
  # Is it our Processor Id
  cmp	w4, w0
  bne	MailboxParkingProtocol

CheckJumpAddressValid:
  # Is the Jump Address NULL
  cmp	x5, xzr
  beq	MailboxParkingProtocol

ValidCondition:
  # Write 0 to Jump Address to Acknowledge
  str	xzr, [x2]
  dsb   sy

  # And Jump to the given entrypoint
  br	x5

#
# Data to patch
#
.align 4
ASM_PFX(MailboxCodeStack):  			.long    0x0
										.long    0x0
ASM_PFX(MailboxCodeProcessorIdOffset):  .long    0x0
										.long    0x0
ASM_PFX(MailboxCodeJumpAddressOffset):  .long    0x0
										.long    0x0
ASM_PFX(GicInterruptInterfaceBase):		.long    FixedPcdGet32 (PcdGicInterruptInterfaceBase)
										.long    0x0
ASM_PFX(MailboxCodeEnd):
