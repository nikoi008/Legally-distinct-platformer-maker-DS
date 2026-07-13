#include <stdio.h>
#include <time.h>
#include <string.h>
#include <nds.h>
#include <filesystem.h>
#include <fat.h>
#include <sys/stat.h>
#include <nf_lib.h>
#include <dirent.h>
#include <dswifi9.h>

#include "blocks.h"
#include "tilemap.h"
#include "editor.h"
#include "camera.h"
#include "defines.h"
#include "types.h"
#include "globals.h"
#include "player.h"
#include "debug.h"
#include "init.h"
#include "keyboard.h"
#include "file_handling.h"
#include "level_sharing.h"
#include "transitions.h"
#include "change_palette.h"

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


    NF_LoadTiledBg("bg/GRID","grid",256,256);
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
    ctx.coords->cameraX =( GRID_X * 16 )/2;
    ctx.coords->cameraY =( GRID_Y * 16 )/2;

    NF_LoadSpriteGfx("bg/key",4,64,64);
    NF_LoadSpritePal("bg/key",4);
    NF_VramSpritePal(1,4,4);
    NF_VramSpriteGfx(1,4,4,false);

    NF_CreateSprite(1,16,4,4,300,300);
    NF_EnableSpriteRotScale(1, 16, 0, true);
    NF_SpriteRotScale(1,0,0,256 + 128,256 + 128);

    NF_CreateSprite(1,17,4,4,300,300);
    NF_SpriteFrame(1,17,1);
    NF_EnableSpriteRotScale(1, 17, 1, true);
    NF_SpriteRotScale(1,1,0,256 + 128,256 + 128);


    NF_LoadSpriteGfx("bg/letters",5,8,8);
    NF_LoadSpritePal("bg/letters",5);
    NF_VramSpriteGfx(1,5,5,false);
    NF_VramSpritePal(1,5,5);

    /*NF_LoadSpriteGfx("bg/uiAssets",6,16,16);
    NF_LoadSpritePal("bg/uiAssets",6);
    NF_VramSpriteGfx(1,6,6,false);
    NF_VramSpritePal(1,6,6);

    NF_CreateSprite(1,37,6,6,10,173);
    NF_CreateSprite(1,31,6,6,30,173);
    NF_CreateSprite(1,32,6,6,50,173);
    NF_CreateSprite(1,33,6,6,70,173);

    NF_SpriteFrame(1,31,1);
    NF_SpriteFrame(1,32,2);
    NF_SpriteFrame(1,33,3);*/



    NF_LoadSpriteGfx("bg/bottomHud",7,64,32);
    NF_LoadSpritePal("bg/bottomHud",7);
    NF_VramSpriteGfx(1,7,7,false);
    NF_VramSpritePal(1,7,7);

    NF_CreateSprite(1,34,7,7,0,192-22); //todo probably make a define for this
    NF_CreateSprite(1,35,7,7,64,192-22);
    NF_CreateSprite(1,36,7,7,128,192-22);
    NF_CreateSprite(1,38,7,7,192,192-22);

    NF_SpriteFrame(1,35,1);
    NF_SpriteFrame(1,36,2);
    NF_SpriteFrame(1,38,3);


   // NF_CreateSprite(1,30,1,1,10,173);

    NF_LoadSpriteGfx("bg/incrementbtn",8,64,32);
    NF_LoadSpritePal("bg/incrementbtn",8);
    NF_VramSpriteGfx(1,8,8,false);
    NF_VramSpritePal(1,8,8);

    NF_CreateSprite(1,39,8,8,16,48);
    NF_CreateSprite(1,40,8,8,16,80);
    NF_CreateSprite(1,41,8,8,16,112);

    NF_CreateSprite(1,42,8,8,176,48);
    NF_CreateSprite(1,43,8,8,176,80);
    NF_CreateSprite(1,44,8,8,176,112);

    NF_SpriteFrame(1,42,1);
    NF_SpriteFrame(1,43,1);
    NF_SpriteFrame(1,44,1);





    for (int i = 0; i < 12; i++)
    {
        NF_CreateSprite(1,18 +i,5,5,18 * i,45);
        NF_SpriteFrame(1,i + 18,i);

        NF_EnableSpriteRotScale(1, 18 + i, i + 1, true);
        NF_SpriteRotScale(1,i + 1,0,256 + 128,256 + 128);

    }

    for (int i = 0; i < 16; i++)
    {
        NF_CreateSprite(1,45 +i,5,5, 8 * i,45);
    }
    NF_CreateSprite(1,63,8,8,192,176);
    NF_SpriteFrame(1,63,2);
    NF_CreateSprite(1,64,8,8,0,176);
    NF_SpriteFrame(1,64,3);
    showKeyboard(false);

    //showEditor(false);
    state = EDITOR;
    showChangePalWindow(false);

    //testWriteSizes();
    Wifi_InitDefault(INIT_ONLY | WIFI_LOCAL_ONLY);

    while (1)
    {

        if (state != SHARE_LEVEL_CLIENT && state != SHARE_LEVEL_HOST)
        {
            NF_ClearTextLayer(0, 0);
        }
        scanKeys();
         ctx.input->buttonsDown = keysDown();
         ctx.input->buttonsHeld = keysHeld();
         ctx.input->buttonsUp = keysUp();

        if (ctx.input->buttonsDown & KEY_Y)
        {
            state = SHARE_LEVEL_CLIENT;
        }
        if (ctx.input->buttonsDown & KEY_X)
        {
            state = SHARE_LEVEL_HOST;
        }
        switch(state){
            case MAIN_MENU:
            if (ctx.input->buttonsDown & KEY_A)
            {
                fadeIn(3,2);
            }
            if (ctx.input->buttonsDown & KEY_B)
            {
                fadeOut(3,2);
            }
                //state = EDITOR;
                nocashMessage("main menu");

            break;
            case EDITOR:
                debugText(&ctx);
                NF_ShowSprite(1,5,false);
                NF_ShowSprite(1,0,true);
                editorFrame(&ctx);
                scrollLogic(&ctx);
                NF_MoveSprite(1,0,(ctx.editor->currentBlock * 20) + 30,3);
                if(state == PLAY_SCREEN){

                    editorToPlayScreen(&ctx);
                }
                if (state == CHANGE_PAL)
                {
                    showChangePalWindow(true);
                    showEditor(false);
                }
                break;


            case PLAY_SCREEN:
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

                if(ctx.input->buttonsDown & KEY_TOUCH || state == EDITOR){ // todo add some sort of bounds ie bottom left corner
                    playScreenToEditor(&ctx);
                }

                break;


            case LEVEL_SAVE:
                static char word[12] = "";
                static bool returned = false;
                if (returned == false)
                {
                    if (keyboardLoop(12, word,&ctx) == '+')
                    {

                        returned = true;
                        showKeyboard(false);
                        ctx.input->keyboardOn = false;
                        char dir[64] = "fat:/YouMakeLevels/";
                        strcat(dir,word);
                        saveLevel(dir);
                        word[0] = '\0';
                    }
                    //todo add a cancel button
                }
                NF_WriteText(0,0,0,14,word);
            break;

            case LEVEL_LOAD:
                showDirs(&ctx);
                break;

            case CHANGE_PAL:
            {
                changePaletteFrame(&ctx);
                break;
            }

        case SHARE_LEVEL_HOST:
            hostMode();
            if (state == EDITOR)
            {
                updateTiles(&ctx);
            }
            break;

        case SHARE_LEVEL_CLIENT:
            ClientMode();
            if (state == EDITOR)
            {
                updateTiles(&ctx);
            }
            break;
        }
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
