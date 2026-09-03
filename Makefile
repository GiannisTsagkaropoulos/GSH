CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

EXE = build/gsh
SOURCES = main.c builtins.c utils.c
OBJECTS = $(SOURCES:%.c=build/%.o)

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
