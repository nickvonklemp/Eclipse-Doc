CC = arm-none-eabi-gcc
DEVICE_INCLUDES = -I ../../STM32F0xx_StdPeriph_Lib_V1.3.1/Libraries/CMSIS/Device/ST/STM32F0xx/Include
ARM_INCLUDES = -I ../../STM32F0xx_StdPeriph_Lib_V1.3.1/Libraries/CMSIS/Include
CFLAGS = ${DEVICE_INCLUDES} ${ARM_INCLUDES} -Wall -mcpu=cortex-m0 -mthumb -g -std=c99

AS = arm-none-eabi-as
ASFLAGS = -mcpu=cortex-m0 -mthumb -g

LD = arm-none-eabi-ld
LINKER_FILE = stm32f0_linker.ld
LDFLAGS = -T $(LINKER_FILE) -nostartfiles

OBJS = \
       main.o \
       startup_stm32f051.o \
       system_stm32f0xx.o \
       lcd_stm32f0.o \
       temp_sensor_lib.o \
       mountain.o \
       eeprom_lib.o \
       interrupts.o

main.elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o main.elf $(OBJS)
	
main.o: main.c temp_sensor_lib.h temp_sensor_lib.c lcd_stm32f0.h lcd_stm32f0.c eeprom_lib.h eeprom_lib.c mountain.c mountain.h
startup_stm32f051.o: startup_stm32f051.s
temp_sensor_lib.o: temp_sensor_lib.c temp_sensor_lib.h
eeprom_lib.o: eeprom_lib.c eeprom_lib.h
lcd_stm32f0.o: lcd_stm32f0.c lcd_stm32f0.h
interrupts.o: interrupts.c
system_stm32f0xx.o: system_stm32f0xx.c
mountain.o: mountain.c mountain.h

clean:
	$(RM) *.o *.elf *.bin
