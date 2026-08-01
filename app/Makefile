DIR_Config	 = c/lib/Config
DIR_EPD		 = c/lib/e-Paper
DIR_FONTS	 = c/lib/Fonts
DIR_GUI		 = c/lib/GUI
DIR_BIN		 = ./bin

EPD = NULL

OBJ_C = $(wildcard ${DIR_EPD}/*.c ${DIR_GUI}/*.c ${DIR_FONTS}/*.c ./*.c )

OBJ_O = $(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))
RPI_DEV_C = $(wildcard $(DIR_BIN)/dev_hardware_SPI.o $(DIR_BIN)/RPI_gpiod.o $(DIR_BIN)/DEV_Config.o )

DEBUG = -D DEBUG

USELIB_RPI = USE_LGPIO_LIB

LIB_RPI=-Wl,--gc-sections
ifeq ($(USELIB_RPI), USE_BCM2835_LIB)
	LIB_RPI += -lbcm2835 -lm 
else ifeq ($(USELIB_RPI), USE_WIRINGPI_LIB)
	LIB_RPI += -lwiringPi -lm 
else ifeq ($(USELIB_RPI), USE_LGPIO_LIB)
	LIB_RPI += -llgpio -lm
else ifeq ($(USELIB_RPI), USE_DEV_LIB)
	LIB_RPI += -lgpiod -lm 
endif
DEBUG_RPI = -D $(USELIB_RPI) -D RPI

TARGET = four_rectangles
CC = gcc
MSG = -g -O -ffunction-sections -fdata-sections -Wall
CFLAGS += $(MSG) -D $(EPD)

RPI_epd:${OBJ_O}
	echo $(@)
	$(CC) $(CFLAGS) -D RPI $(OBJ_O) $(RPI_DEV_C) -o $(TARGET) $(LIB_RPI) $(DEBUG)

$(shell mkdir -p $(DIR_BIN))

${DIR_BIN}/%.o:./%.c
	$(CC) $(CFLAGS) -c	$< -o $@ -I $(DIR_Config) -I $(DIR_GUI) -I $(DIR_EPD) -I $(DIR_FONTS) $(DEBUG)

${DIR_BIN}/%.o:$(DIR_EPD)/%.c
	$(CC) $(CFLAGS) -c	$< -o $@ -I $(DIR_Config) $(DEBUG)

${DIR_BIN}/%.o:$(DIR_FONTS)/%.c 
	$(CC) $(CFLAGS) -c	$< -o $@ $(DEBUG)

${DIR_BIN}/%.o:$(DIR_GUI)/%.c
	$(CC) $(CFLAGS) -c	$< -o $@ -I $(DIR_Config) $(DEBUG)

RPI_DEV:
ifeq ($(USELIB_RPI), USE_DEV_LIB)
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c	 $(DIR_Config)/dev_hardware_SPI.c -o $(DIR_BIN)/dev_hardware_SPI.o $(LIB_RPI) $(DEBUG)
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c	 $(DIR_Config)/DEV_Config.c -o $(DIR_BIN)/DEV_Config.o $(LIB_RPI) $(DEBUG)
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c	 $(DIR_Config)/RPI_gpiod.c -o $(DIR_BIN)/RPI_gpiod.o $(LIB_RPI) $(DEBUG)
else
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c	 $(DIR_Config)/dev_hardware_SPI.c -o $(DIR_BIN)/dev_hardware_SPI.o $(LIB_RPI) $(DEBUG)
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c	 $(DIR_Config)/DEV_Config.c -o $(DIR_BIN)/DEV_Config.o $(LIB_RPI) $(DEBUG)
endif

clean :
	rm $(DIR_BIN)/*.* 
	rm $(TARGET) 

all: RPI_DEV RPI_epd
