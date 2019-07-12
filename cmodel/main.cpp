#include <iostream>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include "shared.h"


dreamcast_t state;

//// mem fns

template<typename T>
bool readMem(dreamcast_t *dc, uint32_t addy, T& data) {
    if ((addy >= 0x8C000000 && addy < 0x8D000000) || addy >= 0x0C000000 && addy < 0x0D000000) {
        data = *reinterpret_cast<T*>(&dc->sys_ram[addy & SYSRAM_MASK]);
        return true;
    } else {
        // return dc->video_ram[addy & VIDEORAM_MASK];

        printf("readMem(%08X)\n", addy);

        return false;
    }
}

template<typename T>
bool writeMem(dreamcast_t *dc, uint32_t addy, T data) {
    if ((addy >= 0x8C000000 && addy < 0x8D000000) || addy >= 0x0C000000 && addy < 0x0D000000) {
        *reinterpret_cast<T*>(&dc->sys_ram[addy & SYSRAM_MASK]) = data;
        return true;
    } else if (addy >= 0xA5000000 && addy < 0xA5800000) {
        *reinterpret_cast<T*>(&dc->video_ram[addy & VIDEORAM_MASK]) = data;
        return true;
    } else {
        // return dc->video_ram[addy & VIDEORAM_MASK];

        printf("writeMem %08X\n", addy);

        return false;
    }
}

void initDreamcast(dreamcast_t *dc) {

    memset(dc, 0, sizeof(*dc));

    BuildOpcodeTables(dc);

    memset(dc->pixels, 0, 640 * 480 * sizeof(uint32_t));

    dc->ctx.pc = 0x8C010000;

    FILE* f = fopen("roto.bin", "rb");

    assert(f != NULL && "Failed to load roto.bin");

    auto read = fread(&dc->sys_ram[0x10000], 256, 1, f);

    assert(read == 1);

    fclose(f);

#if DUMP
    for (auto i = 0; i < 256; i+=2) {
        auto offset = 0x10000 + i;

        uint16_t* ptr = reinterpret_cast<uint16_t*>(&dc->sys_ram[offset]);
        uint16_t opcode = *ptr;

        // printf("%08X: %04x %s\n", 0x8C000000 + offset, opcode, OpDesc[opcode]->diss);
        printf("%s\n", dc->OpDesc[opcode]->diss);
    }
#endif
}


void runDreamcast(dreamcast_t *dc) {

    u16 instr;

    readMem(dc, dc->ctx.pc, instr);

    // printf("%08X: %04x %s\n", dc->ctx.pc, instr, dc->OpDesc[instr]->diss);

    dc->ctx.pc += 2;

    dc->OpPtr[instr](dc, instr);
}

int main(int argc, char ** argv)
{
    bool leftMouseButtonDown = false;
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window * window = SDL_CreateWindow("SDL2 Pixel Drawing",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STATIC, 640, 480);
    

    dreamcast_t* dc = new dreamcast_t();

    initDreamcast(dc);

    while (!quit)
    {
        for (int i = 0; i <1000000; i++) {
            runDreamcast(dc);
        }

        memcpy(dc->pixels, dc->video_ram, std::min(VIDEORAM_SIZE,640 * 480 * 2));

        SDL_Delay(1);

        SDL_UpdateTexture(texture, NULL, dc->pixels, 640 * sizeof(u16));

        while(SDL_PollEvent(&event)) {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        leftMouseButtonDown = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        leftMouseButtonDown = true;
                case SDL_MOUSEMOTION:
                    if (leftMouseButtonDown)
                    {
                        int mouseX = event.motion.x;
                        int mouseY = event.motion.y;
                        dc->pixels[mouseY * 640 + mouseX] = 255;
                    }
                    break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    delete dc;

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
