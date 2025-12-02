#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <SDL3/SDL.h>
#include "b8.h"

#define APP_NAME "b8"
#define FPS 60
#define MS_PER_FRAME (1000 / FPS)

void sdl_init(SDL_Window **window, SDL_Renderer **renderer, int w, int h);

int main(int argc, char **argv) {
    int screen_width = 800;
    int screen_height = 600;

    int opt;
    char *filename;

    while ((opt = getopt(argc, argv, "w:h:")) != -1) {
        switch (opt) {
            case 'w':
                screen_width = atoi(optarg);
                break;
            case 'h':
                screen_height = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        filename = argv[optind];
    }

    if (!filename) {
        fprintf(stderr, "Missing ROM file. Usage: %s <rom_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Chip8 c;
    b8_init(&c);
    b8_load(&c, filename);

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    sdl_init(&window, &renderer, screen_width, screen_height);

    bool is_running = true;
    SDL_Event event;
    uint64_t previous_tick = SDL_GetTicks();

    while (is_running) {
        uint64_t frame_start_tick = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    is_running = false;
                    break;
                default:
                    break;
            }
        }

        b8_emulate(&c);

        if (frame_start_tick - previous_tick >= MS_PER_FRAME) {
            if (c.delay_timer > 0) {
                c.delay_timer--;
            }

            if (c.sound_timer > 0) {
                c.sound_timer--;
            }

            previous_tick = frame_start_tick;
        }

        uint64_t frame_end_tick = SDL_GetTicks();
        uint64_t frame_time = frame_end_tick - frame_start_tick;
        if (frame_time < MS_PER_FRAME) {
            // capping framerate to 60fps (60Hz)
            SDL_Delay(MS_PER_FRAME - frame_time);
        }
    }
}

void sdl_init(SDL_Window **window, SDL_Renderer **renderer, int w, int h) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("sdl: could not init sdl: %s\n", SDL_GetError());
        SDL_Quit();
    }

    if (!SDL_SetAppMetadata(APP_NAME, "1.0.0", NULL)) {
        SDL_Log("sdl: could not set app metadata: %s\n", SDL_GetError());
        SDL_Quit();
    }

    if (!SDL_CreateWindowAndRenderer(APP_NAME, w, h, 0, window, renderer)) {
        SDL_Log("sdl: could not create window and renderer: %s\n", SDL_GetError());
        SDL_Quit();
    }

    if (!SDL_SetWindowPosition(*window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED)) {
        SDL_Log("sdl: could not set window position to centered: %s\n", SDL_GetError());
    }
}

void b8_init(Chip8 *c) {
    c->opcode = 0;
    c->pc = 0x200;
    c->I = 0;
    c->sp = 0;
    c->draw_flag = false;

    memset(c->gfx, 0, sizeof(c->gfx));
    memset(c->memory, 0, sizeof(c->memory));
    memset(c->V, 0, sizeof(c->V));
    memset(c->stack, 0, sizeof(c->stack));
    memset(c->key, 0, sizeof(c->key));

    // store fontset at 050–09F
    for (int i = 0; i < 80; i++) {
        c->memory[0x50 + i] = chip8_fontset[i];
    }

    c->delay_timer = 0;
    c->sound_timer = 0;
}

void b8_load(Chip8 *c, char *filename) {
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

void b8_emulate(Chip8 *c) {
    c->opcode = (c->memory[c->pc] << 8 | c->memory[c->pc + 1]);
    c->pc += 2;

    switch (c->opcode & 0xF000) {

    }
}
