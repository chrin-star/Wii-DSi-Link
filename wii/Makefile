# Wii homebrew Makefile
TARGET := boot
BUILD := build
SOURCES := source
DATA :=
INCLUDES :=

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
OFILES := $(CFILES:.c=.o)

CFLAGS := -O2 -Wall -Wextra
LDFLAGS :=
LIBS := -lwiiuse -lbte -logc

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
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol
else
DEPENDS := $(OFILES:.o=.d)
include $(DEVKITPPC)/wii_rules
-include $(DEPENDS)
endif
