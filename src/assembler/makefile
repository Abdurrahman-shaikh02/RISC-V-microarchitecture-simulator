# Compiler
CC = gcc

# Flags
CFLAGS = -Wextra -Iinclude #-Wall

# Directories
CORE_DIR = core
LOG_DIR = log
BUILD_DIR = build

# Source files
SRC = $(CORE_DIR)/main.c \
      $(CORE_DIR)/opcode_generator.c \
      $(CORE_DIR)/parser.c \
      $(CORE_DIR)/semantics_analyzer.c \
      $(CORE_DIR)/symbol_table.c \
      $(CORE_DIR)/syntax_analyzer.c \
      $(LOG_DIR)/logger.c

# Object files (redirected to build/)
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)

# Output executable
TARGET = assemble

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile rule (handles subdirectories too)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Rebuild
rebuild: clean all
