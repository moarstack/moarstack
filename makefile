.PHONY: all clean

EXE=server
BIN=bin
OBJ=obj

all: dirs $(OBJ)/many_clients.o $(OBJ)/hash.o
	gcc $(OBJ)/many_clients.o $(OBJ)/hash.o -lm -o $(BIN)/$(EXE)

$(OBJ)/many_clients.o: many_clients.c hash.h
	gcc -c -std=c99 many_clients.c -lm -o $(OBJ)/many_clients.o

$(OBJ)/hash.o:
	gcc -c -std=c99 hash.c -o $(OBJ)/hash.o

dirs:
	mkdir -p $(OBJ) $(BIN)

clean:
	rm $(BIN)/$(EXE)
	rm $(OBJ)/*.o
