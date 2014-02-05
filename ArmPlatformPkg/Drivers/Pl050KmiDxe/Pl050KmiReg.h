/** @file
#
#  Component description file for PL011Uart module
#
#  Copyright (c) 2014, ARM Ltd. All rights reserved.<BR>
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#**/

/*
  Registers and fields used in the UEFI Driver for the PL050 Keyboard/Mouse
  Interface (PS2/AT)
*/

#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#define PL050_KB_BASE FixedPcdGet32 (PcdPL050KeyboardBaseAddress)

#define KB_READ_REG8(Offset) MmioRead8 (PL050_KB_BASE + Offset)
#define KB_WRITE_REG8(Offset, Val) MmioWrite8 (PL050_KB_BASE + Offset, Val)

/*
  Each register's offset is a "header", and followed by all of its field
   offsets.
  Register names are as in the datasheet but with underscores added for
   readability.
  Bit names are similarly made more readable, prefixed with the register's name,
   and anything redundant is removed (so we have KMI_CR_TYPE not
   KMI_CR_KMI_TYPE)
*/

#define KMI_CR            0x00

#define KMI_CR_EN         BIT2


#define KMI_STAT              0x04

#define KMI_STAT_TX_EMPTY    BIT6
#define KMI_STAT_TX_BUSY     BIT5
#define KMI_STAT_RX_FULL     BIT4
#define KMI_STAT_RX_BUSY     BIT3
#define KMI_STAT_RX_PARITY   BIT2


#define KMI_DATA    0x08

#define KMI_CLK_DIV 0x0C

#define KMI_IR      0x10

#define EIGHT_MHZ 8000000 // Desired internal clock frequency
#define GET_KMI_CLK_DIV(RefClkFreq) ((RefClkFreq / EIGHT_MHZ) - 1)
