/** @file
*  SNP implementation specific to Yukon device family
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
  Implementation of driver entry point and driver binding protocol.

Copyright (c) 2004 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed
and made available under the terms and conditions of the BSD License which
accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Snp.h"
#include "if_msk.h"
#include <Library/NetLib.h>


/**
  Test to see if this driver supports ControllerHandle. This service
  is called by the EFI boot service ConnectController(). In
  order to make drivers as small as possible, there are a few calling
  restrictions for this service. ConnectController() must
  follow these calling restrictions. If any other agent wishes to call
  Supported() it must also follow these calling restrictions.

  @param  This                Protocol instance pointer.
  @param  ControllerHandle    Handle of device to test.
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval EFI_ALREADY_STARTED This driver is already running on this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS
EFIAPI
SimpleNetworkDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS              Status;
  EFI_PCI_IO_PROTOCOL     *PciIo;

  //
  // Test that the PCI IO Protocol is attached to the controller handle and no other driver is consuming it
  //
  Status = gBS->OpenProtocol (
                Controller,
                &gEfiPciIoProtocolGuid,
                (VOID **) &PciIo,
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_BY_DRIVER
                );
  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  //
  // Test whether the controller is on a supported NIC
  //
  Status = mskc_probe (PciIo);
  if (EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
       Controller,
       &gEfiPciIoProtocolGuid,
       This->DriverBindingHandle,
       Controller
       );

  DEBUG ((EFI_D_INFO, "Yukon: SimpleNetworkDriverSupported(): Status = %d, Controller = %p\n", Status, Controller));

ON_EXIT:
  return Status;
}

EFI_STATUS
InstallSNPProtocol (
  IN EFI_HANDLE                     Controller,
  IN EFI_PCI_IO_PROTOCOL*           PciIo
  )
{
  YUKON_DRIVER    *YukonDriver;
  EFI_STATUS    Status;

  YukonDriver = (YUKON_DRIVER *)AllocateZeroPool (sizeof (YUKON_DRIVER));
  if (YukonDriver == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  YukonDriver->PciIo      = PciIo;
  YukonDriver->Signature  = YUKON_DRIVER_SIGNATURE;

  EfiInitializeLock (&YukonDriver->Lock, TPL_NOTIFY);

  YukonDriver->Snp.Revision       = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  YukonDriver->Snp.Start          = SnpStart;
  YukonDriver->Snp.Stop           = SnpStop;
  YukonDriver->Snp.Initialize     = SnpInitialize;
  YukonDriver->Snp.Reset          = SnpReset;
  YukonDriver->Snp.Shutdown       = SnpShutdown;
  YukonDriver->Snp.ReceiveFilters = SnpReceiveFilters;
  YukonDriver->Snp.StationAddress = SnpStationAddress;
  YukonDriver->Snp.Statistics     = SnpStatistics;
  YukonDriver->Snp.MCastIpToMac   = SnpMcastIpToMac;
  YukonDriver->Snp.NvData         = SnpNvData;
  YukonDriver->Snp.GetStatus      = SnpGetStatus;
  YukonDriver->Snp.Transmit       = SnpTransmit;
  YukonDriver->Snp.Receive        = SnpReceive;
  YukonDriver->Snp.WaitForPacket  = NULL;

  YukonDriver->Snp.Mode           = &YukonDriver->Mode;

  //
  //  Initialize simple network protocol mode structure
  //
  YukonDriver->Mode.State               = EfiSimpleNetworkStopped;
  YukonDriver->Mode.HwAddressSize       = NET_ETHER_ADDR_LEN;
  YukonDriver->Mode.MediaHeaderSize     = sizeof (ETHER_HEAD);
  YukonDriver->Mode.MaxPacketSize       = MAX_SUPPORTED_PACKET_SIZE;
  YukonDriver->Mode.NvRamAccessSize     = 0;
  YukonDriver->Mode.NvRamSize           = 0;
  YukonDriver->Mode.IfType              = NET_IFTYPE_ETHERNET;
  YukonDriver->Mode.MaxMCastFilterCount = 16;
  YukonDriver->Mode.MCastFilterCount    = 0;

  //
  //  Set broadcast address
  //
  SetMem (&YukonDriver->Mode.BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  YukonDriver->Mode.MediaPresentSupported = FALSE;
  YukonDriver->Mode.MacAddressChangeable = FALSE;
  YukonDriver->Mode.MultipleTxSupported = FALSE;
  YukonDriver->Mode.ReceiveFilterMask = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST;
  YukonDriver->Mode.ReceiveFilterSetting = 0;

  YukonDriver->Mode.MediaPresent = TRUE;

  //
  //  Install the SNP protocol
  //
  Status = gBS->InstallProtocolInterface (
                  &Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &(YukonDriver->Snp)
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_NET, "Yukon: InstallSNPProtocol() failed: Status = %r, Controller = %p\n", Status, Controller));
    FreePool (YukonDriver);
  }
  return Status;
}

/**
  Start this driver on ControllerHandle. This service is called by the
  EFI boot service ConnectController(). In order to make
  drivers as small as possible, there are a few calling restrictions for
  this service. ConnectController() must follow these
  calling restrictions. If any other agent wishes to call Start() it
  must also follow these calling restrictions.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to.
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          This driver is added to ControllerHandle
  @retval EFI_ALREADY_STARTED  This driver is already running on ControllerHandle
  @retval other                This driver does not support this device

**/
EFI_STATUS
EFIAPI
SimpleNetworkDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_PCI_IO_PROTOCOL*    PciIo;
  EFI_STATUS              Status;

  DEBUG ((EFI_D_NET, "Yukon: SimpleNetworkDriverStart(): Controller=%p\n", Controller));

  //
  // Open the PciIo Protocol
  //
  Status = gBS->OpenProtocol (
                Controller,
                &gEfiPciIoProtocolGuid,
                (VOID **) &PciIo,
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_BY_DRIVER
                );
  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  Status = InstallSNPProtocol (Controller, PciIo);
  if (EFI_ERROR (Status)) {
    goto CLOSE_PCIIO;
  }

  goto ON_EXIT;

CLOSE_PCIIO:
  gBS->CloseProtocol (
       Controller,
       &gEfiPciIoProtocolGuid,
       This->DriverBindingHandle,
       Controller
       );

ON_EXIT:
  DEBUG ((EFI_D_NET, "Yukon: SimpleNetworkDriverStart() Complete: Status = %d, Controller = %p\n", Status, Controller));
  return Status;
}

/**
  Stop this driver on ControllerHandle. This service is called by the
  EFI boot service DisconnectController(). In order to
  make drivers as small as possible, there are a few calling
  restrictions for this service. DisconnectController()
  must follow these calling restrictions. If any other agent wishes
  to call Stop() it must also follow these calling restrictions.

  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
SimpleNetworkDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_NETWORK_PROTOCOL   *SnpProtocol;
  YUKON_DRIVER                  *YukonDriver;

  DEBUG ((EFI_D_NET, "Yukon: SimpleNetworkDriverStop()\n"));

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol (
             Controller,
             &gEfiSimpleNetworkProtocolGuid,
             (VOID **) &SnpProtocol,
             This->DriverBindingHandle,
             Controller,
             EFI_OPEN_PROTOCOL_GET_PROTOCOL
             );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  YukonDriver = YUKON_DEV_FROM_THIS (SnpProtocol);

  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  &YukonDriver->Snp
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

/*  Status = gBS->CloseProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  This->DriverBindingHandle,
                  Controller
                  );
*/
  FreePool (YukonDriver);

  return Status;
}

//
// Simple Network Protocol Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL mSimpleNetworkDriverBinding = {
  SimpleNetworkDriverSupported,
  SimpleNetworkDriverStart,
  SimpleNetworkDriverStop,
  0xa,
  NULL,
  NULL
};

/**
  The SNP driver entry point.

  @param ImageHandle       The driver image handle.
  @param SystemTable       The system table.

  @retval EFI_SUCCESS      Initialization routine has found and initialized
                           hardware successfully.
  @retval Other            Return value from HandleProtocol for
                           DeviceIoProtocol or LoadedImageProtocol

**/
EFI_STATUS
EFIAPI
InitializeYukonDriver (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  DEBUG ((EFI_D_NET, "Yukon: InitializeYukonDriver()\n"));
  EFI_STATUS Status = EfiLibInstallDriverBindingComponentName2 (
                       ImageHandle,
                       SystemTable,
                       &mSimpleNetworkDriverBinding,
                       NULL,
                       &gSimpleNetworkComponentName,
                       &gSimpleNetworkComponentName2
                       );

  return Status;
}
