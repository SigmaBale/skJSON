LIB = ../build/debug/libskjson.so
BIN = parser_test
LEAKBIN = parser_leak_test
BUILD_DIR = build
SRCDIR = .
OBJDIR := $(BUILD_DIR)/obj

CFILES := $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.c))
OBJECTS := $(patsubst %.c,$(OBJDIR)/%.o,$(CFILES))
DEPFILES := $(patsubst %.o,%.d,$(OBJECTS))

CC = gcc
DEPFLAGS = -MD -MP
SANITIZER_FLAGS = -fsanitize=address -O0 -fno-omit-frame-pointer
CFLAGS = -Werror -Wall -Wextra -Wpedantic -ansi $(DEPFLAGS)

all: $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN): $(OBJECTS) $(LIB)
	$(CC) $(CFLAGS) -g -L../build/debug $^ -lskjson -lcriterion -o $@

$(OBJDIR)/%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR): $(BUILD_DIR)
	mkdir $@

$(BUILD_DIR):
	mkdir $@

test: $(BUILD_DIR)/$(BIN)
	./$(BUILD_DIR)/$(BIN);

test-leak: $(BUILD_DIR)/$(LEAKBIN)
	./$(BUILD_DIR)/$(LEAKBIN)

$(BUILD_DIR)/$(LEAKBIN): $(OBJECTS) $(LIB)
	$(CC) $(CFLAGS) $(SANITIZER_FLAGS) -g -L../build/debug $^ -lskjson -lcriterion -o $@

clean:
	rm -rf $(BUILD_DIR)/$(BIN) $(BUILD_DIR)/$(LEAKBIN) $(DEPFILES) $(OBJECTS)

-include $(DEP_FILES)

.PHONY: all test clean test-leak
