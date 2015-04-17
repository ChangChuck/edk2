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
    Implementation of collecting the statistics on a network interface.

Copyright (c) 2004 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed
and made available under the terms and conditions of the BSD License which
accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include "Snp.h"


/**
  Resets or collects the statistics on a network interface.

  This function resets or collects the statistics on a network interface. If the
  size of the statistics table specified by StatisticsSize is not big enough for
  all the statistics that are collected by the network interface, then a partial
  buffer of statistics is returned in StatisticsTable, StatisticsSize is set to
  the size required to collect all the available statistics, and
  EFI_BUFFER_TOO_SMALL is returned.
  If StatisticsSize is big enough for all the statistics, then StatisticsTable
  will be filled, StatisticsSize will be set to the size of the returned
  StatisticsTable structure, and EFI_SUCCESS is returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.
  If Reset is FALSE, and both StatisticsSize and StatisticsTable are NULL, then
  no operations will be performed, and EFI_SUCCESS will be returned.
  If Reset is TRUE, then all of the supported statistics counters on this network
  interface will be reset to zero.

  @param This            A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param Reset           Set to TRUE to reset the statistics for the network interface.
  @param StatisticsSize  On input the size, in bytes, of StatisticsTable. On output
                         the size, in bytes, of the resulting table of statistics.
  @param StatisticsTable A pointer to the EFI_NETWORK_STATISTICS structure that
                         contains the statistics. Type EFI_NETWORK_STATISTICS is
                         defined in "Related Definitions" below.

  @retval EFI_SUCCESS           The requested operation succeeded.
  @retval EFI_NOT_STARTED       The Simple Network Protocol interface has not been
                                started by calling Start().
  @retval EFI_BUFFER_TOO_SMALL  StatisticsSize is not NULL and StatisticsTable is
                                NULL. The current buffer size that is needed to
                                hold all the statistics is returned in StatisticsSize.
  @retval EFI_BUFFER_TOO_SMALL  StatisticsSize is not NULL and StatisticsTable is
                                not NULL. The current buffer size that is needed
                                to hold all the statistics is returned in
                                StatisticsSize. A partial set of statistics is
                                returned in StatisticsTable.
  @retval EFI_INVALID_PARAMETER StatisticsSize is NULL and StatisticsTable is not
                                NULL.
  @retval EFI_DEVICE_ERROR      The Simple Network Protocol interface has not
                                been initialized by calling Initialize().
  @retval EFI_DEVICE_ERROR      An error was encountered collecting statistics
                                from the NIC.
  @retval EFI_UNSUPPORTED       The NIC does not support collecting statistics
                                from the network interface.

**/
EFI_STATUS
EFIAPI
SnpStatistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN BOOLEAN                     Reset,
  IN OUT UINTN                   *StatisticsSize, OPTIONAL
  IN OUT EFI_NETWORK_STATISTICS  *StatisticsTable OPTIONAL
  )
{
  EFI_STATUS        Status;
  YUKON_DRIVER        *YukonDriver;
  EFI_TPL           OldTpl;

  DEBUG ((EFI_D_NET, "Yukon: SnpStatistics()\n"));
  //
  // Get pointer to SNP driver instance for *This.
  //
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
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
    goto ON_EXIT;

  case EfiSimpleNetworkStarted:
  default:
    Status = EFI_DEVICE_ERROR;
    goto ON_EXIT;
  }

  //
  // Error Checking
  //

  if (!Reset && (StatisticsSize == NULL) && (StatisticsTable == NULL)) {
    Status = EFI_SUCCESS;
    goto ON_EXIT;
  }

  if ((StatisticsSize == NULL) && (StatisticsTable != NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto ON_EXIT;
  }

  if ((StatisticsSize != NULL) && (StatisticsTable == NULL)) {
    *StatisticsSize = sizeof (EFI_NETWORK_STATISTICS);
    Status = EFI_BUFFER_TOO_SMALL;
    goto ON_EXIT;
  }

  if ((StatisticsSize != NULL) && (StatisticsTable != NULL)) {
    if (*StatisticsSize < sizeof (EFI_NETWORK_STATISTICS)) {
      if (*StatisticsSize == 0) {
        Status = EFI_BUFFER_TOO_SMALL;
        // Note: From here on, the Status value must be preserved.
      } else {
        // FixMe: Return partial statistics for the available size and also set
        //        Status = EFI_BUFFER_TOO_SMALL;
        //        but for now it is unsupported.
        Status = EFI_UNSUPPORTED;
        // Note: From here on, the Status value must be preserved.
      }
      *StatisticsSize = sizeof (EFI_NETWORK_STATISTICS);
    } else {
      // FixMe: Return full statistics and also set
      //        Status = EFI_SUCCESS;
      //        but for now it is unsupported.
      Status = EFI_UNSUPPORTED;
    }
  }

  if (Reset == TRUE) {
    // FixMe: Reset all statistics;

    // Preserve any previous errors else return success.
    if (!EFI_ERROR (Status)) {
      // FixMe: Should return success
      //        Status = EFI_SUCCESS;
      //        but for now it is unsupported.
      Status = EFI_UNSUPPORTED;
    }
  }

ON_EXIT:
  gBS->RestoreTPL (OldTpl);

  return Status;
}
