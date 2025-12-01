#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <SDL3/SDL.h>
#include "b8.h"

int main(int argc, char **argv) {
    int opt;
    char *filename;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -f <rom_file>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!filename) {
        fprintf(stderr, "Missing flag. Usage: %s -f <rom_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Chip8 c;
    init(&c);
    load(&c, filename);

    while (1) {
        // fetch
        // decode
        // execute
        execute(&c);
    }

    return 0;
}

void init(Chip8 *c) {
    c->opcode = 0;
    c->pc = 0x200;
    c->I = 0;
    c->sp = 0;

    memset(c->gfx, 0, sizeof(c->gfx));
    memset(c->memory, 0, sizeof(c->memory));
    memset(c->V, 0, sizeof(c->V));
    memset(c->stack, 0, sizeof(c->stack));
    memset(c->key, 0, sizeof(c->key));

    // store fontset at 050–09F
    for (int i = 80; i <= 159; i++) {
        c->memory[i] = chip8_fontset[i];
    }

    c->delay_timer = 0;
    c->sound_timer = 0;
}

void load(Chip8 *c, char *filename) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        fprintf(stderr, "Failed to load ROM\n");
        exit(EXIT_FAILURE);
    }

    // load rom into memory location 0x200
    size_t bytes_read = fread(c->memory + 0x200, 1, 4096 - 0x200, file);

    if (bytes_read == 0 && ferror(file)) {
        fprintf(stderr, "Error reading ROM\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void execute(Chip8 *c) {
    // fetch, decode, execute

}
