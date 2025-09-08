CC = gcc
SRC = $(wildcard *.c)
OUT = cqlite

default: $(OUT)
	./$(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT)
