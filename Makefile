CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

EXE = build/gsh
SOURCES = main.c builtins.c utils.c
OBJECTS = $(SOURCES:%.c=build/%.o)
BUILD_DIR = build

.PHONY: all run debug test clean

all: $(EXE)

run: $(EXE)
	./build/gsh

debug: $(EXE)
	lldb ./build/gsh

$(EXE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@


TEST_EXE = build/test_utils
TEST_SRC = test/utils/main.c
TEST_OBJS = build/utils.o

test: $(TEST_EXE)
	./$(TEST_EXE)

$(TEST_EXE): $(TEST_SRC) $(TEST_OBJS)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -rf $(BUILD_DIR) $(SHELL_BIN) $(TEST_UTILS_BIN)