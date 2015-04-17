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
  Implementation of initializing a network adapter.

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
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
  Resets a network adapter and allocates the transmit and receive buffers
  required by the network interface; optionally, also requests allocation of
  additional transmit and receive buffers.

  This function allocates the transmit and receive buffers required by the network
  interface. If this allocation fails, then EFI_OUT_OF_RESOURCES is returned.
  If the allocation succeeds and the network interface is successfully initialized,
  then EFI_SUCCESS will be returned.

  @param This               A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

  @param ExtraRxBufferSize  The size, in bytes, of the extra receive buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the
                            extra buffer, and the caller will not know if it is
                            actually being used.
  @param ExtraTxBufferSize  The size, in bytes, of the extra transmit buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the
                            extra buffer, and the caller will not know if it is
                            actually being used.

  @retval EFI_SUCCESS           The network interface was initialized.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_OUT_OF_RESOURCES  There was not enough memory for the transmit and
                                receive buffers.
  @retval EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       The increased buffer size feature is not supported.

**/
EFI_STATUS
EFIAPI
SnpInitialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN UINTN                       ExtraRxBufferSize OPTIONAL,
  IN UINTN                       ExtraTxBufferSize OPTIONAL
  )
{
  EFI_STATUS    Status;
  YUKON_DRIVER    *YukonDriver;
  EFI_TPL       OldTpl;

  DEBUG ((EFI_D_NET, "Yukon: SnpInitialize()\n"));
  if (This == NULL) {
    DEBUG ((EFI_D_NET, "Yukon: SnpInitialize() failed with Status = %r\n", EFI_INVALID_PARAMETER));
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

  default:
    Status = EFI_DEVICE_ERROR;
    goto ON_ERROR_RESTORE_TPL;
  }

  Status = gBS->CreateEvent (
                    EVT_NOTIFY_WAIT,
                    TPL_NOTIFY,
                    &SnpWaitForPacketNotify,
                    YukonDriver,
                    &YukonDriver->Snp.WaitForPacket
                    );

  if (EFI_ERROR (Status)) {
    YukonDriver->Snp.WaitForPacket = NULL;
    Status = EFI_DEVICE_ERROR;
    goto ON_ERROR_RESTORE_TPL;
  }
  //
  //
  //
  YukonDriver->Mode.MCastFilterCount      = 0;
  YukonDriver->Mode.ReceiveFilterSetting  = 0;
  ZeroMem (YukonDriver->Mode.MCastFilter, sizeof YukonDriver->Mode.MCastFilter);
  CopyMem (&YukonDriver->Mode.CurrentAddress, &YukonDriver->Mode.PermanentAddress, sizeof (EFI_MAC_ADDRESS));

  Status = mskc_init ();

  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (YukonDriver->Snp.WaitForPacket);
    goto ON_ERROR_RESTORE_TPL;
  }

  YukonDriver->Mode.State = EfiSimpleNetworkInitialized;
  goto ON_EXIT;

ON_ERROR_RESTORE_TPL:
  DEBUG ((EFI_D_NET, "Yukon: SnpInitialize() failed with Status = %r\n", Status));

ON_EXIT:
  gBS->RestoreTPL (OldTpl);
  return Status;
}
