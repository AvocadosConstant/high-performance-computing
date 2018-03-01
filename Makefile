CC          = g++
LD          = g++
CFLAGS      = -g -std=c++17 -Wall -Wextra -pedantic -Iinclude
LDFLAGS		= -Wl,--no-as-needed -ldl -pthread

PROG_NAME   = k-nn

SRC_DIR     = src
BUILD_DIR   = build
BIN_DIR     = .


all: $(PROG_NAME)

$(PROG_NAME): $(BUILD_DIR)/main.o
	@echo "\nLinking..."
	$(CC) $(BUILD_DIR)/main.o $(BUILD_DIR)/tree.o -o $(PROG_NAME) $(LDFLAGS)
	@echo ""

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(BUILD_DIR)/tree.o
	@echo "\nCompiling main..."
	$(CC) -c $< -o $@ $(CFLAGS)
	@echo ""

$(BUILD_DIR)/tree.o: $(SRC_DIR)/tree.cpp
	@echo "\nCompiling tree..."
	$(CC) -c $< -o $@ $(CFLAGS)
	@echo ""

clean:
	@echo "\nCleaning..."
	rm -f $(BIN_DIR)/$(PROG_NAME) $(BUILD_DIR)/*.o
	@echo ""
