# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Source files
TARGET = framebuffer
SOURCES = frameAndCanvas.c screenRender.c
OBJS = $(SOURCES:.c=.o)

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean