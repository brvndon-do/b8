CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC = b8.c
OUT = b8

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) `sdl3-config --cflags --libs`

clean:
	rm -f $(OUT)
