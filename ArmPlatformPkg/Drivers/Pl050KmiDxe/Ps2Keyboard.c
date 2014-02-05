/** @file

  PS/2 Keyboard driver. Routines that interacts with callers,
  conforming to EFI driver model

Copyright (c) 2006 - 2012, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2014, ARM Limited. All rights reserved.
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ps2Keyboard.h"

/**
  Free the waiting key notify list.

  @param ListHead  Pointer to list head

  @retval EFI_INVALID_PARAMETER  ListHead is NULL
  @retval EFI_SUCCESS            Sucess to free NotifyList
**/
EFI_STATUS
KbdFreeNotifyList (
  IN OUT LIST_ENTRY           *ListHead
  )
{
  KEYBOARD_CONSOLE_IN_EX_NOTIFY *NotifyNode;

  if (ListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  while (!IsListEmpty (ListHead)) {
    NotifyNode = CR (
                   ListHead->ForwardLink,
                   KEYBOARD_CONSOLE_IN_EX_NOTIFY,
                   NotifyEntry,
                   KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                   );
    RemoveEntryList (ListHead->ForwardLink);
    gBS->FreePool (NotifyNode);
  }

  return EFI_SUCCESS;
}

/**
  The module Entry Point for module Ps2Keyboard.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
Pl050DxeEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                                Status;
  EFI_STATUS                                Status1;
  KEYBOARD_CONSOLE_IN_DEV                   *ConsoleIn;
  UINT8                                     Data;

  gImageHandle = ImageHandle;

  //
  // Allocate private data
  //
  ConsoleIn = AllocateZeroPool (sizeof (KEYBOARD_CONSOLE_IN_DEV));
  if (ConsoleIn == NULL) {
    Status      = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
  //
  // Setup the device instance
  //
  ConsoleIn->Signature              = KEYBOARD_CONSOLE_IN_DEV_SIGNATURE;
  ConsoleIn->Handle                 = ImageHandle;
  (ConsoleIn->ConIn).Reset          = KeyboardEfiReset;
  (ConsoleIn->ConIn).ReadKeyStroke  = KeyboardReadKeyStroke;
  ConsoleIn->DevicePath             = NULL;

  ConsoleIn->ConInEx.Reset               = KeyboardEfiResetEx;
  ConsoleIn->ConInEx.ReadKeyStrokeEx     = KeyboardReadKeyStrokeEx;
  ConsoleIn->ConInEx.SetState            = KeyboardSetState;
  ConsoleIn->ConInEx.RegisterKeyNotify   = KeyboardRegisterKeyNotify;
  ConsoleIn->ConInEx.UnregisterKeyNotify = KeyboardUnregisterKeyNotify;

  InitializeListHead (&ConsoleIn->NotifyList);

  //
  // Setup the WaitForKey event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  KeyboardWaitForKey,
                  ConsoleIn,
                  &((ConsoleIn->ConIn).WaitForKey)
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
  //
  // Setup the WaitForKeyEx event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  KeyboardWaitForKeyEx,
                  ConsoleIn,
                  &(ConsoleIn->ConInEx.WaitForKeyEx)
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
  // Setup a periodic timer, used for reading keystrokes at a fixed interval
  //
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  KeyboardTimerHandler,
                  ConsoleIn,
                  &ConsoleIn->TimerEvent
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  Status = gBS->SetTimer (
                  ConsoleIn->TimerEvent,
                  TimerPeriodic,
                  KEYBOARD_TIMER_INTERVAL
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  //
  // Reset the keyboard device
  //
  Status = ConsoleIn->ConInEx.Reset (&ConsoleIn->ConInEx, FALSE);
  if (EFI_ERROR (Status)) {
    Status      = EFI_DEVICE_ERROR;
    goto ErrorExit;
  }

  ConsoleIn->ControllerNameTable = NULL;
  AddUnicodeString2 (
    "eng",
    gPs2KeyboardComponentName.SupportedLanguages,
    &ConsoleIn->ControllerNameTable,
    L"PS/2 Keyboard Device",
    TRUE
    );
  AddUnicodeString2 (
    "en",
    gPs2KeyboardComponentName2.SupportedLanguages,
    &ConsoleIn->ControllerNameTable,
    L"PS/2 Keyboard Device",
    FALSE
    );


  //
  // Install protocol interfaces for the keyboard device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gEfiSimpleTextInProtocolGuid,
                  &ConsoleIn->ConIn,
                  &gEfiSimpleTextInputExProtocolGuid,
                  &ConsoleIn->ConInEx,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  return Status;

ErrorExit:

  if ((ConsoleIn != NULL) && (ConsoleIn->ConIn.WaitForKey != NULL)) {
    gBS->CloseEvent (ConsoleIn->ConIn.WaitForKey);
  }

  if ((ConsoleIn != NULL) && (ConsoleIn->TimerEvent != NULL)) {
    gBS->CloseEvent (ConsoleIn->TimerEvent);
  }
  if ((ConsoleIn != NULL) && (ConsoleIn->ConInEx.WaitForKeyEx != NULL)) {
    gBS->CloseEvent (ConsoleIn->ConInEx.WaitForKeyEx);
  }
  KbdFreeNotifyList (&ConsoleIn->NotifyList);
  if ((ConsoleIn != NULL) && (ConsoleIn->ControllerNameTable != NULL)) {
    FreeUnicodeStringTable (ConsoleIn->ControllerNameTable);
  }
  //
  // Since there will be no timer handler for keyboard input any more,
  // exhaust input data just in case there is still keyboard data left
  //
  if (ConsoleIn != NULL) {
    Status1 = EFI_SUCCESS;
    while (!EFI_ERROR (Status1) && (Status != EFI_DEVICE_ERROR)) {
      Status1 = KeyboardRead (ConsoleIn, &Data);;
    }
  }

  if (ConsoleIn != NULL) {
    gBS->FreePool (ConsoleIn);
  }

  return Status;
}

