# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall

# Output executable name
TARGET = run

# Source files
SRCS = \
	src\main.c \
	src\program\program.c \
	src\program\utils.c \
	src\keylogger\keylogger.c \
	src\client\client.c \
	src\client\commands.c 

# Build target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lwininet

# Clean target to remove compiled files
clean:
	del .\$(TARGET).exe

	