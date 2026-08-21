CC = gcc
CFLAGS = -g -fsanitize=address -fno-omit-frame-pointer -O2 -Wall -Werror -Wextra -Iinclude -Iext 
LDFLAGS = -lm -fsanitize=address

SRC_DIR = src
EXT_DIR = external
DEMO_DIR = demo
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
EXT_SRCS = $(wildcard $(EXT_DIR)/*.c)
DEMO_SRCS = $(wildcard $(DEMO_DIR)/*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRCS))
EXT_OBJS = $(patsubst $(EXT_DIR)/%.c,$(BIN_DIR)/%.o,$(EXT_SRCS))
DEMO_OBJS = $(patsubst $(DEMO_DIR)/%.c,$(BIN_DIR)/%.o,$(DEMO_SRCS))

TARGET = $(BUILD_DIR)/demo

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) $(EXT_OBJS) $(DEMO_OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) $(EXT_OBJS) $(DEMO_OBJS) -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(EXT_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(DEMO_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)