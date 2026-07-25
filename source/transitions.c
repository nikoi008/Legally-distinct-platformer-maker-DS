#include "transitions.h"

void fadeOut(int screen, int frames)
{
    for(int b = 0; b >= -16; b--)
    {
        setBrightness(screen, b);
        for(int f = 0; f < frames; f++)
        {
            swiWaitForVBlank();
        }
    }
}

void fadeIn(int screen, int frames)
{
    for(int b = -16; b <= 0; b++)
    {
        setBrightness(screen, b);
        for(int f = 0; f < frames; f++)
        {
            swiWaitForVBlank();
        }
    }
}

void editorToPlayScreen(gameContext *ctx)
{
    fadeOut(3, 4);
    showEditor(false);
    ctx->player->playerX = ctx->editor->flagPosX * 16;
    ctx->player->playerY = ctx->editor->flagPosY * 16;
    ctx->coords->cameraX = ctx->player->playerX - 128;
    ctx->coords->cameraY = ctx->player->playerY - 96;
    ctx->coords->cameraTileX = coordsToTile(ctx->coords->cameraX);
    ctx->coords->cameraTileY = coordsToTile(ctx->coords->cameraY);
    ctx->coords->scrollX = 128;
    ctx->coords->scrollY = 160;

    updateOrigin(ctx);
    updateTiles(ctx);
    NF_ShowSprite(1, 1, false);
    NF_ShowSprite(1, 2, false);
    NF_ShowSprite(1, 3, false);
    NF_ShowSprite(1, 4, false);
    NF_DeleteTiledBg(1, 2);
    lcdSwap();
    for(int i = 0; i < 10; i++)
    {
        NF_ShowSprite(1, i + 6, false);
    }
    scrollLogic(ctx);
    updateTiles(ctx);

    NF_ShowSprite(1, 5, true);
    if(ctx->player->hflip == false)
    {
        NF_MoveSprite(1, 5, 128, 96 - 16);
    }
    else
    {
        NF_MoveSprite(1, 5, 128 - 16, 96 - 16);
    }

    NF_ClearTextLayer(0, 0);
    NF_SpriteOamSet(0);
    NF_SpriteOamSet(1);
    swiWaitForVBlank();
    NF_UpdateVramMap(1, 3);
    NF_ScrollBg(1, 3, ctx->coords->scrollX, ctx->coords->scrollY);
    NF_ScrollBg(1, 2, ctx->coords->cameraX % 16, ctx->coords->cameraY % 16);
    NF_UpdateTextLayers();
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);

    playerAnim(ctx);
    playerPhysics(ctx);
    playerScroll(ctx);

    scrollLogic(ctx);
    fadeIn(3, 2);
}

void playScreenToEditor(gameContext *ctx)
{
    fadeOut(3, 4);
    NF_CreateTiledBg(1, 2, "grid");
    showEditor(true);
    lcdSwap();
    ctx->coords->cameraX = ctx->editor->flagPosX * 16- 64; //mysterious offset todo figure out
    ctx->coords->cameraY = ctx->editor->flagPosY * 16 - 64;
    ctx->coords->scrollX = 128;
    ctx->coords->scrollY = 160;
    updateTiles(ctx);
    updateOrigin(ctx);
    for(int i = 0; i < 10; i++)
    {
        NF_ShowSprite(1, i + 6, true);
    }
    scrollLogic(ctx);
    updateTiles(ctx);
    updateOrigin(ctx);
    state = EDITOR;
    //debugText(ctx);
    NF_ShowSprite(1, 5, false);
    NF_ShowSprite(1, 0, true);
    editorFrame(ctx);
    NF_MoveSprite(1, 0, (ctx->editor->currentBlock * 20) + 30, 3);
    NF_SpriteOamSet(0);
    NF_SpriteOamSet(1);
    swiWaitForVBlank();
    NF_UpdateVramMap(1, 3);
    NF_ScrollBg(1, 3, ctx->coords->scrollX, ctx->coords->scrollY);
    NF_ScrollBg(1, 2, ctx->coords->cameraX % 16, ctx->coords->cameraY % 16);
    NF_UpdateTextLayers();
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
    fadeIn(3, 2);
}