#
#  Copyright (c) 2014-2015, ARM Limited. All rights reserved.
#  
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

from arm_ds.debugger_v1 import Debugger
from arm_ds.debugger_v1 import DebugException

import re, sys, getopt

def tt_entry_to_str(entry, level):
    str = ""
    if (level == 3):
        if (entry & 0x3) == 0x3:
            str += "BLOCK3 addr:" + hex(entry & (0xFFFFFFFFF << 12)) + " index:" + hex((entry >> 2) & 0x7) + " entry:" + hex(entry)
        else:
            str += "INVALID"
    else:
        if (entry & 0x3) == 0x3:
            str += "TABLE"
        elif (entry & 0x3) == 0x1:
            str += "BLOCK addr:" + hex(entry & (0xFFFFFFFFF << 12)) + " index:" + hex((entry >> 2) & 0x7) + " entry:" + hex(entry)
        else:
            str += "INVALID"
    # Print Secure bit
    if (entry & (1 << 5)) != 0:
        str += " (Non Secure)"
    return str

def print_tt(table, entry_count, level):
    for i in range(entry_count):
        indent = ""
        j = level
        while j != 0:
            indent += '\t'
            j = j - 1
        entry = ec.getMemoryService().readMemory64(table + (i * 8)) # & 0xFFFFFFFFFFFFFFFF
        if (level != 3) and ((entry & 0x3) == 0x3):
            print indent + "TABLE at " + hex(entry & (0xFFFFFFFFF << 12))
            print_tt(entry & (0xFFFFFFFFF << 12), 512, level+1)
        else:
            print indent + "(" + hex(table + (i * 8)) + ")" + tt_entry_to_str(entry, level)

# Debugger object for accessing the debugger
debugger = Debugger()

# Initialisation commands
ec = debugger.getExecutionContext(0)
ec.getExecutionService().stop()
ec.getExecutionService().waitForStop()
# in case the execution context reference is out of date
ec = debugger.getExecutionContext(0)

exception_level = "EL2"

# Print Index
try:
    mair = ec.getRegisterService().getValue('MAIR_%s' % exception_level)
except Exception:
    mair = ec.getRegisterService().getValue('System::Memory::MAIR_%s' % exception_level)

for i in range(0,7):
    mair_index = (mair >> (i * 8)) & 0xFF

    mair_index_high = mair_index >> 4
    if mair_index_high == 0:
        mair_high_str = "Device memory"
    elif mair_index_high == 0x4:
        mair_high_str = "Normal Memory, Outer Non-Cacheable"
    elif (mair_index_high & 0xC) == 0x0:
        mair_high_str = "Normal Memory, Outer Write-through transient"
    elif (mair_index_high & 0xC) == 0x4:
        mair_high_str = "Normal Memory, Outer Write-back transient"
    elif (mair_index_high & 0xC) == 0x8:
        mair_high_str = "Normal Memory, Outer Write-through non-transient"
    elif (mair_index_high & 0xC) == 0xC:
        mair_high_str = "Normal Memory, Outer Write-back non-transient"

    mair_index_low = mair_index & 0xF
    if mair_index_low == 0:
        mair_device_str = "Device-nGnRnE memory"
        mair_normal_str = "UNPREDICTABLE"
    elif mair_index_low == 0x4:
        mair_device_str = "Device-nGnRE memory"
        mair_normal_str = "Normal memory, Inner Non-Cacheable"
    elif mair_index_low == 0x8:
        mair_device_str = "Device-nGRE memory"
        mair_normal_str = "Normal Memory, Inner Write-through non-transient"
    elif (mair_index_low & 0xC) == 0xC:
        mair_device_str = "Device-GRE memory"
        mair_normal_str = "Normal Memory, Inner Write-back non-transient"
    else:
        mair_device_str = "UNPREDICTABLE"
        mair_normal_str = "<TODO>"

    if mair_index_high == 0:
        print "Index[%d]: %s - %s (%x)" % (i, mair_high_str, mair_device_str, mair_index_low)
    else:
        print "Index[%d]: %s - %s" % (i, mair_high_str, mair_normal_str)

try:
    ttbr = ec.getRegisterService().getValue('TTBR0_%s' % exception_level)
except Exception:
    ttbr = ec.getRegisterService().getValue('System::Memory::TTBR0_%s' % exception_level)

table = int(ttbr)
print "TTBR: " + hex(table)

print_tt(table, 512, 1)
