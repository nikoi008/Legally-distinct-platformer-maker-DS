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
#include "input.h"
#include "camera.h"
#include "player.h"
#include "debug.h"
#include "init.h"

int main(int argc, char **argv){
    fatInitDefault();
    srand(time(NULL));
    initialise();
    mkdir("fat:/YouMakeLevels", 0777);
    NF_LoadTilesForBg("bg/tiles", "tiles", 512, 512, 0, TOTAL_BLOCKS + 200);//todo dont just add +10,
    NF_CreateTiledBg(1, TILE_LAYER, "tiles");


    NF_LoadTiledBg("bg/GRID","grid",512,256);
    NF_CreateTiledBg(1,2,"grid");

    NF_LoadSpriteGfx("bg/animatedSpriteHud", 0, 64,32);
    NF_LoadSpritePal("bg/animatedSpriteHud", 0);//todo make dedicated sprite folder


    NF_VramSpriteGfx(1, 0, 0, false);
    NF_VramSpritePal(1, 0, 0);

    NF_CreateSprite(1, 4, 0, 0, 0, 0);

    NF_CreateSprite(1, 1, 0, 0, 64, 0);
    NF_SpriteFrame(1, 1, 1);
    NF_CreateSprite(1, 2, 0, 0, 128, 0);
    NF_SpriteFrame(1, 2, 3);
    NF_CreateSprite(1, 3, 0, 0, 192, 0);
    NF_SpriteFrame(1, 3, 2); //all hud elements + frames

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
    initBlocks();

    editorContext ctxE = {};
    playerContext ctxP = {};
    ctxP.grounded = true;
    ctxE.currentBlock = 1;
    ctxE.rectFillOn = false;
    ctxE.firstTouchX = -1;
    ctxE.firstTouchY = -1;
    worldCoordinates coords = {};
    coords.cameraY =( GRID_Y * 16 )/2;
    inputs input = {};

    while (1)
    {

        scanKeys();
        input.buttonsDown = keysDown();
        input.buttonsHeld = keysHeld();
        input.buttonsUp = keysUp();


        switch(state){
            case MAIN_MENU:
            state = EDITOR;
            break;
            case EDITOR:
                NF_ShowSprite(1,5,false);
                doInputsEditor(&coords,&input,&ctxE);
                scrollLogic(&coords);
                NF_MoveSprite(1,0,(ctxE.currentBlock * 18) + 29,3);
                if(state == PLAY_SCREEN){
                    ctxP.playerX = ctxE.flagPosX*16;
                    ctxP.playerY = ctxE.flagPosY*16;
                    coords.cameraX = ctxP.playerX - 128;
                    coords.cameraY = ctxP.playerY - 96;
                    coords.scrollX = 128;
                    coords.scrollY = 160;
                    scrollLogic(&coords);
                    updateTiles(&coords);
                }
                break;


            case PLAY_SCREEN:
                NF_ShowSprite(1,5,true);
                playerAnim(&ctxP);
                playerPhysics(&ctxP, &input, &coords);
                playerScroll(&coords, &ctxP);
                if(ctxP.hflip == false){
                    NF_MoveSprite(1, 5, 128, 96 - 16);
                }else{
                   NF_MoveSprite(1, 5, 128 - 16, 96 - 16);
                }
                NF_MoveSprite(1, 0, 128, 96);

                if(input.buttonsDown & KEY_TOUCH){ // todo add some sort of bounds ie bottom left corner
                    NF_ShowSprite(1,1,true);
                    NF_ShowSprite(1,2,true);
                    NF_ShowSprite(1,3,true);
                    NF_ShowSprite(1,4,true);
                    NF_CreateTiledBg(1,2,"grid");
                    lcdSwap();
                    state = EDITOR;
                    coords.cameraX = ctxE.flagPosX - 64;//mysterious offset todo figure out
                    coords.cameraY = ctxE.flagPosY * 16 - 64;
                    coords.scrollX = 128;
                    coords.scrollY = 160;
                    scrollLogic(&coords);
                    updateTiles(&coords);
                    updateOrigin(&coords);
                }
                break;
        }
        debugText(&coords, &input,&ctxE,&ctxP);
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
