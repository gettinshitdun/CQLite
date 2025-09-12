CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -Isrc/include
SRC = $(wildcard src/*.c)
OUT = cqlite

default: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

# Run clang-format on all C source & header files
format:
	clang-format -i src/*.c src/include/*.h

# Clean build artifacts
clean:
	rm -f $(OUT)
