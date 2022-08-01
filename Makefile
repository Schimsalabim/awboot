# Target
TARGET = awboot
TOPDIR=$(shell pwd)

# Build revision
BUILD_REVISION_H = "build_revision.h"
BUILD_REVISION_D = "BUILD_REVISION"

SRCS =  main.c board.c lib/debug.c lib/xformat.c lib/div.c lib/fdt.c lib/string.c 

INCLUDE_DIRS= -I . -I include -I lib 
LIB_DIR= -L ./
LIBS= -lm 

include	arch/arch.mk
include	lib/fatfs/fatfs.mk

CFLAGS += -march=armv7-a -mtune=cortex-a7  -mthumb-interwork -mno-unaligned-access -mabi=aapcs-linux
CFLAGS += -Os -std=gnu99 -Wall -g $(INCLUDES)

ASFLAGS += -march=armv7-a -mtune=cortex-a7 -mthumb-interwork -mno-unaligned-access -mabi=aapcs-linux
ASFLAGS += -Os -std=gnu99 -Wall -g $(INCLUDES)

LDFLAGS += -march=armv7-a -mtune=cortex-a7 -mthumb-interwork -mno-unaligned-access -mabi=aapcs-linux
LDFLAGS += -T ./arch/arm32/mach-t113s3/link.ld -nostdlib

STRIP=arm-none-eabi-strip
CC=arm-none-eabi-gcc
SIZE=arm-none-eabi-size
OBJCOPY=arm-none-eabi-objcopy
HOSTCC=gcc
HOSTSTRIP=strip
DATE=/bin/date
CAT=/bin/cat
ECHO=/bin/echo
WORKDIR=$(/bin/pwd)
MAKE=make
OPENOCD = openocd
MKSUNXI = ./tools/mksunxi

# Objects
EXT_OBJS =
BUILD_OBJS = $(SRCS:.c=.o) 
BUILD_OBJSA = $(ASRCS:.S=.o) 
OBJS = $(BUILD_OBJSA) $(BUILD_OBJS) $(EXT_OBJS)

LBC_VERSION = $(shell grep LBC_APP_VERSION main.h | cut -d '"' -f 2)"-"$(shell /bin/cat .build_revision)

all: begin build_revision build mkboot

begin:
	@echo "---------------------------------------------------------------"
	@echo -n "Compiler version: "
	@$(CC) -v 2>&1 | tail -1

build_revision:
	@/bin/expr `/bin/cat .build_revision` + 1 > .build_revision
	@echo "// Generated by make, DO NOT EDIT" > $(BUILD_REVISION_H)
	@echo "#ifndef __$(BUILD_REVISION_D)_H__" >> $(BUILD_REVISION_H)
	@echo "#define $(BUILD_REVISION_D)" `/bin/cat .build_revision` >> $(BUILD_REVISION_H)
	@echo "#endif" >> $(BUILD_REVISION_H)
	@echo "Build revision:" `/bin/cat .build_revision`
	@echo "---------------------------------------------------------------"


.SILENT:

build: $(TARGET).elf $(TARGET).bin
#$(STRIP) $(TARGET)

.SECONDARY : $(TARGET)
.PRECIOUS : $(OBJS)
$(TARGET).elf: $(OBJS)
	echo "  LD    $@"
	$(CC) $^ -o $@ $(LIB_DIR) $(LIBS) $(LDFLAGS) -Wl,-Map,$(TARGET).map

$(TARGET).bin: $(TARGET).elf
	@echo OBJCOPY $@
	$(OBJCOPY) -O binary $< $@
	$(SIZE) $(TARGET).elf

%.o : %.c
	echo "  CC    $<"
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)

%.o : %.S
	echo "  CC    $<"
	$(CC) $(ASFLAGS) -c $< -o $@ $(INCLUDE_DIRS)

clean:
	echo "RM  $(OBJS)"
	rm -f $(OBJS)
	rm -f *.o
	echo "  CC    $<"
	rm -f $(TARGET)
	rm -f $(TARGET).bin
	rm -f $(TARGET).map
	rm -f $(TARGET).elf
	rm -f .deps
	rm -f .dep/*.d

mkboot:
	$(MKSUNXI) $(TOPDIR)/$(TARGET).bin

-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
