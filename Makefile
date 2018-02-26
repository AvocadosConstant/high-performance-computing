CC          = g++
LD          = g++
CFLAGS      = -g -std=c++17 -Wall -Wextra -pedantic -Iinclude
LDFLAGS		= -Wl,--no-as-needed -ldl

PROG_NAME   = k-nn

SRC_DIR     = src
BUILD_DIR   = build
BIN_DIR     = .

SRC_LIST 	= $(wildcard $(SRC_DIR)/*.cpp)
OBJ_LIST 	= $(BUILD_DIR)/$(notdir $(SRC_LIST:.cpp=.o))


.PHONY: all clean $(PROG_NAME) compile

all: $(PROG_NAME)

compile:
	@echo "\nCompiling..."
	$(CC) -c $(CFLAGS) $(SRC_LIST) -o $(OBJ_LIST)
	@echo ""

$(PROG_NAME): compile
	@echo "Linking..."
	$(LD) $(LDFLAGS) $(OBJ_LIST) -o $(BIN_DIR)/$@
	@echo ""

clean:
	@echo "\nCleaning..."
	rm -f $(BIN_DIR)/$(PROG_NAME) $(BUILD_DIR)/*.o
	@echo ""
