# Calendar Display System Makefile
# Builds the C-based application for Raspberry Pi with E-Paper display

# Project configuration
TARGET = calendar_display
CC = gcc
CFLAGS = -g -O2 -Wall -Wextra -std=c11

# Include paths
INC_DIRS = -I./src -I./lib -I./lib/Config -I./lib/e-Paper -I./lib/GUI -I./lib/Fonts

# Library paths
LIB_DIRS = 

# Libraries to link
LIBS = -lcurl -lmicrohttpd -llgpio -lm -lpthread

# Source files
SRC_FILES = \
	src/main.c \
	src/config.c \
	src/caldav_client.c \
	src/display.c \
	src/web_server.c \
	lib/Config/DEV_Config.c \
	lib/Config/RPI_gpiod.c \
	lib/Config/dev_hardware_SPI.c \
	lib/e-Paper/EPD_2in15g.c \
	lib/GUI/GUI_Paint.c \
	lib/GUI/GUI_BMPfile.c \
	lib/Fonts/font8.c \
	lib/Fonts/font12.c \
	lib/Fonts/font16.c \
	lib/Fonts/font20.c \
	lib/Fonts/font24.c \
	lib/Fonts/font12CN.c \
	lib/Fonts/font24CN.c

# Object files
OBJ_FILES = $(SRC_FILES:.c=.o)

# Compiler flags
CFLAGS += $(INC_DIRS) -DRPI -DUSE_DEV_LIB -DEPD=epd2in15g

# Build rules
all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(TARGET) $(OBJ_FILES)

# Install dependencies
install-deps:
	sudo apt-get update
	sudo apt-get install -y gcc make libcurl4-openssl-dev libmicrohttpd-dev libgpiod-dev

# Run the application
run: $(TARGET)
	./$(TARGET)

# Build and run
build-run: clean all run

# Create a debug build
debug: CFLAGS += -DDEBUG -g3
debug: all

.PHONY: all clean install-deps run build-run debug
