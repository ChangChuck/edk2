/** @file

  This file defines the protocol to run the FDT installation process.

  Copyright (c) 2015, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __INSTALL_FDT_H___
#define __INSTALL_FDT_H___

extern EFI_GUID gInstallFdtProtocolGuid;

typedef struct _INSTALL_FDT_PROTOCOL INSTALL_FDT_PROTOCOL;

/**

  Run the FDT installation process (see FdtPlatfromDxe for further
  information).

  @param  This  A pointer to a INSTALL_FDT_PROTOCOL

  @retval  EFI_SUCCESS            The FDT was installed.
  @retval  EFI_NOT_FOUND          Failed to locate a protocol or a file.
  @retval  EFI_INVALID_PARAMETER  Invalid device path.
  @retval  EFI_UNSUPPORTED        Device path not supported.
  @retval  EFI_OUT_OF_RESOURCES   An allocation failed.

**/
typedef
EFI_STATUS
(*INSTALL_FDT_RUN)(
  IN  INSTALL_FDT_PROTOCOL  *This
);

// The INSTALL_FDT_PROTOCOL allows to run the FDT installation process
// that consists of the retrieving of the FDT and its installation into
// the UEFI Configuration Table for use by EFI applications.

struct _INSTALL_FDT_PROTOCOL {
  INSTALL_FDT_RUN  Run;
};

#endif /* __INSTALL_FDT_H___ */
