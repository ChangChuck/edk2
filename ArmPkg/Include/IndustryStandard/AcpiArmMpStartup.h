/** @file
*
*  Copyright (c) 2014, ARM Limited. All rights reserved.
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

#ifndef __ACPI_ARM_MP_STARTUP_H__
#define __ACPI_ARM_MP_STARTUP_H__

#define ACPI_ARM_MP_MAILBOX_SIZE              SIZE_4KB
#define ACPI_ARM_MP_MAILBOX_ALIGN             SIZE_4KB

#define ACPI_ARM_MP_MAILBOX_FW_SIZE           SIZE_2KB

#define ACPI_ARM_MP_MAILBOX_CPU_ID_OFFSET     (ACPI_ARM_MP_MAILBOX_SIZE - 0x4)
#define ACPI_ARM_MP_MAILBOX_JUMP_ADDR_OFFSET  (ACPI_ARM_MP_MAILBOX_SIZE - 0x10)

#define ACPI_ARM_MP_MAILBOX_CPU_ID(Cluster,Core) \
  ((((Cluster) & 0xFFFF) << 16) | ((Core) & 0xFFFF))

#define ACPI_ARM_MP_MAILBOX_CPU_ID_INVALID    ACPI_ARM_MP_MAILBOX_CPU_ID(~0,~0)

#endif
