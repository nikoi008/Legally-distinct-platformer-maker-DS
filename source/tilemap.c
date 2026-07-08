#include <stdio.h>

#include <nf_lib.h>

#include "tilemap.h"
#include "globals.h"
#include "defines.h"

void updateTiles(gameContext *ctx)
{
    int oY = ctx->coords->originTileY;
    int oX = ctx->coords->originTileX;
    for(int y = oY; y < oY + MAP_Y; y++){
        for(int x = oX; x < oX + MAP_X; x++){
            int coordX = x - oX;
            int coordY = y - oY;
            int blockID = 0;
            if(x >= 0 && x < GRID_X && y >= 0 && y < GRID_Y)
                blockID = TILE_MAP[y][x];
            else
                blockID = 0;
            NF_SetTileOfMap(1, TILE_LAYER, coordX*2,   coordY*2,   blocks[blockID].topLeftTile);
            NF_SetTileOfMap(1, TILE_LAYER, coordX*2+1, coordY*2,   blocks[blockID].topRightTile);
            NF_SetTileOfMap(1, TILE_LAYER, coordX*2,   coordY*2+1, blocks[blockID].bottomLeftTile);
            NF_SetTileOfMap(1, TILE_LAYER, coordX*2+1, coordY*2+1, blocks[blockID].bottomRightTile);
        }
    }
}

