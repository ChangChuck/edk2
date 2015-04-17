/**
*
*  Copyright (c) 2011-2012, ARM Limited. All rights reserved.
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
    Event handler to check for available packet.

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed
and made available under the terms and conditions of the BSD License which
accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Snp.h"


/**
  Notification call back function for WaitForPacket event.

  @param  Event       EFI Event.
  @param  SnpPtr      Pointer to YUKON_DRIVER structure.

**/
VOID
EFIAPI
SnpWaitForPacketNotify (
  EFI_EVENT     Event,
  VOID          *SnpPtr
  )
{
  DEBUG ((EFI_D_NET, "Yukon: SnpWaitForPacketNotify()\n"));
  //
  // Do nothing if either parameter is a NULL pointer.
  //
  if (Event == NULL || SnpPtr == NULL) {
    return ;
  }
  //
  // Do nothing if the SNP interface is not initialized.
  //
  switch (((YUKON_DRIVER *) SnpPtr)->Mode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
  case EfiSimpleNetworkStarted:
  default:
    return ;
  }
//  if (PxeDbGetStatus.RxFrameLen != 0) {
//    gBS->SignalEvent (Event);
//  }
}
