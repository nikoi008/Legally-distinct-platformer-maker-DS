#include "globals.h"

gameStates state = MAIN_MENU;

block blocks[TOTAL_BLOCKS] = {};

u8 TILE_MAP[GRID_Y][GRID_X]; // 512 pixels, 32 tiles, every frame is 2x2 (16/8) tiles
