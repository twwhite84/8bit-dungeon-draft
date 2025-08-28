# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=c99 -fdiagnostics-color=always -g -I/usr/include/SDL2 -D_REENTRANT -I./src

# Linker flags
LDFLAGS = -lSDL2

# Source and output directories
SRC_DIR = src
OUT_DIR = out

# Output binary
OUT = $(OUT_DIR)/out

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)

.PHONY: all clean

all: $(OUT)

# Link object files to create the executable
$(OUT): $(OBJ)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile .c files into .o files inside the out directory
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OUT_DIR)/*.o $(OUT)
