# Copyright Botez Luca 314CA 2022-2023

# Compiler setup
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Define targets
TARGET = image_editor
OBJS = image_editor.o

all: $(TARGET)

# Compile object file
image_editor.o: image_editor.c image_editor.h
	$(CC) $(CFLAGS) -c image_editor.c -o image_editor.o

# Link executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -lm -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
