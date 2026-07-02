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
char keyboardPresses(gameContext *ctx)
{
    char layer1[10] = {'0','1','2','3','4','5','6','7','8','9'}; // rect bounds from x40 y80 to x204 y 104
    char layer2[10] = {'Q','W','E','R','T','Y','U','I','O','P'}; // x40 to y104 x204 y126
    char layer3[9] = {'A','S','D','F','G','H','J','K','L'}; //x48 y128 to x198 y148
    char layer4[7] = {'Z','X','C','V','B','N','M'}; //x64 y 152 to x180 y170

    char keyPressed[3];
    int  tX = ctx->input->touchPos.px;
    int tY = ctx->input->touchPos.py;
    if (ctx->input-> buttonsUp & KEY_TOUCH)
    {
        touchRead(&ctx->input->touchPos);
        if (tX > 40 && tX < 204 && tY > 80 && tY < 104)
        {
            nocashMessage("layer 1");
            char key[2];
            key[0] = layer1[(tX - 41) / 16];
            key[1] = '\0';
            nocashMessage(key);
            return key[0];

        }

        else if (tX > 40 && tX < 204 && tY > 104 && tY < 126)
        {
            nocashMessage("layer 2");
            //Q 41 W 57,E 73
            //algo is (tx - 41) / 16
            char key[2];
            key[0] = layer2[(tX - 41) / 16];
            key[1] = '\0';
            nocashMessage(key);
            return key[0];

        }

        else if (tX > 48 && tX < 198 && tY > 128 && tY < 148)
        {
            nocashMessage("layer 3");
            //64,80
            //algo is (tx - 64) / 16
            //+ 16????????
            // literally no clue why i have to add 16 but we ball
            char key[2];
            key[0] = layer3[(tX - 48) / 16];
            key[1] = '\0';
            nocashMessage(key);
            return key[0];
        }

        else if (tX > 64 && tX < 180  && tY > 152 && tY < 170)
        {
            //80,96
            //algo is (tx - 80) /16 mysterious +16 offset strikes again

            nocashMessage("layer 4");
            char key[2];
            key[0] = layer4[(tX - 64) / 16];
            key[1] = '\0';
            nocashMessage(key);
            return key[0];
        }

        else if (tX > 37 && tX < 218 && tY > 168 && tY < 191)
        {
            nocashMessage("space");
            return ' ';
        }

        else if ((tX > 203 && tX < 218 && tY > 80 && tY < 146) || (tX > 195 && tX < 218 && tY > 125 && tY < 146))
        {
            //203 80 218 146
            //195 125
            nocashMessage("return");
            return '+';
        }

        else if (tX > 180 && tX < 217 && tY > 147 && tY < 168)
        {
            //180 147, 217 168
            nocashMessage("backspace");
            return '-';
        }

    }




}

char keyboardLoop(int maxChars, char *string)
{

}
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

    NF_LoadSpriteGfx("bg/key",4,64,64);
    NF_LoadSpritePal("bg/key",4);
    NF_VramSpritePal(1,4,4);
    NF_VramSpriteGfx(1,4,4,false);

    NF_CreateSprite(1,16,4,4,37,80);
    NF_EnableSpriteRotScale(1, 16, 0, true);
    NF_SpriteRotScale(1,0,0,256 + 128,256 + 128);
    NF_CreateSprite(1,17,4,4,128 + 37,80);
    NF_SpriteFrame(1,17,1);
    NF_EnableSpriteRotScale(1, 17, 1, true);
    NF_SpriteRotScale(1,1,0,256 + 128,256 + 128);



    state = MAIN_MENU;
    while (1)
    {


        scanKeys();
         ctx.input->buttonsDown = keysDown();
         ctx.input->buttonsHeld = keysHeld();
         ctx.input->buttonsUp = keysUp();
         keyboardPresses(&ctx);

        switch(state){
            case MAIN_MENU:
            state = EDITOR;
            break;
            case EDITOR:
                NF_ShowSprite(1,5,false);
                NF_ShowSprite(1,0,true);
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
                        //ctx.player->playerX = ctx.editor->leftMostX * 16;
                        //ctx.player->playerY = ctx.editor->leftMostY * 16;

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
