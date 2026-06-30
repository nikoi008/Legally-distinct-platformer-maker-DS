#include <stdio.h>
#include <time.h>
#include <string.h>
#include <nds.h>
#include <filesystem.h>
#include <fat.h>
#include <sys/stat.h>
#include <nf_lib.h>

#include "defines.h"
#include "types.h"
#include "globals.h"
#include "blocks.h"
#include "tilemap.h"
#include "editor.h"
#include "camera.h"
#include "player.h"
#include "debug.h"
#include "init.h"

int main(int argc, char **argv){
    editorContext editor = {};
    playerContext player = {};
    inputs input = {};
    worldCoordinates coords = {};

    gameContext ctx = {};

    ctx.player = &player;
    ctx.editor = &editor;
    ctx.input = &input;
    ctx.coords = &coords;
    consoleDebugInit(DebugDevice_NOCASH);
    fatInitDefault();
    srand(time(NULL));
    initialise();
    mkdir("fat:/YouMakeLevels", 0777);
    NF_LoadTilesForBg("bg/tiles", "tiles", 512, 512, 0, TOTAL_BLOCKS + 200);//todo dont just add +10,
    NF_CreateTiledBg(1, TILE_LAYER, "tiles");


    NF_LoadTiledBg("bg/GRID","grid",512,256);
    NF_CreateTiledBg(1,2,"grid");

    NF_LoadSpriteGfx("bg/animatedHUD", 0, 64,32);
    NF_LoadSpritePal("bg/animatedHUD", 0);//todo make dedicated sprite folder


    NF_VramSpriteGfx(1, 0, 0, false);
    NF_VramSpritePal(1, 0, 0);

  

    NF_CreateSprite(1, 1, 0, 0, 0, 0);
    NF_SpriteFrame(1, 1, 0);
    NF_CreateSprite(1, 2, 0, 0, 64, 0);
    NF_SpriteFrame(1, 2, 1);
    NF_CreateSprite(1, 3, 0, 0, 128, 0);
    NF_SpriteFrame(1, 3, 2); //all hud elements + frames
    NF_CreateSprite(1, 4, 0, 0, 192, 0);
    NF_SpriteFrame(1, 4, 3);

    NF_LoadSpriteGfx("bg/highlight",1,16,16);
    NF_LoadSpritePal("bg/highlight",1);
    NF_VramSpriteGfx(1,1,1,true);
    NF_VramSpritePal(1,1,1);
    NF_CreateSprite(1,0,1,1,29,3);

    NF_LoadSpriteGfx("bg/player",2,32,32); //todo probably make the sprite folder somewhat soon xx
    NF_LoadSpritePal("bg/player",2);
    NF_VramSpriteGfx(1,2,2,false);
    NF_VramSpritePal(1,2,2);
    NF_CreateSprite(1,5,2,2,100,50);
    initBlocks(&ctx);
    

    NF_LoadSpriteGfx("bg/tileSprites",3,16,16);
    NF_LoadSpritePal("bg/tileSprites",3);
    NF_VramSpriteGfx(1,3,3,false);
    NF_VramSpritePal(1,3,3);

    for(int i = 0; i < 10; i++){
        NF_CreateSprite(1,i + 6,3,3,30 + (i * 20),3);
        NF_SpriteFrame(1,i + 6,0);
    }

     NF_SpriteFrame(1,6,1);
     NF_SpriteFrame(1,7,2);
     NF_SpriteFrame(1,8,3);
     NF_SpriteFrame(1,9,4);
     NF_SpriteFrame(1,10,5);




     ctx.player->grounded = true;
     ctx.editor->currentBlock = 1;
     ctx.editor->rectFillOn = false;
     ctx.editor->firstTouchX = -1;
     ctx.editor->firstTouchY = -1;
    ctx.editor->leftMostX = 1024;
    ctx.editor->leftMostY = 1024;

     ctx.editor->flagPosX = -1;
    ctx.editor->flagPosY = -1;
     ctx.coords->cameraY =( GRID_Y * 16 )/2;


    while (1)
    {
        fprintf(stderr, "Frame: %%frame%%\n");
        scanKeys();
         ctx.input->buttonsDown = keysDown();
         ctx.input->buttonsHeld = keysHeld();
         ctx.input->buttonsUp = keysUp();


        switch(state){
            case MAIN_MENU:
            state = EDITOR;
            break;
            case EDITOR:
                NF_ShowSprite(1,5,false);
                NF_ShowSprite(1,0,true);
                //editorFrame(&coords,&input,&ctxE);
                //editorFrame(gameContext *ctx);
                editorFrame(&ctx);
                scrollLogic(&ctx);
                NF_MoveSprite(1,0,(ctx.editor->currentBlock * 20) + 30,3);
                if(state == PLAY_SCREEN){
                    if (ctx.editor->flagPosX >= 0 && ctx.editor->flagPosY >= 0)
                    {
                        ctx.player->playerX = ctx.editor->flagPosX*16;
                        ctx.player->playerY = ctx.editor->flagPosY*16;
                    }else
                    {
                        ctx.player->playerX = ctx.editor->leftMostX * 16;
                        ctx.player->playerY = ctx.editor->leftMostY * 16;

                    }
                    ctx.coords->cameraX = ctx.player->playerX - 128;
                    ctx.coords->cameraY = ctx.player->playerY - 96;
                    ctx.coords->scrollX = 128;
                    ctx.coords->scrollY = 160;
                    for (int i = 0; i < 10; i++)
                    {
                        NF_ShowSprite(1,i + 6,false);
                    }
                    scrollLogic(&ctx);
                    updateTiles(&ctx);
                }
                break;


            case PLAY_SCREEN:
                NF_ShowSprite(1,5,true);
                playerAnim(&ctx);
                playerPhysics(&ctx);
                playerScroll(&ctx);
                if(ctx.player->hflip == false){
                    NF_MoveSprite(1, 5, 128, 96 - 16);
                }else{
                    NF_MoveSprite(1, 5, 128 - 16, 96 - 16);
                }
                //NF_MoveSprite(1, 0, 128, 96); uncomment to see hitbox
                NF_ShowSprite(1,0,false); //COMMENT TO SEE hitbx

                if(ctx.input->buttonsDown & KEY_TOUCH){ // todo add some sort of bounds ie bottom left corner
                    NF_ShowSprite(1,1,true);
                    NF_ShowSprite(1,2,true);
                    NF_ShowSprite(1,3,true);
                    NF_ShowSprite(1,4,true);
                    NF_CreateTiledBg(1,2,"grid");
                    lcdSwap();
                    state = EDITOR;
                    ctx.coords->cameraX = ctx.editor->flagPosX - 64;//mysterious offset todo figure out
                    ctx.coords->cameraY = ctx.editor->flagPosY * 16 - 64;
                    ctx.coords->scrollX = 128;
                    ctx.coords->scrollY = 160;
                    for (int i = 0; i < 10; i++)
                    {
                        NF_ShowSprite(1,i + 6,true);
                    }
                    scrollLogic(&ctx);
                    updateTiles(&ctx);
                    updateOrigin(&ctx);
                }
                break;
        }
        debugText(&ctx);
        NF_SpriteOamSet(0);
        NF_SpriteOamSet(1);
        swiWaitForVBlank();
        NF_UpdateVramMap(1, 3);
        NF_ScrollBg(1, 3,coords.scrollX,coords.scrollY);
        NF_ScrollBg(1,2,coords.cameraX % 16,coords.cameraY % 16);
        NF_UpdateTextLayers();
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }
    return 0;

}
