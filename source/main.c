#include <stdio.h>
#include <time.h>
#include <string.h>
#include <nds.h>
#include <filesystem.h>
#include <fat.h>
#include <sys/stat.h>
#include <nf_lib.h>
#include <dirent.h>
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
void testWriteSizes()
{
    FILE *f;
    size_t n;

    f = fopen("fat:/YouMakeLevels/test1k.bin", "wb");
    n = fwrite(TILE_MAP, 1, 1024, f);
    fclose(f);

    f = fopen("fat:/YouMakeLevels/test64k.bin", "wb");
    n = fwrite(TILE_MAP, 1, 65536, f);
    fclose(f);

    f = fopen("fat:/YouMakeLevels/test512k.bin", "wb");
    n = fwrite(TILE_MAP, 1, 524288, f);
    fclose(f);
}


void showDirs(gameContext *ctx)
{
    static char levels[512][16];
    static int maxEntries = 0;
    static bool alreadyRead = false;
    static int position = 0;
    static int frame = 0;

    if (!alreadyRead)
    {
        DIR *dr = opendir("fat:/YouMakeLevels/");
        struct dirent *de;
        while (dr != NULL && (de = readdir(dr)) != NULL)
        {
            strncpy(levels[maxEntries], de->d_name, 15);
            levels[maxEntries][15] = '\0';
            maxEntries++;
        }
        if (dr != NULL) closedir(dr);
        alreadyRead = true;
    }

    frame = (frame + 1) % 60;

    if (ctx->input->buttonsUp & KEY_UP)
    {
        position--;
        if (position < 0) position = 0;
    }
    if (ctx->input->buttonsUp & KEY_DOWN)
    {
        position++;
        if (position >= maxEntries) position = maxEntries - 1;
    }

    int textRowCounter = 0;
    for (int i = 0; i < maxEntries; i++)
    {
        if (strchr(levels[i], '.') != NULL) continue;
        if (i < position) continue;

        bool isSelected = (i == position);
        bool blinkOn = (frame <= 30);

        if (!isSelected || blinkOn)
        {
            NF_WriteText(0, 0, 1, (textRowCounter % 32) + 1, levels[i]);
        }

        if (isSelected)
        {
            NF_WriteText(0, 0, 1, 0, levels[i]);

            if (ctx->input->buttonsUp & KEY_A)
            {
                state = EDITOR;
                char dir[64] = "fat:/YouMakeLevels/";
                strcat(dir, levels[i]);
                loadLevel(dir, ctx);
            }
        }

        textRowCounter += 2;
    }
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

    for (int i = 0; i < 12; i++)
    {
        NF_CreateSprite(1,18 +i,5,5,18 * i,45);
        NF_SpriteFrame(1,i + 18,i);

        NF_EnableSpriteRotScale(1, 18 + i, i + 1, true);
        NF_SpriteRotScale(1,i + 1,0,256 + 128,256 + 128);

    }
    showKeyboard(false);


    state = LEVEL_LOAD;


    //testWriteSizes();
    while (1)
    {

        NF_ClearTextLayer(0, 0);
        scanKeys();
         ctx.input->buttonsDown = keysDown();
         ctx.input->buttonsHeld = keysHeld();
         ctx.input->buttonsUp = keysUp();


        switch(state){
            case MAIN_MENU:
                state = EDITOR;
            break;
            case EDITOR:
                debugText(&ctx);
                NF_ShowSprite(1,5,false);
                NF_ShowSprite(1,0,true);
                editorFrame(&ctx);
                scrollLogic(&ctx);
                NF_MoveSprite(1,0,(ctx.editor->currentBlock * 20) + 30,3);
                if(state == PLAY_SCREEN){

                    ctx.player->playerX = ctx.editor->flagPosX*16;
                    ctx.player->playerY = ctx.editor->flagPosY*16;

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
                    }
                    //todo add a cancel button
                }
                NF_WriteText(0,0,0,14,word);

            case LEVEL_LOAD:
                showDirs(&ctx);


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
