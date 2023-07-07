LIB = ../build/debug/libskjson.so
BIN = skJSON_test
LEAKBIN = skJSON_leak_test
BUILD_DIR = build
OBJDIR := $(BUILD_DIR)/obj

CFILES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,$(OBJDIR)/%.o,$(CFILES))
DEPFILES := $(patsubst %.o,%.d,$(OBJECTS))
INPUT_FILES := $(wildcard *.json)

CC = gcc
DEPFLAGS = -MD -MP
SANITIZER_FLAGS = -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
CFLAGS = -Werror -Wall -Wextra -Wpedantic -std=c99 $(DEPFLAGS)

all: $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN): $(OBJECTS) $(LIB)
	$(CC) $(CFLAGS) -g -L../build/debug $^ -lskjson -lcriterion -o $@

$(OBJDIR)/%.o:%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR): $(BUILD_DIR)
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@
	$(foreach json,$(INPUT_FILES),cp $(json) $(BUILD_DIR)/.;)

test: all
	./$(BUILD_DIR)/$(BIN);

test-valgrind: all
	valgrind \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		./$(BUILD_DIR)/$(BIN)

test-sanitizer: $(BUILD_DIR)/$(LEAKBIN)
	./$(BUILD_DIR)/$(LEAKBIN)

$(BUILD_DIR)/$(LEAKBIN): $(OBJECTS) $(LIB)
	$(CC) $(CFLAGS) $(SANITIZER_FLAGS) $^ -o $@ -g -L../build/debug -lskjson -lcriterion

clean:
	rm -rf $(BUILD_DIR)

-include $(DEP_FILES)

.PHONY: all test clean test-sanitizer test-valgrind
