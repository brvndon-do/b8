#include <getopt.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "b8.h"

#define APP_NAME "b8"
#define FPS 60
#define MS_PER_FRAME (1000 / FPS)
#define INSTRUCTIONS_PER_FRAME 5

void sdl_init(SDL_Window **window, SDL_Renderer **renderer, int w, int h);
void sdl_render(Chip8 *c, SDL_Renderer *renderer);
int sdl_map_key(SDL_Keycode key);
void sdl_beep();
void sdl_cleanup(SDL_Window *window, SDL_Renderer *renderer);

int main(int argc, char **argv) {
    srand(time(NULL));

    int screen_width = CHIP8_WIDTH * CHIP8_PIXEL_SCALE;
    int screen_height = CHIP8_HEIGHT * CHIP8_PIXEL_SCALE;

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
                case SDL_EVENT_KEY_DOWN: {
                    int key = sdl_map_key(event.key.scancode);
                    if (key != -1) {
                        c.key[key] = 1;
                    }
                    break;
                }
                case SDL_EVENT_KEY_UP: {
                    int key = sdl_map_key(event.key.scancode);
                    if (key != -1) {
                        c.key[key] = 0;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
            b8_emulate(&c);
        }

        if (c.draw_flag) {
            sdl_render(&c, renderer);
        }

        if (frame_start_tick - previous_tick >= MS_PER_FRAME) {
            if (c.delay_timer > 0) {
                c.delay_timer--;
            }

            // TODO: need to properly implement sound
            if (c.sound_timer > 0) {
                sdl_beep();
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

    sdl_cleanup(window, renderer);
    SDL_Quit();
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

void sdl_render(Chip8 *c, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < CHIP8_HEIGHT; y++) {
        for (int x = 0; x < CHIP8_WIDTH; x++) {
            if (c->gfx[y * CHIP8_WIDTH + x]) {
                SDL_FRect pixel = {
                    .x = x * CHIP8_PIXEL_SCALE,
                    .y = y * CHIP8_PIXEL_SCALE,
                    .w = CHIP8_PIXEL_SCALE,
                    .h = CHIP8_PIXEL_SCALE
                };

                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }

    if (!SDL_RenderPresent(renderer)) {
        SDL_Log("sdl: render failure: %s\n", SDL_GetError());
    }
}

int sdl_map_key(SDL_Keycode key) {
    switch (key) {
        case SDL_SCANCODE_1:
            return 0x1;
        case SDL_SCANCODE_2:
            return 0x2;
        case SDL_SCANCODE_3:
            return 0x3;
        case SDL_SCANCODE_4:
            return 0xC;
        case SDL_SCANCODE_Q:
            return 0x4;
        case SDL_SCANCODE_W:
            return 0x5;
        case SDL_SCANCODE_E:
            return 0x6;
        case SDL_SCANCODE_R:
            return 0xD;
        case SDL_SCANCODE_A:
            return 0x7;
        case SDL_SCANCODE_S:
            return 0x8;
        case SDL_SCANCODE_D:
            return 0x9;
        case SDL_SCANCODE_F:
            return 0xE;
        case SDL_SCANCODE_Z:
            return 0xA;
        case SDL_SCANCODE_X:
            return 0x0;
        case SDL_SCANCODE_C:
            return 0xB;
        case SDL_SCANCODE_V:
            return 0xF;
        default:
            return -1;
    }
}

void sdl_beep() {
    // TODO: implement beeping audio
}

void sdl_cleanup(SDL_Window *window, SDL_Renderer *renderer) {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
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
    c->draw_flag = false;

    uint16_t opcode = (c->memory[c->pc] << 8 | c->memory[c->pc + 1]);
    uint8_t op = (opcode & 0xF000) >> 12;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = (opcode & 0x000F);
    uint8_t kk = (opcode & 0x00FF);
    uint16_t nnn = (opcode & 0x0FFF);

    c->opcode = opcode;
    c->pc += 2;

    switch (op) {
        case 0x0:
            switch (kk) {
                case 0xE0:
                    memset(c->gfx, 0, sizeof(c->gfx));
                    c->draw_flag = true;
                    break;
                case 0xEE:
                    c->sp--;
                    c->pc = c->stack[c->sp];
                    break;
            }
            break;
        case 0x1:
            c->pc = nnn;
            break;
        case 0x2:
            c->stack[c->sp] = c->pc;
            c->sp++;
            c->pc = nnn;
            break;
        case 0x3:
            if (c->V[x] == kk) {
                c->pc += 2;
            }
            break;
        case 0x4:
            if (c->V[x] != kk) {
                c->pc += 2;
            }
            break;
        case 0x5:
            if (c->V[x] == c->V[y]) {
                c->pc += 2;
            }
            break;
        case 0x6:
            c->V[x] = kk;
            break;
        case 0x7:
            c->V[x] += kk;
            break;
        case 0x8:
            switch (n) {
                case 0x0:
                    c->V[x] = c->V[y];
                    break;
                case 0x1:
                    c->V[x] |= c->V[y];
                    break;
                case 0x2:
                    c->V[x] &= c->V[y];
                    break;
                case 0x3:
                    c->V[x] ^= c->V[y];
                    break;
                case 0x4:
                    uint16_t sum = c->V[x] + c->V[y];
                    c->V[0xF] = sum > 0xFF ? 1 : 0;
                    c->V[x] = sum & 0xFF;
                    break;
                case 0x5:
                    c->V[0xF] = c->V[x] > c->V[y] ? 1 : 0;
                    c->V[x] -= c->V[y];
                    break;
                case 0x6:
                    c->V[0xF] = c->V[x] & 0x1;
                    c->V[x] >>= 1;
                    break;
                case 0x7:
                    c->V[0xF] = c->V[y] > c->V[x] ? 1 : 0;
                    c->V[x] = c->V[y] - c->V[x];
                    break;
                case 0xE:
                    c->V[0xF] = (c->V[x] & 0x80) >> 7;
                    c->V[x] <<= 1;
                    break;
            }
            break;
        case 0x9:
            if (c->V[x] != c->V[y]) {
                c->pc += 2;
            }
            break;
        case 0xA:
            c->I = nnn;
            break;
        case 0xB:
            c->pc = nnn + c->V[0x0];
            break;
        case 0xC:
            c->V[x] = (rand() & 0xFF) & kk;
            break;
        case 0xD:
            uint8_t vx = c->V[x];
            uint8_t vy = c->V[y];
            uint8_t height = n;
            c->V[0xF] = 0;

            for (int y_line = 0; y_line < height; y_line++) {
                uint8_t sprite_byte = c->memory[c->I + y_line];
                for (int x_line = 0; x_line < 8; x_line++) {
                    uint8_t sprite_pixel = (sprite_byte >> (7 - x_line)) & 1;
                    int px = (vx + x_line) % CHIP8_WIDTH;
                    int py = (vy + y_line) % CHIP8_HEIGHT;
                    int idx = py * CHIP8_WIDTH + px;

                    if (sprite_pixel) {
                        if (c->gfx[idx]) {
                            c->V[0xF] = 1;
                        }

                        c->gfx[idx] ^= 1;
                    }
                }
            }

            c->draw_flag = true;
            break;
        case 0xE:
            switch (kk) {
                case 0x9E:
                    if (c->key[c->V[x]]) {
                        c->pc += 2;
                    }
                    break;
                case 0xA1:
                    if (!c->key[c->V[x]]) {
                        c->pc += 2;
                    }
                    break;
            }
            break;
        case 0xF:
            switch (y) {
                case 0x0:
                    switch (n) {
                        case 0x7:
                            c->V[x] = c->delay_timer;
                            break;
                        case 0xA:
                            bool key_pressed = false;
                            for (int i = 0; i < 16; i++) {
                                if (c->key[i]) {
                                    c->V[x] = i;
                                    key_pressed = true;
                                    break;
                                }
                            }

                            if (!key_pressed) {
                                c->pc -= 2;
                            }
                            break;
                    }
                    break;
                case 0x1:
                    switch (n) {
                        case 0x5:
                            c->delay_timer = c->V[x];
                            break;
                        case 0x8:
                            c->sound_timer = c->V[x];
                            break;
                        case 0xE:
                            c->I += c->V[x];
                            break;
                    }
                    break;
                case 0x2:
                    c->I = 0x50 + (c->V[x] * 5);
                    break;
                case 0x3:
                    c->memory[c->I] = c->V[x] / 100;
                    c->memory[c->I + 1] = (c->V[x] / 10) % 10;
                    c->memory[c->I + 2] = c->V[x] % 10;
                    break;
                case 0x5:
                    for (int i = 0; i <= x; i++) {
                        c->memory[c->I + i] = c->V[i];
                    }
                    break;
                case 0x6:
                    for (int i = 0; i <= x; i++) {
                        c->V[i] = c->memory[c->I + i];
                    }
                    break;
            }
            break;
    }
}
