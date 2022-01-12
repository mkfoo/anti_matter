#include "gamestate.h"

bool sc_splash(GameState* gs, Backend* be);
bool sc_title_anim(GameState* gs, Backend* be);
bool sc_title_move(GameState* gs, Backend* be);
bool sc_title(GameState* gs, Backend* be);
bool sc_start_level(GameState* gs, Backend* be);
bool sc_playing(GameState* gs, Backend* be);
bool sc_paused(GameState* gs, Backend* be);
bool sc_wait(GameState* gs, Backend* be);
bool sc_swap(GameState* gs, Backend* be); 
bool sc_level_clear(GameState* gs, Backend* be);
bool sc_clear_wait(GameState* gs, Backend* be);
bool sc_death1(GameState* gs, Backend* be);
bool sc_death2(GameState* gs, Backend* be);
bool sc_game_over(GameState* gs, Backend* be);
