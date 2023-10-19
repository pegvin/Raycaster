CC:=gcc
CFLAGS:=-std=c99 -Wall -MMD -MP -Ifenster/
LFLAGS:=

BIN        := raycaster
BUILD      := build
BUILD_TYPE := Debug
SOURCES    := src/main.c src/impl.c src/gfx.c
OBJECTS    := $(SOURCES:.c=.c.o)
OBJECTS    := $(patsubst %,$(BUILD)/%,$(OBJECTS))
DEPENDS    := $(OBJECTS:.o=.d)

ifeq ($(BUILD_TYPE),Debug)
	CFLAGS+=-O0 -g
else
	ifeq ($(BUILD_TYPE),Release)
		CFLAGS+=-O3
	else
$(error Unknown Build Type "$(BUILD_TYPE)")
	endif
endif

ifeq ($(OS),Windows_NT)
	LFLAGS+=-lgdi32 -lwinmm
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LFLAGS+=-lX11
    endif
    ifeq ($(UNAME_S),Darwin)
        LFLAGS+=-framework Cocoa
    endif
endif

-include $(DEPENDS)

all: $(BIN)

$(BUILD)/%.c.o: %.c
	@echo "CC  -" $<
	@mkdir -p "$$(dirname "$@")"
	@bear --output $(BUILD)/compile_commands.json -- $(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJECTS)
	@echo Linking $@
	@$(CXX) $(OBJECTS) $(LFLAGS) -o $@

.PHONY: run
.PHONY: clean

run: $(BIN)
	@./$(BIN)

clean:
	$(RM) -rv $(BIN) $(BUILD)

