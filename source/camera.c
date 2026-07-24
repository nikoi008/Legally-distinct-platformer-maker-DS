#include "camera.h"
#include "blocks.h"
#include "tilemap.h"

void updateOrigin(gameContext *ctx)
{
    ctx->coords->originTileX = ctx->coords->cameraTileX - 8;
    ctx->coords->originTileY = ctx->coords->cameraTileY - 10;
}

void scrollLogic(gameContext *ctx)
{
    int jitteroffsetX = ctx->coords->cameraX % 16;
    int jitterOffsetY = ctx->coords->cameraY % 16;
    ctx->coords->cameraTileX = coordsToTile(ctx->coords->cameraX);
    ctx->coords->cameraTileY = coordsToTile(ctx->coords->cameraY);
    int sX = ctx->coords->scrollX;
    int sY = ctx->coords->scrollY;
    if(sX < 16 || sX > 240 /*512-16-256*/ || sY < 16 || sY > 304 /*512 - 192 - 16*/)
    {
        ctx->coords->scrollX = 128 + jitteroffsetX;
        ctx->coords->scrollY = 160 + jitterOffsetY;
        updateOrigin(ctx);
        updateTiles(ctx);
    }
}

void playerScroll(gameContext *ctx)
{
    ctx->coords->cameraX = ctx->player->playerX - 128;
    ctx->coords->cameraY = ctx->player->playerY - 96;
    ctx->coords->cameraTileX = coordsToTile(ctx->coords->cameraX);
    ctx->coords->cameraTileY = coordsToTile(ctx->coords->cameraY);
    ctx->coords->scrollX = 128 + (ctx->coords->cameraX % 16);
    ctx->coords->scrollY = 160 + (ctx->coords->cameraY % 16);
    updateOrigin(ctx);
    updateTiles(ctx);
}
