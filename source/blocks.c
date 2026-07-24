#include "blocks.h"
#include "globals.h"
#include "defines.h"
#include <nds.h>
#include <nf_lib.h>
#include "types.h"
#include "camera.h"
#include "tilemap.h"

void backToEditor(gameContext *ctx)
{

    state = EDITOR;
}

void initBlocks(gameContext *ctx)
{
    blocks[0].solid = false; //air
    blocks[0].ifTouched = NULL;
    blocks[0].topRightTile = 0;
    blocks[0].topLeftTile = 0;
    blocks[0].bottomLeftTile = 0;
    blocks[0].bottomRightTile = 0;

    blocks[1].solid = true; //dirt
    blocks[1].ifTouched = NULL;
    blocks[1].topLeftTile = 1;
    blocks[1].topRightTile = 2;
    blocks[1].bottomLeftTile = 3;
    blocks[1].bottomRightTile = 4;

    blocks[2].solid = false; //flag
    blocks[2].ifTouched = NULL;
    blocks[2].topLeftTile = 5;
    blocks[2].topRightTile = 6;
    blocks[2].bottomLeftTile = 7;
    blocks[2].bottomRightTile = 8;

    blocks[3].solid = true; //spike
    blocks[3].ifTouched = backToEditor;
    blocks[3].topLeftTile = 9;
    blocks[3].topRightTile = 10;
    blocks[3].bottomLeftTile = 11;
    blocks[3].bottomRightTile = 12;

    blocks[4].solid = false; //end flag
    blocks[4].ifTouched = backToEditor;
    blocks[4].topLeftTile = 13;
    blocks[4].topRightTile = 14;
    blocks[4].bottomLeftTile = 15;
    blocks[4].bottomRightTile = 16;
}
bool tileSolid(int tileX, int tileY)
{
    return blocks[TILE_MAP[tileY][tileX]].solid;
}

int coordsToTile(int coord)
{
    return coord / 16;
}

void addTile(int tileX, int tileY, int currentBlock, editorContext *ctx)
{
    if(tileX < 0 || tileX >= GRID_X || tileY < 0 || tileY >= GRID_Y)
        return;
    if(ctx->currentBlock == 2)
    {
        TILE_MAP[ctx->flagPosY][ctx->flagPosX] = 0;
        ctx->flagPosX = tileX;
        ctx->flagPosY = tileY;
    }
    if(ctx->flagPosX >= 0 && ctx->flagPosY >= 0 && ctx->currentBlock == 0 && TILE_MAP[tileY][tileX] == 2)
    {
        ctx->flagPosX = -1;
        ctx->flagPosY = -1;
    }
    TILE_MAP[tileY][tileX] = currentBlock;
}
