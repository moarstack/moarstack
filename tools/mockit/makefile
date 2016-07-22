.PHONY: all clean

EXE=mockit
BIN=bin
OBJ=obj
SRC=src
INC=inc

all: dirs $(OBJ)/many_clients.o $(OBJ)/hash.o
	gcc $(OBJ)/many_clients.o $(OBJ)/hash.o -lm -o $(BIN)/$(EXE)

$(OBJ)/many_clients.o: $(SRC)/many_clients.c $(INC)/hash.h
	gcc -c -std=c99 -I$(INC) $(SRC)/many_clients.c -lm -o $(OBJ)/many_clients.o

$(OBJ)/hash.o:
	gcc -c -std=c99 -I$(INC) $(SRC)/hash.c -o $(OBJ)/hash.o

dirs:
	mkdir -p $(OBJ) $(BIN)

clean:
	rm $(BIN)/$(EXE)
	rm $(OBJ)/*.o 