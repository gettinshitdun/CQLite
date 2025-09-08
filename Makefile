CC = gcc
SRC = $(wildcard *.c)
OUT = cqlite

default: $(OUT)
	./$(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT)

# Run clang-format on all source files
format:
	clang-format -i $(SRC)

# Clean build artifacts
clean:
	rm -f $(OUT)
