CC = gcc
CFLAGS = -g -fsanitize=address -fno-omit-frame-pointer -O1 -Wall -Werror -Wextra -Iinclude
CFLAGS_B = -g -O0 -Wall -Wextra -Iinclude
LDFLAGS = -lm -fsanitize=address

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRCS))

TARGET = $(BUILD_DIR)/app

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)