LIB = libskjson.so
SRCDIR = src
BUILD_DIR = build
TESTDIR = tests
RELEASE_DIR := $(BUILD_DIR)/release
OBJDIR := $(RELEASE_DIR)/obj
DBUG_DIR := $(BUILD_DIR)/debug
DBUG_OBJDIR := $(DBUG_DIR)/obj

HEADER_INSTALL_DIR = /usr/local/include
LIB_INSTALL_DIR = /usr/local/lib

CFILES := $(wildcard $(SRCDIR)/*.c)
OBJFILES := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CFILES))
DEP_FILES := $(patsubst %.o,%.d,$(OBJFILES))
DBUG_OBJFILES := $(patsubst $(SRCDIR)/%.c,$(DBUG_OBJDIR)/%.o,$(CFILES))
DBUG_DEP_FILES := $(patsubst %.o,%.d,$(DBUG_OBJFILES))

CC = gcc
INCLUDES = -Isrc
DEPFLAGS = -MD -MP
DBUG = -DSK_DBUG -DSK_ERRMSG
OPT_BUILD = -Os
CFLAGS := -Wall -Werror -Wextra -Wpedantic -ansi $(OPT_BUILD) $(INCLUDES) $(DEPFLAGS)
DBUG_CFLAGS := -Wall -Werror -Wextra -Wpedantic -ansi $(INCLUDES) $(DEPFLAGS)

all: $(DBUG_DIR)/$(LIB) $(RELEASE_DIR)/$(LIB)

$(DBUG_DIR)/$(LIB): $(DBUG_OBJFILES)
	$(CC) -shared -o $@ $^

$(DBUG_OBJFILES): | $(DBUG_OBJDIR)

$(DBUG_OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(DBUG) $(DBUG_CFLAGS) -g -c -fPIC $< -o $@

$(DBUG_OBJDIR): $(DBUG_DIR)
	mkdir -p $@
 
$(DBUG_DIR): $(BUILD_DIR)
	mkdir -p $@

$(RELEASE_DIR)/$(LIB): $(OBJFILES)
	$(CC) -shared -o $@ $^

$(OBJFILES): | $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -fPIC $< -o $@

$(OBJDIR): $(RELEASE_DIR)
	mkdir -p $@

$(RELEASE_DIR): $(BUILD_DIR)
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@

debug: $(DBUG_DIR)/$(LIB)

release: $(RELEASE_DIR)/$(LIB)

clean:
	rm -rf $(DBUG_DIR)/$(LIB) $(RELEASE_DIR)/$(LIB) $(OBJFILES) $(DBUG_OBJFILES) $(DBUG_DEP_FILES) $(DEP_FILES)
	$(MAKE) -C $(TESTDIR) clean

test: debug
	$(MAKE) -C $(TESTDIR) test

test-leak: debug
	$(MAKE) -C $(TESTDIR) test-leak

install: $(RELEASE_DIR)/$(LIB)
	sudo cp -i $(RELEASE_DIR)/$(LIB) $(LIB_INSTALL_DIR)

-include $(DEP_FILES) $(DBUG_DEP_FILES)

.PHONY: all clean test install dirs release debug test-leak
