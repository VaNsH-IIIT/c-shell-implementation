CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm

# Directories
SRCDIR = src
INCLUDEDIR = include

# Source files
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/prompt.c $(SRCDIR)/parser.c $(SRCDIR)/builtin.c $(SRCDIR)/hop.c $(SRCDIR)/reveal.c $(SRCDIR)/log.c $(SRCDIR)/executor.c $(SRCDIR)/background.c $(SRCDIR)/sequential.c $(SRCDIR)/activities.c $(SRCDIR)/ping.c $(SRCDIR)/signals.c $(SRCDIR)/job_control.c $(SRCDIR)/process_mgmt.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = shell.out

# Default target
all: $(TARGET)

# Build the shell
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Compile object files
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Force rebuild
rebuild: clean all

.PHONY: all clean rebuild