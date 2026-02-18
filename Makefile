# =========================================================
# Makefile – EasyCharge project
# Usage:
#   make          build the executable (output: build/easycharge)
#   make clean    remove all object files and the executable
# =========================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c99 -Isrc
LDFLAGS = -lm

SRC_DIR   = src
BUILD_DIR = build
TARGET    = $(BUILD_DIR)/easycharge

SRCS = $(SRC_DIR)/main.c       \
       $(SRC_DIR)/queue.c      \
       $(SRC_DIR)/port_list.c  \
       $(SRC_DIR)/car_bst.c    \
       $(SRC_DIR)/station_bst.c\
       $(SRC_DIR)/file_io.c    \
       $(SRC_DIR)/menu_ops.c

OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .c from src/ to .o in build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
