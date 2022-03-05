#include "backend.h"
#include "texture_data.h"

void am_audio_callback(void* userdata, uint8_t* stream, int len);
static Event be_get_keydown(Backend* be, SDL_Keycode key);
static void be_toggle_fullscreen(Backend* be);
static void be_toggle_scale(Backend* be);

const SDL_Color COLORS[16] = {
    {0xff, 0x00, 0xff, 0xff}, // TRANSPARENT
    {0x00, 0x00, 0x00, 0xff}, // BLACK
    {0x3e, 0xb8, 0x49, 0xff}, // MEDIUM_GREEN
    {0x74, 0xd0, 0x7d, 0xff}, // LIGHT_GREEN
    {0x59, 0x55, 0xe0, 0xff}, // DARK_BLUE
    {0x80, 0x76, 0xf1, 0xff}, // LIGHT_BLUE
    {0xb9, 0x5e, 0x51, 0xff}, // DARK_RED
    {0x65, 0xdb, 0xef, 0xff}, // CYAN
    {0xdb, 0x65, 0x59, 0xff}, // MEDIUM_RED
    {0xff, 0x89, 0x7d, 0xff}, // LIGHT_RED
    {0xcc, 0xc3, 0x5e, 0xff}, // DARK_YELLOW
    {0xde, 0xd0, 0x87, 0xff}, // LIGHT_YELLOW
    {0x3a, 0xa2, 0x41, 0xff}, // DARK_GREEN
    {0xb7, 0x66, 0xb5, 0xff}, // MAGENTA
    {0xcc, 0xcc, 0xcc, 0xff}, // GRAY
    {0xff, 0xff, 0xff, 0xff}, // WHITE
};

Backend* be_init(void) {
    SDL_AudioSpec spec_wanted;
    SDL_AudioSpec spec_received = { 0 };
    int err = 0;

    Backend* be = calloc(1, sizeof(Backend));
    LOG_ERR(be == NULL, "alloc failure");

    err = SDL_Init(SDL_INIT_VIDEO | 
                   SDL_INIT_AUDIO | 
                   SDL_INIT_EVENTS);
    LOG_ERR(err, SDL_GetError());

    be->win = SDL_CreateWindow(WINDOW_TITLE,
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               WINDOW_W, WINDOW_H, 0);
    LOG_ERR(be->win == NULL, SDL_GetError());

    be->ren = SDL_CreateRenderer(be->win, -1, 0);
    LOG_ERR(be->ren == NULL, SDL_GetError());

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom((void*) TEXTURE_DATA, 
                                                                   TEXTURE_W, 
                                                                   TEXTURE_H, 
                                                                   8, 
                                                                   TEXTURE_W, 
                                                                   SDL_PIXELFORMAT_INDEX8);
    LOG_ERR(surf == NULL, SDL_GetError());

    err = SDL_SetPaletteColors(surf->format->palette, COLORS, 0, 16);
    LOG_ERR(err, SDL_GetError());

    err = SDL_SetColorKey(surf, SDL_TRUE, 0);
    LOG_ERR(err, SDL_GetError());

    be->tex = SDL_CreateTextureFromSurface(be->ren, surf);
    LOG_ERR(be->tex == NULL, SDL_GetError());
    SDL_FreeSurface(surf);

    be->snd = sg_init();
    LOG_ERR(be->snd == NULL, "sg_init failed");

    spec_wanted = (SDL_AudioSpec) {
        .freq = SAMPLE_RATE,
        .format = AUDIO_S16SYS,
        .channels = 1,
        .silence = 0,
        .samples = BUF_LEN,
        .size = 0,
        .callback = am_audio_callback,
        .userdata = be->snd,
    };

    be->dev = SDL_OpenAudioDevice(NULL, 0, &spec_wanted, &spec_received, 0);
    LOG_ERR(be->dev == 0, SDL_GetError());

    SDL_PauseAudioDevice(be->dev, 0);
    be_toggle_scale(be);
    return be;
}

void am_audio_callback(void* userdata, uint8_t* stream, int len) {
    SoundGen* snd = (SoundGen*) userdata;
    sg_generate_i16(snd, stream, len);
}

void be_quit(Backend* be) {
    SDL_DestroyTexture(be->tex);
    SDL_DestroyRenderer(be->ren);
    SDL_DestroyWindow(be->win);
    SDL_CloseAudioDevice(be->dev);
    SDL_free(be);
    SDL_Quit();
}

Event be_get_event(Backend* be) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return QUIT;
        } 

        if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
            return be_get_keydown(be, e.key.keysym.sym);
        } 
    }

    return IDLE;
}

static Event be_get_keydown(Backend* be, SDL_Keycode key) {
    switch (key) {
        case SDLK_F10:
            return QUIT;
        case SDLK_ESCAPE:
            return KD_ESC;
        case SDLK_UP:
            return KD_UP;
        case SDLK_DOWN:
            return KD_DOWN;
        case SDLK_LEFT:
            return KD_LEFT;
        case SDLK_RIGHT:
            return KD_RIGHT;
        case SDLK_SPACE:
            return KD_SPC;
        case SDLK_F1:
            return KD_F1;
        case SDLK_F2:
            return KD_F2;
        case SDLK_F3:
            be_toggle_scale(be);
            return IDLE;
        case SDLK_F4:
            be_toggle_fullscreen(be);
            return IDLE;
        case SDLK_F5:
            return KD_F5;
        case SDLK_F6:
            return KD_F6;
        default:
            return IDLE;
    }
}

void be_set_color(Backend* be, int color) {
    SDL_Color c = COLORS[color]; 
    SDL_SetRenderDrawColor(be->ren, c.r, c.g, c.b, c.a);
}

void be_clear(Backend* be) {
    SDL_RenderClear(be->ren);
}

void be_present(Backend* be) {
    SDL_RenderPresent(be->ren);
}

void be_blit_tile(Backend* be, int x, int y, int n) {
    int src_x = n % TILES_PER_ROW * TILE_W;
    int src_y = n / TILES_PER_ROW * TILE_H;
    SDL_Rect src = {src_x, src_y, TILE_W, TILE_H};
    SDL_Rect dst = {x, y, TILE_W, TILE_H};
    SDL_RenderCopy(be->ren, be->tex, &src, &dst);
}

void be_blit_text(Backend* be, int x, int y, char* str) {
    while (*str) {
        int n = FONT_OFFSET + (int) *str;
        int src_x = n % CHARS_PER_ROW * FONT_W;
        int src_y = n / CHARS_PER_ROW * FONT_H;
        SDL_Rect src = {src_x, src_y, FONT_W, FONT_H};
        SDL_Rect dst = {x, y, FONT_W, FONT_H};
        SDL_RenderCopy(be->ren, be->tex, &src, &dst);
        x += FONT_W;
        str++;
    }
}

void be_send_audiomsg(Backend* be, int msg) {
    SDL_LockAudioDevice(be->dev);
    sg_handle_message(be->snd, msg);
    SDL_UnlockAudioDevice(be->dev);
}

void be_draw_line(Backend* be, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(be->ren, x1, y1, x2, y2);
}

static void be_toggle_fullscreen(Backend* be) {
    if (SDL_GetWindowFlags(be->win) & SDL_WINDOW_FULLSCREEN) {
        SDL_SetWindowDisplayMode(be->win, NULL);
        SDL_SetWindowFullscreen(be->win, 0);
    } else {
        SDL_SetWindowFullscreen(be->win, SDL_WINDOW_FULLSCREEN);
    }
}

static void be_toggle_scale(Backend* be) {
    float sx, sy;
    SDL_RenderGetScale(be->ren, &sx, &sy);

    if (sx == 1.0 && sy == 1.0) {
        sx = 2.0;
        sy = 2.0;
    } else if (sx == 2.0 && sy == 2.0) { 
        sx = 3.0;
        sy = 3.0;
    } else if (sx == 3.0 && sy == 3.0) { 
        sx = 4.0;
        sy = 4.0;
    } else if (sx == 4.0 && sy == 4.0) { 
        sx = 5.0;
        sy = 5.0;
    } else { 
        sx = 1.0;
        sy = 1.0;
    }

    SDL_SetWindowSize(be->win, ((int)sx) * WINDOW_W, ((int)sy) * WINDOW_H);
    SDL_SetWindowPosition(be->win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_RenderSetScale(be->ren, sx, sy);

    if (SDL_GetWindowFlags(be->win) & SDL_WINDOW_FULLSCREEN) {
        SDL_SetWindowDisplayMode(be->win, NULL);
    }
}

uint64_t be_get_millis(void) {
    return SDL_GetTicks();
}

void be_delay(uint32_t dur) {
    SDL_Delay(dur);
}
