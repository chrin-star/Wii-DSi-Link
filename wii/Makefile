.SUFFIXES:

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment")
endif
include $(DEVKITPPC)/wii_rules

TARGET := boot
BUILD := build
SOURCES := source
DATA :=
INCLUDES :=
LIBS := -lwiiuse -lbte -logc
LIBDIRS := $(PORTLIBS)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
OFILES := $(CFILES:.c=.o)

CFLAGS := -O2 -Wall $(MACHDEP) $(INCLUDE)
LDFLAGS := $(MACHDEP)

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
export OFILES
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBOGC_LIB)

.PHONY: all clean $(BUILD)
all: $(BUILD)

$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol
else
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(DEPENDS)
endif
