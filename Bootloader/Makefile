##########################################################################
# User configuration and firmware specific object files	
##########################################################################

# The target, flash and ram of the LPC1xxx microprocessor.
# Use for the target the value: LPC11xx, LPC13xx or LPC17xx
TARGET = LPC11xx
FLASH = 32K
SRAM = 8K

# For USB support the LPC1xxx reserves 384 bytes from the sram,
# if you don't want to use the USB features, just use 0 here.
SRAM_USB = 0

VPATH = 
OBJS = $(OBJECT_PATH)/main.o

##########################################################################
# Debug settings
##########################################################################

# Set DEBUGBUILD to 'TRUE' for full debugging (larger, slower binaries), 
# or to 'FALSE' for release builds (smallest, fastest binaries)
#DEBUGBUILD = TRUE

##########################################################################
# IDE Flags (Keeps various IDEs happy)
##########################################################################

OPTDEFINES = -D __NEWLIB__

##########################################################################
# Project-specific files 
##########################################################################

# RSA Encryption/Descryption
VPATH += drivers/rsa
OBJS += $(OBJECT_PATH)/rsa.o

##########################################################################
# Library files 
##########################################################################
VPATH += core core/cpu core/gpio core/pmu
VPATH += core/systick core/timer16 core/timer32 core/uart
VPATH += core/libc core/wdt core/pwm core/iap
OBJS += $(OBJECT_PATH)/cpu.o $(OBJECT_PATH)/gpio.o $(OBJECT_PATH)/pmu.o $(OBJECT_PATH)/systick.o $(OBJECT_PATH)/timer16.o
OBJS += $(OBJECT_PATH)/timer32.o $(OBJECT_PATH)/uart.o $(OBJECT_PATH)/uart_buf.o $(OBJECT_PATH)/stdio.o $(OBJECT_PATH)/string.o $(OBJECT_PATH)/wdt.o $(OBJECT_PATH)/sysinit.o
OBJS += $(OBJECT_PATH)/iap.o
#@OBJS += $(OBJECT_PATH)/main-full.o
OBJS += $(OBJECT_PATH)/IntQueue.o
OBJS += $(OBJECT_PATH)/port.o
OBJS += $(OBJECT_PATH)/tasks.o

##########################################################################
# GNU GCC compiler prefix and location
##########################################################################

CROSS_COMPILE = ${COMPILER_PATH}arm-none-eabi-
AS = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
OUTFILE = $(BUILD_PATH)/firmware

##########################################################################
# GNU GCC compiler flags
##########################################################################
ROOT_PATH = .
INCLUDE_PATHS = -I$(ROOT_PATH)
INCLUDE_PATHS += -I$(FREERTOS_INCLUDE_PATH)
INCLUDE_PATHS += -I$(FREERTOS_DEMO_INCLUDE_PATH)
INCLUDE_PATHS += -I$(FREERTOS_TARGET_PATH)
INCLUDE_PATHS += -I$(FREERTOS_PORTABLE_PATH)
INCLUDE_PATHS += -I$(CMS_INCLUDE_PATH)

##########################################################################
# Startup files
##########################################################################

LD_PATH = lpc1xxx
LD_SCRIPT = $(LD_PATH)/linkscript.ld
LD_TEMP = memory.ld

ifeq (LPC11xx,$(TARGET))
  CORTEX_TYPE=m0
else
  CORTEX_TYPE=m3
endif

CPU_TYPE = cortex-$(CORTEX_TYPE)
VPATH += lpc1xxx
OBJS += $(OBJECT_PATH)/$(TARGET)_handlers.o $(OBJECT_PATH)/LPC1xxx_startup.o

##########################################################################
# Compiler settings, parameters and flags
##########################################################################
ifeq (TRUE,$(DEBUGBUILD))
  CFLAGS  = -c -g -O0 $(INCLUDE_PATHS) -Wall -mthumb -ffunction-sections -fdata-sections -fmessage-length=0 -mcpu=$(CPU_TYPE) -DTARGET=$(TARGET) -fno-builtin $(OPTDEFINES)
  ASFLAGS = -c -g -O0 $(INCLUDE_PATHS) -Wall -mthumb -ffunction-sections -fdata-sections -fmessage-length=0 -mcpu=$(CPU_TYPE) -D__ASSEMBLY__ -x assembler-with-cpp
else
  CFLAGS  = -c -g -Os $(INCLUDE_PATHS) -Wall -mthumb -ffunction-sections -fdata-sections -fmessage-length=0 -mcpu=$(CPU_TYPE) -DTARGET=$(TARGET) -fno-builtin $(OPTDEFINES)
  ASFLAGS = -c -g -Os $(INCLUDE_PATHS) -Wall -mthumb -ffunction-sections -fdata-sections -fmessage-length=0 -mcpu=$(CPU_TYPE) -D__ASSEMBLY__ -x assembler-with-cpp
endif
LDFLAGS = -nostartfiles -mcpu=$(CPU_TYPE) -mthumb -Wl,--gc-sections
LDLIBS  = -lm
OCFLAGS = --strip-unneeded

all: firmware

$(OBJECT_PATH)/main-full.o : $(SOURCE_PATH)/main-full.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/%.o : $(SOURCE_PATH)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/rsa.o : $(SOURCE_PATH)/drivers/rsa/rsa.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/cpu.o : $(SOURCE_PATH)/core/cpu/cpu.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/gpio.o : $(SOURCE_PATH)/core/gpio/gpio.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/pmu.o : $(SOURCE_PATH)/core/pmu/pmu.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/systick.o : $(SOURCE_PATH)/core/systick/systick.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/timer16.o : $(SOURCE_PATH)/core/timer16/timer16.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/timer32.o : $(SOURCE_PATH)/core/timer32/timer32.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/uart.o : $(SOURCE_PATH)/core/uart/uart.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/uart_buf.o : $(SOURCE_PATH)/core/uart/uart_buf.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/stdio.o : $(SOURCE_PATH)/core/libc/stdio.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/string.o : $(SOURCE_PATH)/core/libc/string.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/wdt.o : $(SOURCE_PATH)/core/wdt/wdt.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/iap.o : $(SOURCE_PATH)/core/iap/iap.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/timers.o : $(SOURCE_PATH)/core/iap/iap.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/tasks.o : $(FREERTOS_SOURCE_PATH)/tasks.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/IntQueue.o : $(FREERTOS_DEMO_SOURCE_PATH)/IntQueue.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/port.o : $(FREERTOS_PORTABLE_PATH)/port.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/LPC11xx_handlers.o : $(SOURCE_PATH)/lpc1xxx/LPC11xx_handlers.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJECT_PATH)/LPC1xxx_startup.o : $(SOURCE_PATH)/lpc1xxx/LPC1xxx_startup.c
	$(CC) $(CFLAGS) -o $@ $<

firmware: $(OBJS) $(SYS_OBJS)
	-@echo "MEMORY" > $(OBJECT_PATH)/$(LD_TEMP)
	-@echo "{" >> $(OBJECT_PATH)/$(LD_TEMP)
	-@echo "  flash(rx): ORIGIN = 0x00000000, LENGTH = $(FLASH)" >> $(OBJECT_PATH)/$(LD_TEMP)
	-@echo "  sram(rwx): ORIGIN = 0x10000000+$(SRAM_USB), LENGTH = $(SRAM)-$(SRAM_USB)" >> $(OBJECT_PATH)/$(LD_TEMP)
	-@echo "}" >> $(OBJECT_PATH)/$(LD_TEMP)
	-@echo "INCLUDE $(LD_SCRIPT)" >> $(OBJECT_PATH)/$(LD_TEMP)
	$(LD) $(LDFLAGS) -T $(OBJECT_PATH)/$(LD_TEMP) -o $(OUTFILE).elf $(OBJS) $(LDLIBS)
	-@echo ""
	$(SIZE) $(OUTFILE).elf
	-@echo ""
	$(OBJCOPY) $(OCFLAGS) -O binary $(OUTFILE).elf $(OUTFILE).bin
	$(OBJCOPY) $(OCFLAGS) -O ihex $(OUTFILE).elf $(OUTFILE).hex
  
#clean:
#	-@echo OBJECTS:$(OBJS)
#	-@echo OBJECT_PATH:$(OBJECT_PATH) BUILD_PATH:$(BUILD_PATH)
#	rm -f $(OBJS) $(OBJECT_PATH)/$(LD_TEMP) $(OUTFILE).elf $(OUTFILE).bin $(OUTFILE).hex
