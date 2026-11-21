# Nintendo DS/DSi homebrew Makefile
TARGET := WiiDSiLink
BUILD := build
SOURCES := source
DATA :=
INCLUDES :=

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
OFILES := $(CFILES:.c=.o)

CFLAGS := -O2 -Wall -Wextra
LDFLAGS :=
LIBS :=

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

.PHONY: all clean
all: $(BUILD)

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).nds
else
DEPENDS := $(OFILES:.o=.d)
include $(DEVKITARM)/ds_rules
-include $(DEPENDS)
endif
