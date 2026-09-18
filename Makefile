CC := gcc
CSTD := -std=c17
WARN := -Wall -Wextra -Wpedantic
INC := -Iinclude

SRC_DIR := src
BUILD_DIR := build
BIN := cplay

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default build: debug-friendly but with warnings-as-errors off for now
CFLAGS := $(CSTD) $(WARN) $(INC) -g

.PHONY: all debug release clean test install

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

debug: CFLAGS += -fsanitize=address,undefined -O0
debug: clean $(BIN)

release: CFLAGS := $(CSTD) $(WARN) $(INC) -O2
release: clean $(BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN)

test:
	@echo "No tests yet — coming once we have real logic to test (Phase 2+)."

install: release
	install -Dm755 $(BIN) /usr/local/bin/$(BIN)
