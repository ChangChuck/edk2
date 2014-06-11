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

#include <AsmMacroIoLib.h>
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
  mrc   p15, 0, r0, c0, c0, 5       @ read MPIDR
  mov	r1, #ARM_CLUSTER_MASK
  and	r1, r0, r1
  lsl	r1, r1, #8
  and	r0, r0, #ARM_CORE_MASK
  orr	r0, r0, r1

  # Set stack: The stack is located at the top of the firmware region
  adr	r1, MailboxCodeStack
  ldr	sp, [r1]
  //TODO: RemoveMe mov	sp, x1

  # Get GIC CPU Interface
  adr	r1, GicInterruptInterfaceBase
  ldr	r3, [r1]

  #
  # Mailbox Initialization
  #

  # Initialize Processor Id of the mailbox (to '~0')
  adr	r1, MailboxCodeProcessorIdOffset
  ldr	r1, [r1]
  mov	r2, #0
  str	r2, [r1]

  # Initialize Jump Address (to '0')
  adr	r2, MailboxCodeJumpAddressOffset
  ldr	r2, [r2]
  mov	r4, #0
  str	r4, [r2]

  #
  # Parking protocol
  #
  # r0: Expected Processor Id
  # r1: Processor Id Mailbox Address
  # r2: Jump Address Mailbox Address
  # r3: Gic Cpu Interface
MailboxParkingProtocol:
  wfi
  dsb

  # Read Processor Id
  ldr	r4, [r1]
  # Read Jump Address and check it is not zero
  ldr	r5, [r2]

AcknowledgeInterrupt:
  # Acknowledge Interrupt - Read the Interrupt Acknowledge Register
  ldr	r6, [r3, #ARM_GIC_ICCIAR]

  # Check it is not a spurious interrupt
  #TODO: We need to support GICv2 & GICv3
  #LoadConstantToReg (ARM_GIC_ICCIAR_ACKINTID, r7)
  LoadConstantToReg (0x3FF, r7)
  and	r6, r6, r7
  cmp	r6, #1020
  bhi	CheckProcessorIdValid

  # End Of Interrupt
  str	r5, [r3, #ARM_GIC_ICCEIOR]

CheckProcessorIdValid:
  # Is it our Processor Id
  cmp	r4, r0
  bne	MailboxParkingProtocol

CheckJumpAddressValid:
  # Is the Jump Address NULL
  cmp	r5, #0
  beq	MailboxParkingProtocol

ValidCondition:
  # Write 0 to Jump Address to Acknowledge
  mov	r6, #0
  str	r6, [r2]
  dsb

  # And Jump to the given entrypoint
  bx	r5

#
# Data to patch
#
.align 4
ASM_PFX(MailboxCodeStack):  			.long    0x0
ASM_PFX(MailboxCodeProcessorIdOffset):  .long    0x0
ASM_PFX(MailboxCodeJumpAddressOffset):  .long    0x0
ASM_PFX(GicInterruptInterfaceBase):		.long    FixedPcdGet32 (PcdGicInterruptInterfaceBase)
ASM_PFX(MailboxCodeEnd):
