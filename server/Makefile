SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .

EXE := $(BIN_DIR)/srvFtp
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CXX = gcc
CXXFLAGS := -Wall

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJS) | $(BIN_DIR)
	$(CXX) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rvf $(OBJS) $(EXE) $(OBJ_DIR)

-include $(OBJ:.o=.d)