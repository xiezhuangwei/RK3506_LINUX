# SPDX-License-Identifier: BSD-3-Clause */

# Copyright (c) 2024 Rockchip Electronics Co., Ltd.

ROOT_PATH	:= ../../..

#############################################################################
# Cross compiler
#############################################################################
ifneq ($(wildcard ${ROOT_PATH}/../prebuilts/gcc/linux-x86/riscv64/xpack-riscv-none-embed-gcc-10.2.0-1.2-linux-x64/bin),)
CROSS_COMPILE	?= ${ROOT_PATH}/../prebuilts/gcc/linux-x86/riscv64/xpack-riscv-none-embed-gcc-10.2.0-1.2-linux-x64/bin/riscv-none-embed-
else
CROSS_COMPILE	?= riscv-none-embed-
endif

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

CPU		+= -DUSE_PLIC -DUSE_M_TIME -DNO_INIT -mcmodel=medany -msmall-data-limit=8 -L.
ASFLAGS		+= $(CPU) -c -x assembler-with-cpp -D__ASSEMBLY__
CFLAGS		+= $(CPU) -std=gnu99 -Os -g
CFLAGS		+= -ffunction-sections -fdata-sections -flto
LDFLAGS		+= -nostartfiles $(CPU) -Wl,--gc-sections --specs=nosys.specs -lc -lm -lgcc
OCFLAGS		= -R .note -R .note.gnu.build-id -R .comment -S

HAL_CFLAGS	+= -Wformat=2 -Wall -Wno-unused-parameter
HAL_CFLAGS	+= -Wstrict-prototypes -Wmissing-prototypes -Wimplicit-fallthrough
HAL_CFLAGS	+= -Werror

LINKER_SCRIPT	?= $(ROOT_PATH)/lib/CMSIS/Device/$(SOC)/Source/Templates/GCC/gcc_riscv.ld

#############################################################################
# Output files
#############################################################################
BIN		:= TestDemo.bin
ELF		:= TestDemo.elf
MAP		:= TestDemo.map

#############################################################################
# Options
#############################################################################
QUIET ?= n

ifeq ($(QUIET), y)
  Q := @
  S := -s
endif

#############################################################################
# Source code and include
#############################################################################
INCLUDES += \
-I"../src" \
-I"$(ROOT_PATH)/lib/CMSIS/RISCV/Include"

SRC_DIRS += \
    ../src \
    $(ROOT_PATH)/lib/CMSIS/Device/$(SOC)/Source/Templates/GCC

CORE_VENDOR ?= syntacore

ifeq ($(CORE_VENDOR), syntacore)
    ASFLAGS  += -DSYNTACORE
    CFLAGS   += -DSYNTACORE
    INCLUDES += \
    -I"$(ROOT_PATH)/lib/CMSIS/RISCV/Include/syntacore"
endif

export HAL_PATH := $(ROOT_PATH)
include $(HAL_PATH)/tools/build_lib.mk
SRC_DIRS += $(HAL_LIB_SRC)
INCLUDES += $(HAL_LIB_INC)
-include $(HAL_PATH)/test/build_test.mk
CFLAGS += -DUNITY_INCLUDE_CONFIG_H

SRCS += $(basename $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.[cS])))
OBJS += $(addsuffix .o,$(basename $(SRCS)))
CFLAGS += $(INCLUDES)
ASFLAGS += $(INCLUDES)

HAL_SRCS += $(basename $(foreach dir,$(HAL_LIB_SRC) $(ROOT_PATH)/lib/CMSIS/Device/$(SOC)/Source/Templates/GCC,$(wildcard $(dir)/*.[cS])))
HAL_OBJS += $(addsuffix .o,$(basename $(HAL_SRCS)))
$(HAL_OBJS): CFLAGS += $(HAL_CFLAGS)

all: $(BIN)

$(ELF): $(OBJS) $(LINKER_SCRIPT)
	$(Q) $(CC) $(OBJS) $(LDFLAGS) $(CFLAGS) -T$(LINKER_SCRIPT) -Wl,-Map=$(MAP),-cref -o $@

$(BIN): $(ELF)
	$(Q) $(OBJCOPY) $(OCFLAGS) -O binary $< $@

clean:
	rm -f $(OBJS)
	rm -f TestDemo*

.PHONY: all clean
