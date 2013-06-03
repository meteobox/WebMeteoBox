###############################################################################
# Makefile for the project WebMeteoBox
#
# WebMeteoBox source code. ver 1.0.0.0
# Created by Zager V.B. and Krylov A.I. 2012-2013 Dubna (c)
# 
# Project home page: http://meteobox.tk
# Email: valery@jinr.ru
#
#
# 03.06.2013 
#
###############################################################################
## General Flags
PROJECT = MeteoBOX
MCU = atmega128
TARGET = MeteoBOX.elf
CC = avr-gcc.exe


# Processor frequency.
#     This will define a symbol, F_CPU, in all source code files equal to the 
#     processor frequency. You can then use this symbol in your source code to 
#     calculate timings. Do NOT tack on a 'UL' at the end, this will be done
#     automatically to create a 32-bit value in your source code.
#     Typical values are:
#         F_CPU =  1000000
#         F_CPU =  1843200
#         F_CPU =  2000000
#         F_CPU =  3686400
#         F_CPU =  4000000
#         F_CPU =  7372800
F_CPU =  8000000
#         F_CPU = 11059200
#         F_CPU = 14745600
#         F_CPU = 16000000
#         F_CPU = 18432000
#         F_CPU = 20000000
#		  F_CPU = 14745600

CDEFS = -DF_CPU=$(F_CPU)UL

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += $(CDEFS)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -Os -funsigned-char 
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

##EXTMEMOPTS = -Wl,-Tdata=0x1100,--defsym=__heap_end=0x8000
##-------------------------------------------------------------


#---------------- External Memory Options ---------------- 

# 64 KB of external RAM, starting after internal RAM (ATmega128!), 
# used for variables (.data/.bss) and heap (malloc()). 
#EXTMEMOPTS = -Wl,--section-start,.data=0x801100,--defsym=__heap_end=0x80ffff 

# 64 KB of external RAM, starting after internal RAM (ATmega128!), 
# only used for heap (malloc()). 
#EXTMEMOPTS = -Wl,--defsym=__heap_start=0x801100,--defsym=__heap_end=0x80ffff 




## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=MeteoBOX.map
LDFLAGS +=  $(EXTMEMOPTS)
LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"c:\WINAVR-PROJECTS\CPU-wiznet-5300\MeteoBOX\main\ex03_webserver\..\..\mcu" 

## Objects that must be built in order to link
OBJECTS = dns.o evb.o dataflash.o socket.o w5300.o main.o task_config.o wiz_util.o protocol.o term.o DHT.o i2csoft.o OWISWBitFunctions.o OWIHighLevelFunctions.o ../common_files/OWIcrc.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) MeteoBOX.hex MeteoBOX.eep MeteoBOX.lss size

## Compile
evb.o: ../../../evb/evb.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

dataflash.o: ../../../evb/dataflash.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

#romfile.o: ../../../evb/romfile.c
	#$(CC) $(INCLUDES) $(CFLAGS) -c  $<

socket.o: ../../../iinchip/socket.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

w5300.o: ../../../iinchip/w5300.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<
	
task_config.o: ../task_config.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

#delay.o: ../../../mcu/delay.c
#	$(CC) $(INCLUDES) $(CFLAGS) -c  $<	

i2csoft.o: ../i2csoft.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<	

term.o: ../term.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<	

DHT.o: ../DHT.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<	

OWISWBitFunctions.o: ../OWISWBitFunctions.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

OWIHighLevelFunctions.o: ../OWIHighLevelFunctions.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

common_files/OWIcrc.o: ../common_files/OWIcrc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

wiz_util.o: ../../../util/wiz_util.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

#sockutil.o: ../../../util/sockutil.c
#	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

#lcd.o: ../../../evb/lcd.c
#	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

#tc77.o: ../../../evb/tc77.c
#	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

#timer.o: ../../../mcu/timer.c
#	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

dns.o: ../../../inet/dns.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

#dhcp_task.o: ../dhcp_task.c
#	$(CC) $(INCLUDES) $(CFLAGS) -c  $<
	
protocol.o: ../protocol.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<		

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) MeteoBOX.elf dep MeteoBOX.hex MeteoBOX.eep MeteoBOX.lss MeteoBOX.map


## 	C:\WinAVR\bin\avrdude.exe -C C:\WinAVR\bin\avrdude.conf -p m128 -P lpt1 -c stk200  -U flash:w:C:\WINAVR-PROJECTS\CPU-wiznet-5300\CPU-02_USB_ver1.1\main\ex03_webserver\default\boot_and_firm1.1.hex:i 
## 	C:\WinAVR\bin\avrdude.exe -C C:\WinAVR\bin\avrdude.conf -p m128 -P lpt1 -c stk200  -U flash:w:$(PROJECT).hex:i 
##  -V do not verify
##  Help: http://www.ladyada.net/images/avrtutorial/avrtest.jpg
program : $(PROJECT).hex
	C:\WinAVR-20090313\bin\avrdude.exe -C C:\WinAVR-20090313\bin\avrdude.conf -p m128 -P lpt1 -c stk200  -U flash:w:$(PROJECT).hex:i 

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

