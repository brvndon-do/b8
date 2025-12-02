CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I/usr/include/SDL3
LDFLAGS = -lSDL3
SRC = b8.c
OUT = b8

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -f $(OUT)
