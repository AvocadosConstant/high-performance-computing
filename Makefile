CC          = g++
LD          = g++
CFLAGS      = -g -std=c++17 -Wall -Wextra -pedantic -Iinclude
LDFLAGS		= -Wl,--no-as-needed -ldl

PROG_NAME   = k-nn

SRC_DIR     = src
BUILD_DIR   = build
BIN_DIR     = .


all: $(PROG_NAME)

$(PROG_NAME): $(BUILD_DIR)/main.o
	$(CC) main.o tree.o -o $(PROG_NAME)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(BUILD_DIR)/tree.o
	@echo "Compiling main..."
	$(CC) -c $(CFLAGS) $(SRC_DIR)/main.cpp

$(BUILD_DIR)/tree.o: $(SRC_DIR)/tree.cpp
	@echo "Compiling tree..."
	$(CC) -c $(CFLAGS) $(SRC_DIR)/tree.cpp

clean:
	@echo "\nCleaning..."
	rm -f $(BIN_DIR)/$(PROG_NAME) $(BUILD_DIR)/*.o
	@echo ""
