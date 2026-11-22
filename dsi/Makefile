# BlocksDS Makefile for DSi Wi-Fi with DSi-mode support

export BLOCKSDS ?= /opt/blocksds/core
export WONDERFUL_TOOLCHAIN ?= /opt/wonderful
ARM_NONE_EABI_PATH ?= $(WONDERFUL_TOOLCHAIN)/toolchain/gcc-arm-none-eabi/bin/

NAME := WiiDSiLink
GAME_TITLE := Wii-DSi-Link
GAME_SUBTITLE := DSi Wi-Fi test
GAME_AUTHOR := chrin-star
GAME_ICON := icon.gif

SOURCEDIRS := source
INCLUDEDIRS :=
DEFINES :=

ARM7ELF := $(BLOCKSDS)/sys/arm7/main_core/arm7_dswifi_maxmod.elf

LIBS := -ldswifi9 -lnds9
LIBDIRS := $(BLOCKSDS)/libs/dswifi \
           $(BLOCKSDS)/libs/libnds

BUILDDIR := build/$(NAME)
ELF := build/$(NAME).elf
MAP := build/$(NAME).map
ROM := $(NAME).nds

PREFIX := $(ARM_NONE_EABI_PATH)arm-none-eabi-
CC := $(PREFIX)gcc
LD := $(PREFIX)gcc
OBJDUMP := $(PREFIX)objdump

DEFINES += -D__NDS__ -D__BLOCKSDS__ -DARM9
ARCH := -mthumb -mcpu=arm946e-s+nofp
SPECS := $(BLOCKSDS)/sys/crts/ds_arm9.specs
WARNFLAGS := -Wall

LIBS += -lc

INCLUDEFLAGS := $(foreach path,$(INCLUDEDIRS),-I$(path)) \
                $(foreach path,$(LIBDIRS),-I$(path)/include)
LIBDIRSFLAGS := $(foreach path,$(LIBDIRS),-L$(path)/lib)

CFLAGS += $(WARNFLAGS) $(INCLUDEFLAGS) $(DEFINES) $(ARCH) -O2 \
          -ffunction-sections -fdata-sections -specs=$(SPECS)
LDFLAGS += $(ARCH) $(LIBDIRSFLAGS) -Wl,-Map,$(MAP) $(DEFINES) \
           -Wl,--start-group $(LIBS) -Wl,--end-group -specs=$(SPECS)

SOURCES_C := $(shell find -L $(SOURCEDIRS) -name "*.c")
OBJS := $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_C)))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean
all: $(ROM)

$(ROM): $(ELF)
	$(BLOCKSDS)/tools/ndstool/ndstool -c $@ -7 $(ARM7ELF) -9 $(ELF) \
		-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE);$(GAME_AUTHOR)"

$(ELF): $(OBJS)
	@mkdir -p $(@D)
	$(LD) -o $@ $(OBJS) $(LDFLAGS)

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -rf $(ROM) $(ELF) $(MAP) build

-include $(DEPS)
