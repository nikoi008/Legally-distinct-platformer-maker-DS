#include <stdio.h>

#include <nf_lib.h>

#include "tilemap.h"
#include "globals.h"
#include "defines.h"

void updateTiles(worldCoordinates *coords)
{
    int oY = coords->originTileY;
    int oX = coords->originTileX;
    for(int y = oY; y < oY + MAP_Y; y++){
        for(int x = oX; x < oX + MAP_X; x++){
            int coordX = x - oX;
            int coordY = y - oY;
            int blockID = 0;
            if(x >= 0 && x < GRID_X && y >= 0 && y < GRID_Y)
                blockID = TILE_MAP[y][x];
            else
                blockID = 4;
            NF_SetTileOfMap(1, TILE_LAYER, coordX*2,   coordY*2,   blocks[blockID].topLeftTile);
            NF_SetTileOfMap(1, TILE_LAYER, coordX*2+1, coordY*2,   blocks[blockID].topRightTile);
            NF_SetTileOfMap(1, TILE_LAYER, coordX*2,   coordY*2+1, blocks[blockID].bottomLeftTile);
            NF_SetTileOfMap(1, TILE_LAYER, coordX*2+1, coordY*2+1, blocks[blockID].bottomRightTile);
        }
    }
}

void saveLevel(char* name){
    FILE *ptr = fopen(name, "wb");
    if (!ptr) return;
    fwrite(TILE_MAP, sizeof(u8), GRID_X * GRID_Y, ptr);
    fclose(ptr);
}

void loadLevel(char* name,worldCoordinates *coords){
    FILE *ptr = fopen(name, "rb");
    if (!ptr) return;
    fread(TILE_MAP, sizeof(u8), GRID_X * GRID_Y, ptr);
    fclose(ptr);

    updateTiles(coords);
}
