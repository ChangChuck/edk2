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
    Implementation of stopping a network interface.

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
  Changes the state of a network interface from "started" to "stopped."

  This function stops a network interface. This call is only valid if the network
  interface is in the started state. If the network interface was successfully
  stopped, then EFI_SUCCESS will be returned.

  @param  This                    A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL
                                  instance.


  @retval EFI_SUCCESS             The network interface was stopped.
  @retval EFI_NOT_STARTED         The network interface has not been started.
  @retval EFI_INVALID_PARAMETER   This parameter was NULL or did not point to a
                                  valid EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR        The command could not be sent to the network
                                  interface.
  @retval EFI_UNSUPPORTED         This function is not supported by the network
                                  interface.

**/
EFI_STATUS
EFIAPI
SnpStop (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
{
  EFI_STATUS    Status;
  YUKON_DRIVER    *YukonDriver;
  EFI_TPL       OldTpl;

  DEBUG ((EFI_D_NET, "Yukon: SnpStop()\n"));
  if (This == NULL) {
    DEBUG ((EFI_D_NET, "Yukon: SnpStop() failed with Status = %r\n", EFI_INVALID_PARAMETER));
    return EFI_INVALID_PARAMETER;
  }

  YukonDriver = YUKON_DEV_FROM_THIS (This);

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (YukonDriver->Mode.State) {
  case EfiSimpleNetworkStarted:
    break;

  case EfiSimpleNetworkStopped:
    Status = EFI_NOT_STARTED;
    goto ON_ERROR_RESTORE_TPL;

  case EfiSimpleNetworkInitialized:
  default:
    Status = EFI_DEVICE_ERROR;
    goto ON_ERROR_RESTORE_TPL;
  }

  mskc_detach ();
  YukonDriver->Mode.State = EfiSimpleNetworkStopped;
  ZeroMem (&YukonDriver->Mode.CurrentAddress, sizeof (EFI_MAC_ADDRESS));
  Status = EFI_SUCCESS;
  goto ON_EXIT;

ON_ERROR_RESTORE_TPL:
  DEBUG ((EFI_D_NET, "Yukon: SnpStop() failed with Status = %r\n", Status));

ON_EXIT:
  gBS->RestoreTPL (OldTpl);
  return Status;
}
