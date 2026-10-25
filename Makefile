MODE ?= debug

CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11
CPPFLAGS = -I"./include"
LD_FLAGS = -lm

ifeq ($(MODE),release)
	CFLAGS += -O3 -DNDEBUG
else ifeq ($(MODE),debug)
	CFLAGS += -DDEBUG -g -O0 -fsanitize=address
	LD_FLAGS += -g -fsanitize=address
else
	$(error Unknown MODE="$(MODE)". Only debug/release supported)
endif

OUT_DIR = out/$(MODE)
OUT_DIR_MODULES = out/$(MODE)/modules

HEADERS := $(wildcard include/*.h)

SRC_MODULES := $(wildcard src/*/)
SRC_EXECS := $(wildcard src/*.c)
ALL_EXEC_NAMES := $(patsubst src/%.c,%,$(SRC_EXECS))

CPPFLAGS += $(patsubst src/%/,-I"./src/%",$(SRC_MODULES))


.PHONY: all clean test $(ALL_EXEC_NAMES)

all: $(ALL_EXEC_NAMES)

clean:
	rm -rf out

test:
	@echo "No tests yet"

include macros.mk

$(eval $(call COMPILE_EXECUTABLE,daman,util lang compiler))

$(eval $(call COMPILE_MODULE,util,))
$(eval $(call COMPILE_MODULE,lang,util))
$(eval $(call COMPILE_MODULE,compiler,util lang))
