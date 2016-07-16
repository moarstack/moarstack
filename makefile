.PHONY: all clean

EXE=server
BIN=bin
OBJ=obj

all: $(OBJ)/many_clients.o $(OBJ)/hash.o
	gcc -lm $(OBJ)/many_clients.o $(OBJ)/hash.o -o $(BIN)/$(EXE)

$(OBJ)/many_clients.o: many_clients.c hash.h
	gcc -c -std=c99 -lm many_clients.c -o $(OBJ)/many_clients.o

$(OBJ)/hash.o:
	gcc -c -std=c99 hash.c -o $(OBJ)/hash.o

clean:
	rm $(BIN)/$(EXE)
	rm $(OBJ)/*.o
