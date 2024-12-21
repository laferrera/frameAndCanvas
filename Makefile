# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Platform-specific settings
ifeq ($(shell uname), Linux)
    OS = linux
    SCREEN_RENDER_SRC = screenRender_linux.c
else
    OS = mac
    SCREEN_RENDER_SRC = screenRender_mac.c
    SDL_LIBS = -lSDL2
endif

# Executable and source files
TARGET = frameAndCanvas
MAIN_SRC = frameAndCanvas.c
SCREEN_RENDER_HEADER = screenRender.h
OBJS = $(MAIN_SRC:.c=.o) $(SCREEN_RENDER_SRC:.c=.o)

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(SDL_LIBS)

%.o: %.c $(SCREEN_RENDER_HEADER)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean