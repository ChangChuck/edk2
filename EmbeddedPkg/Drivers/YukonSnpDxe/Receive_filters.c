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
    Implementation of managing the multicast receive filters of a network
    interface.

Copyright (c) 2004 - 2007, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under the
terms and conditions of the BSD License which accompanies this distribution. The
full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/



#include "Snp.h"
#include "if_msk.h"


/**
  Manages the multicast receive filters of a network interface.

  This function is used enable and disable the hardware and software receive
  filters for the underlying network device.
  The receive filter change is broken down into three steps:
  * The filter mask bits that are set (ON) in the Enable parameter are added to
    the current receive filter settings.
  * The filter mask bits that are set (ON) in the Disable parameter are subtracted
    from the updated receive filter settings.
  * If the resulting receive filter setting is not supported by the hardware a
    more liberal setting is selected.
  If the same bits are set in the Enable and Disable parameters, then the bits
  in the Disable parameter takes precedence.
  If the ResetMCastFilter parameter is TRUE, then the multicast address list
  filter is disabled (irregardless of what other multicast bits are set in the
  Enable and Disable parameters). The SNP->Mode->MCastFilterCount field is set
  to zero. The Snp->Mode->MCastFilter contents are undefined.
  After enabling or disabling receive filter settings, software should verify
  the new settings by checking the Snp->Mode->ReceiveFilterSettings,
  Snp->Mode->MCastFilterCount and Snp->Mode->MCastFilter fields.
  Note: Some network drivers and/or devices will automatically promote receive
    filter settings if the requested setting can not be honored. For example, if
    a request for four multicast addresses is made and the underlying hardware
    only supports two multicast addresses the driver might set the promiscuous
    or promiscuous multicast receive filters instead. The receiving software is
    responsible for discarding any extra packets that get through the hardware
    receive filters.
    Note: Note: To disable all receive filter hardware, the network driver must
      be Shutdown() and Stopped(). Calling ReceiveFilters() with Disable set to
      Snp->Mode->ReceiveFilterSettings will make it so no more packets are
      returned by the Receive() function, but the receive hardware may still be
      moving packets into system memory before inspecting and discarding them.
      Unexpected system errors, reboots and hangs can occur if an OS is loaded
      and the network devices are not Shutdown() and Stopped().
  If ResetMCastFilter is TRUE, then the multicast receive filter list on the
  network interface will be reset to the default multicast receive filter list.
  If ResetMCastFilter is FALSE, and this network interface allows the multicast
  receive filter list to be modified, then the MCastFilterCnt and MCastFilter
  are used to update the current multicast receive filter list. The modified
  receive filter list settings can be found in the MCastFilter field of
  EFI_SIMPLE_NETWORK_MODE. If the network interface does not allow the multicast
  receive filter list to be modified, then EFI_INVALID_PARAMETER will be returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.
  If the receive filter mask and multicast receive filter list have been
  successfully updated on the network interface, EFI_SUCCESS will be returned.

  @param This             A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param Enable           A bit mask of receive filters to enable on the network
                          interface.
  @param Disable          A bit mask of receive filters to disable on the network
                          interface. For backward compatibility with EFI 1.1
                          platforms, the EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST bit
                          must be set when the ResetMCastFilter parameter is TRUE.
  @param ResetMCastFilter Set to TRUE to reset the contents of the multicast
                          receive filters on the network interface to their
                          default values.
  @param MCastFilterCnt   Number of multicast HW MAC addresses in the new MCastFilter
                          list. This value must be less than or equal to the
                          MCastFilterCnt field of EFI_SIMPLE_NETWORK_MODE.
                          This field is optional if ResetMCastFilter is TRUE.
  @param MCastFilter      A pointer to a list of new multicast receive filter HW
                          MAC addresses. This list will replace any existing
                          multicast HW MAC address list. This field is optional
                          if ResetMCastFilter is TRUE.

  @retval EFI_SUCCESS            The multicast receive filter list was updated.
  @retval EFI_NOT_STARTED        The network interface has not been started.
  @retval EFI_INVALID_PARAMETER  One or more of the following conditions is TRUE:
                                 * This is NULL
                                 * There are bits set in Enable that are not set
                                   in Snp->Mode->ReceiveFilterMask
                                 * There are bits set in Disable that are not set
                                   in Snp->Mode->ReceiveFilterMask
                                 * Multicast is being enabled (the
                                   EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST bit is
                                   set in Enable, it is not set in Disable, and
                                   ResetMCastFilter is FALSE) and MCastFilterCount
                                   is zero
                                 * Multicast is being enabled and MCastFilterCount
                                   is greater than Snp->Mode->MaxMCastFilterCount
                                 * Multicast is being enabled and MCastFilter is NULL
                                 * Multicast is being enabled and one or more of
                                   the addresses in the MCastFilter list are not
                                   valid multicast MAC addresses
  @retval EFI_DEVICE_ERROR       One or more of the following conditions is TRUE:
                                 * The network interface has been started but has
                                   not been initialized
                                 * An unexpected error was returned by the
                                   underlying network driver or device
  @retval EFI_UNSUPPORTED        This function is not supported by the network
                                 interface.

**/
EFI_STATUS
EFIAPI
SnpReceiveFilters (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN UINT32                      Enable,
  IN UINT32                      Disable,
  IN BOOLEAN                     ResetMCastFilter,
  IN UINTN                       MCastFilterCnt,  OPTIONAL
  IN EFI_MAC_ADDRESS             *MCastFilter     OPTIONAL
  )
{
  EFI_STATUS    Status;
  YUKON_DRIVER    *YukonDriver;
  EFI_TPL       OldTpl;
  UINT32        newReceiveFilter;

  DEBUG ((EFI_D_NET, "Yukon: SnpReceiveFilters()\n"));
  if (This == NULL) {
    DEBUG ((EFI_D_NET, "Yukon: SnpReceiveFilters() failed with Status = %r\n", EFI_INVALID_PARAMETER));
    return EFI_INVALID_PARAMETER;
  }

  YukonDriver = YUKON_DEV_FROM_THIS (This);

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

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
  //
  // check if we are asked to enable or disable something that the NIC
  // does not even support!
  //
  newReceiveFilter = (YukonDriver->Mode.ReceiveFilterSetting | Enable) & ~Disable;
  if ((newReceiveFilter & ~YukonDriver->Mode.ReceiveFilterMask) != 0) {
    DEBUG ((EFI_D_NET, "Yukon: SnpReceiveFilters() NIC does not support Enable = 0x%x, Disable = 0x%x\n", Enable, Disable));
    Status = EFI_INVALID_PARAMETER;
    goto ON_ERROR_RESTORE_TPL;
  }

  if (ResetMCastFilter) {
    newReceiveFilter &= ~(EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST & YukonDriver->Mode.ReceiveFilterMask);
    MCastFilterCnt = 0;
    MCastFilter    = NULL;
  } else {
    if (MCastFilterCnt != 0) {
      if ((MCastFilterCnt > YukonDriver->Mode.MaxMCastFilterCount) ||
          (MCastFilter == NULL)) {

        DEBUG ((EFI_D_NET, "Yukon: SnpReceiveFilters() NIC does not support MCastFilterCnt = %d (Max = %d)\n", MCastFilterCnt,
          YukonDriver->Mode.MaxMCastFilterCount));
        Status = EFI_INVALID_PARAMETER;
        goto ON_ERROR_RESTORE_TPL;
      }
    }
  }

  if (newReceiveFilter == YukonDriver->Mode.ReceiveFilterSetting && !ResetMCastFilter && MCastFilterCnt == 0) {
    Status = EFI_SUCCESS;
    goto ON_EXIT;
  }

  if ((Enable & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) != 0 && MCastFilterCnt == 0) {
    Status = EFI_INVALID_PARAMETER;
    goto ON_ERROR_RESTORE_TPL;
  }

  YukonDriver->Mode.ReceiveFilterSetting = newReceiveFilter;
  mskc_rxfilter (YukonDriver->Mode.ReceiveFilterSetting, MCastFilterCnt, MCastFilter);

  Status = EFI_SUCCESS;
  goto ON_EXIT;

ON_ERROR_RESTORE_TPL:
  DEBUG ((EFI_D_NET, "Yukon: SnpReceiveFilters() failed with Status = %r\n", Status));

ON_EXIT:
  gBS->RestoreTPL (OldTpl);
  return Status;
}
