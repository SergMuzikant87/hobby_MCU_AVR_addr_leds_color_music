#######################################################################################################################
#	Файл автоматизации компиляции сборки проекта и загрузки прошивки в микроконтроллер.
#	разработал Николаев Сергей (г. Кулебаки).
#	Все права защищены.
#######################################################################################################################

TARGET_MCU := atmega328p
SOURCE_FORDER := source/
OUTPUT_FORDER := output/
VERSION := v1_0_0_DEBUG
PROGRAMM_NAME := programm_$(VERSION)
ELF := $(OUTPUT_FORDER)$(PROGRAMM_NAME).elf
HEX := $(OUTPUT_FORDER)$(PROGRAMM_NAME).hex

TARGET := -mmcu=$(TARGET_MCU)
FLAGS := -Wall -g2 -gstabs -O2

build:
	avr-gcc $(FLAGS) $(TARGET) -c $(SOURCE_FORDER)main.c -o $(OUTPUT_FORDER)main.o
	avr-gcc $(FLAGS) $(TARGET) -c $(SOURCE_FORDER)addr_leds_driver.S -o $(OUTPUT_FORDER)addr_leds_driver.o
	avr-gcc $(FLAGS) $(TARGET) -c $(SOURCE_FORDER)matrix.c -o $(OUTPUT_FORDER)matrix.o
	avr-gcc $(FLAGS) $(TARGET) -c $(SOURCE_FORDER)analog.c -o $(OUTPUT_FORDER)analog.o
	avr-gcc $(FLAGS) $(TARGET) -c $(SOURCE_FORDER)watchdog.c -o $(OUTPUT_FORDER)watchdog.o

	avr-gcc -mmcu=$(TARGET_MCU) $(FLAGS) $(OUTPUT_FORDER)*.o -o $(ELF)
	@avr-size --format=avr $(ELF)
	avr-objcopy -O ihex $(ELF) $(HEX)

download:
	@echo Programm $(TARGET_MCU)
	avrdude -p $(TARGET_MCU) -c usbasp -u -U flash:w:$(HEX):a 
