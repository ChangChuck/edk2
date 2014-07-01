## @file
#
# Copyright (c) 2014, ARM Ltd. All rights reserved.<BR>
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

PP  = $(GCC48_AARCH64_PREFIX)gcc -E
ASM = $(GCC48_AARCH64_PREFIX)gcc
CC  = $(GCC48_AARCH64_PREFIX)gcc
LD  = $(GCC48_AARCH64_PREFIX)ld

PP_FLAGS  = -include $(DEST_DIR_DEBUG)/AutoGen.h
ASM_FLAGS = -c -x assembler -g -imacros $(DEST_DIR_DEBUG)/AutoGen.h
CC_FLAGS = -c -g -mcmodel=large -mlittle-endian -Wall -Werror
LD_FLAGS = -Ttext=0x0 --emit-relocs -nostdlib -e AArch64ToAArch32ShimStart

INCS = -I$(WORKSPACE)/MdePkg/Include -I$(WORKSPACE)/MdePkg/Include/AArch64 -I$(WORKSPACE)/ArmPkg/Include -I$(MODULE_DIR)

OBJECT_FILES = $(OUTPUT_DIR)/AArch64ToAArch32Shim.o \
               $(OUTPUT_DIR)/AArch64ToAArch32Vector.o \
               $(OUTPUT_DIR)/AArch64ToAArch32Trap.o

.PHONY: all clean

all: $(OBJECT_FILES)
	# Create ELF file from the object files
	$(LD) $(LD_FLAGS) -o DEBUG/$(BASE_NAME).elf $(OBJECT_FILES)

	# Convert ELF file to EFI file
	# The module is defined as SEC to make GenFw patch the FD file to jump at the entrypoint of this module.
	"GenFw" -e SEC -o DEBUG/$(BASE_NAME).efi DEBUG/$(BASE_NAME).elf
	cp -f DEBUG/$(BASE_NAME).efi $(OUTPUT_DIR)
	cp -f DEBUG/$(BASE_NAME).efi $(BIN_DIR)


$(OUTPUT_DIR)/AArch64ToAArch32Shim.o: $(MODULE_DIR)/AArch64ToAArch32Shim.S
	$(PP) $(PP_FLAGS) $(INCS) $(MODULE_DIR)/AArch64ToAArch32Shim.S > $(OUTPUT_DIR)/AArch64ToAArch32Shim.i
	Trim --trim-long --source-code -o $(OUTPUT_DIR)/AArch64ToAArch32Shim.iii $(OUTPUT_DIR)/AArch64ToAArch32Shim.i
	$(ASM) $(ASM_FLAGS) -o $@ $(INCS) $(OUTPUT_DIR)/AArch64ToAArch32Shim.iii

$(OUTPUT_DIR)/AArch64ToAArch32Vector.o: $(MODULE_DIR)/AArch64ToAArch32Vector.S
	$(PP) $(PP_FLAGS) $(INCS) $(MODULE_DIR)/AArch64ToAArch32Vector.S > $(OUTPUT_DIR)/AArch64ToAArch32Vector.i
	Trim --trim-long --source-code -o $(OUTPUT_DIR)/AArch64ToAArch32Vector.iii $(OUTPUT_DIR)/AArch64ToAArch32Vector.i
	$(ASM) $(ASM_FLAGS) -o $@ $(INCS) $(OUTPUT_DIR)/AArch64ToAArch32Vector.iii

$(OUTPUT_DIR)/AArch64ToAArch32Trap.o: $(MODULE_DIR)/AArch64ToAArch32Trap.c
	$(CC) $(CC_FLAGS) -o $@ $(INCS) $(MODULE_DIR)/AArch64ToAArch32Trap.c

clean:
	rm -Rf $(OUTPUT_DIR)
