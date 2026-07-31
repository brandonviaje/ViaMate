# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -O3

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source files
SRCS = $(SRC_DIR)/Attack.c \
       $(SRC_DIR)/Board.c \
       $(SRC_DIR)/Evaluate.c \
       $(SRC_DIR)/Magic.c \
       $(SRC_DIR)/Main.c \
       $(SRC_DIR)/MoveGen.c \
       $(SRC_DIR)/MovePicker.c \
       $(SRC_DIR)/PieceSquareTable.c \
       $(SRC_DIR)/Search.c \
       $(SRC_DIR)/Transposition.c \
       $(SRC_DIR)/Utils.c \
       $(SRC_DIR)/Zobrist.c \
       $(SRC_DIR)/uci.c \
       $(TEST_DIR)/Perft.c

# Object files placed under build
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Output executable
TARGET = viaje_engine

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) -lm

# Compile *.c → build/*.o
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Ensure build exists
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)
