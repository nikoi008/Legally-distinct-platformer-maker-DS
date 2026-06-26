#ifndef DEFINES_H
#define DEFINES_H

// ON DS  ONE TILE IS 8X8!!!!!!
//bg size is 512 hence 512/8 = 64
#define SPEED 4        // Scroll speed
#define MAP_X 32       // Map width (animated blocks are 2x2, 128 / 2 = 64)
#define MAP_Y 32        // Map height (animated blocks are 2x2, 64 / 2 = 32)
#define GRID_X 1024
#define GRID_Y 1024 //16kx16k may be a bit overkill
#define ROW_LENGTH 16
#define TOTAL_BLOCKS 4
#define TILE_LAYER 3
#define BLACK 0
#define JUMP_FORCE 16
#define GRAVITY 2
#define TRANSPARENT_BLOCK_OFFSET 64 // ie the green blocks
#define HUD_Y_START 22

#endif
