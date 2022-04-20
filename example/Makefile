include ../lib/make-pal/pal.mak
DIR_BUILD:=build
CC:=gcc

MAIN_NAME:=example
MAIN_SRC:=$(wildcard src/*.c)
MAIN_OBJ:=$(MAIN_SRC:src/%.c=$(DIR_BUILD)/%.o)
MAIN_DEP:=$(MAIN_OBJ:%.o=%.d)
MAIN_CC_FLAGS:=-g -Werror -Wno-unused-parameter -W -Wall -Wextra -Wconversion -Wshadow -DDEBUG -I../include
MAIN_LD_FLAGS:=

ifeq ($(OS),Windows_NT)
	MAIN_CC_FLAGS+=-lwinscard
else
	UNAME:=$(shell uname -s)
	MAIN_CC_FLAGS+=-lpcsclite
	ifeq ($(UNAME),Linux)
        MAIN_CC_FLAGS+=-pthread -I/usr/include/PCSC
    endif
	ifeq ($(UNAME),Darwin)
        MAIN_CC_FLAGS+=-framework PCSC
	endif
endif

all: main

# Create example for using the library.
main: $(DIR_BUILD) $(DIR_BUILD)/$(MAIN_NAME).$(EXT_BIN)
$(DIR_BUILD)/$(MAIN_NAME).$(EXT_BIN): $(MAIN_OBJ) ../build/$(LIB_PREFIX)scraw.$(EXT_LIB_STATIC)
	$(CC) $(^) -o $(@) $(MAIN_CC_FLAGS) $(MAIN_LD_FLAGS)

# Compile source files to object files.
$(DIR_BUILD)/%.o: src/%.c
	$(CC) $(<) -o $(@) $(MAIN_CC_FLAGS) -c -MMD

# Recompile source files after a header they include changes.
-include $(MAIN_DEP)

$(DIR_BUILD):
	$(call pal_mkdir,$(@))
clean:
	$(call pal_rmdir,$(DIR_BUILD))

.PHONY: all main clean
