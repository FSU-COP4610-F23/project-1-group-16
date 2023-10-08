SRC := src
OBJ := obj
BIN := bin
EXECUTABLE:= shell
EXECUTABLE2:= mytimeout

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
INCS := -Iinclude/
DIRS := $(OBJ)/ $(BIN)/
EXEC := $(BIN)/$(EXECUTABLE)

CC := gcc
CFLAGS := -g -Wall -std=c99 $(INCS)
LDFLAGS :=

all: $(EXEC) $(BIN)/$(EXECUTABLE2) #if this does not exisist, go below

$(BIN)/$(EXECUTABLE2): 
	$(CC) $(CFLAGS) -o $(BIN)/$(EXECUTABLE2) $(SRC)/mytimeout/mytimeout.c

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXEC)
	$(EXEC)

clean:
	rm $(OBJ)/*.o $(EXEC) $(BIN)/$(EXECUTABLE2)

$(shell mkdir -p $(DIRS))

.PHONY: run clean all
