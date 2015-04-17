/**
*
*  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
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

/** @file
    Implementation of shutting down a network adapter.

Copyright (c) 2004 - 2007, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed
and made available under the terms and conditions of the BSD License which
accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Snp.h"
#include "if_msk.h"


/**
  Resets a network adapter and leaves it in a state that is safe for another
  driver to initialize.

  This function releases the memory buffers assigned in the Initialize() call.
  Pending transmits and receives are lost, and interrupts are cleared and disabled.
  After this call, only the Initialize() and Stop() calls may be used. If the
  network interface was successfully shutdown, then EFI_SUCCESS will be returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.

  @param  This  A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

  @retval EFI_SUCCESS           The network interface was shutdown.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.

**/
EFI_STATUS
EFIAPI
SnpShutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
{
  EFI_STATUS    Status;
  YUKON_DRIVER    *YukonDriver;
  EFI_TPL       OldTpl;

  DEBUG ((EFI_D_NET, "Yukon: SnpShutdown()\n"));
  //
  // Get pointer to SNP driver instance for *This.
  //
  if (This == NULL) {
    DEBUG ((EFI_D_NET, "Yukon: SnpShutdown() failed with Status = %r\n", EFI_INVALID_PARAMETER));
    return EFI_INVALID_PARAMETER;
  }

  YukonDriver = YUKON_DEV_FROM_THIS (This);

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  //
  // Return error if the SNP is not initialized.
  //
  switch (YukonDriver->Mode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    Status = EFI_NOT_STARTED;
    goto ON_ERROR_RESTORE_TPL;

  default:
    Status = EFI_DEVICE_ERROR;
    goto ON_ERROR_RESTORE_TPL;
  }

  mskc_shutdown ();
  YukonDriver->Mode.State = EfiSimpleNetworkStarted;
  Status = EFI_SUCCESS;

  YukonDriver->Mode.State                 = EfiSimpleNetworkStarted;
  YukonDriver->Mode.ReceiveFilterSetting  = 0;

  YukonDriver->Mode.MCastFilterCount      = 0;
  YukonDriver->Mode.ReceiveFilterSetting  = 0;
  ZeroMem (YukonDriver->Mode.MCastFilter, sizeof YukonDriver->Mode.MCastFilter);
  CopyMem (
    &YukonDriver->Mode.CurrentAddress,
    &YukonDriver->Mode.PermanentAddress,
    sizeof (EFI_MAC_ADDRESS)
    );

  gBS->CloseEvent (YukonDriver->Snp.WaitForPacket);
  goto ON_EXIT;

ON_ERROR_RESTORE_TPL:
  DEBUG ((EFI_D_NET, "Yukon: SnpShutdown() failed with Status = %r\n", Status));

ON_EXIT:
  gBS->RestoreTPL (OldTpl);
  return Status;
}
