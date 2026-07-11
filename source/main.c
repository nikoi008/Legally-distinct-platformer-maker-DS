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
#include "file_handling.h"
#include <dswifi9.h>


typedef struct __attribute__((packed))
{
    u8 worldChunk[512];
    u16 chunkNumber;
} hostPktToClient;

typedef struct __attribute__((packed)) {
    u16 chunkRecieved;
} clientPktToHost;

#define MAX_CLIENTS 1
#define NUM_CHUNKS 512
#define RETRY_TIMEOUT_FRAMES 60

Wifi_AccessPoint AccessPoint;
int chunkRecievedYay = -1;

void SendHostStateToClients(int chunkNumber)
{
    hostPktToClient hostPacket;
    hostPacket.chunkNumber = chunkNumber;

    for (int i = 0; i < 512; i++)
    {
        hostPacket.worldChunk[i] = TILE_MAP[chunkNumber][i];
    }

    Wifi_MultiplayerHostCmdTxFrame(&hostPacket, sizeof(hostPacket));
}

void FromHostPacketHandler(Wifi_MPPacketType type, int base, int len)
{
    if (len < sizeof(hostPktToClient))
    {
        return;
    }

    if (type != WIFI_MPTYPE_CMD)
        return;

    hostPktToClient packet;
    Wifi_RxRawReadPacket(base, sizeof(packet), (void *)&packet);

    if (packet.chunkNumber >= NUM_CHUNKS)
        return;

    for (int i = 0; i < 512; i++)
    {
        TILE_MAP[packet.chunkNumber][i] = packet.worldChunk[i];
    }
    clientPktToHost packetC;
    packetC.chunkRecieved = packet.chunkNumber;
    Wifi_MultiplayerClientReplyTxFrame(&packetC, sizeof(packetC));
}

void FromClientPacketHandler(Wifi_MPPacketType type, int aid, int base, int len)
{
    if (len < sizeof(clientPktToHost))
    {
        return;
    }

    if (type != WIFI_MPTYPE_REPLY)
        return;

    clientPktToHost packet;
    Wifi_RxRawReadPacket(base, sizeof(packet), (void *)&packet);
    chunkRecievedYay = packet.chunkRecieved;
}

void hostMode()
{
    Wifi_MultiplayerHostMode(MAX_CLIENTS, sizeof(hostPktToClient), sizeof(clientPktToHost));
    Wifi_MultiplayerFromClientSetPacketHandler(FromClientPacketHandler);

    while (!Wifi_LibraryModeReady()) swiWaitForVBlank();
    Wifi_SetChannel(6);
    Wifi_MultiplayerAllowNewClients(true);

    Wifi_BeaconStart("NintendoDS", 0xCAFEF00D);
    swiWaitForVBlank();
    swiWaitForVBlank();

    while (1)
    {

        scanKeys();
        u16 keys_down = keysDown();
        swiWaitForVBlank();

        int num_clients = Wifi_MultiplayerGetNumClients();
        u16 players_mask = Wifi_MultiplayerGetClientMask();
        Wifi_ConnectedClient client[MAX_CLIENTS];
        num_clients = Wifi_MultiplayerGetClients(MAX_CLIENTS, &(client[0]));


        if ((keys_down & KEY_A) && num_clients > 0)
            break;
    }
    Wifi_MultiplayerAllowNewClients(false);

    while (1)
    {
        for (int chunk = 0; chunk < NUM_CHUNKS; chunk++)
        {
            chunkRecievedYay = -1;


            int framesWaited = 0;
            SendHostStateToClients(chunk);

            while (chunkRecievedYay != chunk)
            {
                swiWaitForVBlank();
                framesWaited++;

                if (framesWaited >= RETRY_TIMEOUT_FRAMES)
                {

                    if (Wifi_MultiplayerGetClientMask() == 0)
                        goto client_lost;

                    SendHostStateToClients(chunk);
                    framesWaited = 0;
                }
            }
        }
    }

client_lost:
    return;
}

bool AccessPointSelectionMenu()
{

    Wifi_ScanMode();
    int numAPs = Wifi_GetNumAP();

    if (numAPs <= 0)
        return false;

    Wifi_AccessPoint ap;
    Wifi_GetAPData(0, &ap);
    AccessPoint = ap;
    return true;
}

void ClientMode()
{
    connect:
    if (!AccessPointSelectionMenu())
        goto end;

    Wifi_MultiplayerFromHostSetPacketHandler(FromHostPacketHandler);
    Wifi_ConnectOpenAP(&AccessPoint);

    while (1)
    {
        swiWaitForVBlank();
        int status = Wifi_AssocStatus();
        if (status == ASSOCSTATUS_ASSOCIATED)
            break;
    }
    end:
    Wifi_IdleMode();
}








int clampInt(int val,int min, int max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void fadeOut(int screen, int frames) {
    for (int b = 0; b >= -16; b--) {
        setBrightness(screen, b);
        for (int f = 0; f < frames; f++) {
            swiWaitForVBlank();
        }
    }
}

void fadeIn(int screen, int frames) {
    for (int b = -16; b <= 0; b++) {
        setBrightness(screen, b);
        for (int f = 0; f < frames; f++) {
            swiWaitForVBlank();
        }
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

    NF_LoadSpriteGfx("bg/uiAssets",6,16,16);
    NF_LoadSpritePal("bg/uiAssets",6);
    NF_VramSpriteGfx(1,6,6,false);
    NF_VramSpritePal(1,6,6);

    NF_CreateSprite(1,37,6,6,10,173);
    NF_CreateSprite(1,31,6,6,30,173);
    NF_CreateSprite(1,32,6,6,50,173);
    NF_CreateSprite(1,33,6,6,70,173);

    NF_SpriteFrame(1,31,1);
    NF_SpriteFrame(1,32,2);
    NF_SpriteFrame(1,33,3);



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
    while (1)
    {

        NF_ClearTextLayer(0, 0);
        scanKeys();
         ctx.input->buttonsDown = keysDown();
         ctx.input->buttonsHeld = keysHeld();
         ctx.input->buttonsUp = keysUp();


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

                    fadeOut(3,4);
                    showEditor(false);
                    ctx.player->playerX = ctx.editor->flagPosX*16;
                    ctx.player->playerY = ctx.editor->flagPosY*16;
                    ctx.coords->cameraX = ctx.player->playerX-128;
                    ctx.coords->cameraY = ctx.player->playerY-96;
                    ctx.coords->cameraTileX = coordsToTile(ctx.coords->cameraX);
                    ctx.coords->cameraTileY = coordsToTile(ctx.coords->cameraY);
                    ctx.coords->scrollX = 128;
                    ctx.coords->scrollY = 160;

                    updateOrigin(&ctx);
                    updateTiles(&ctx);
                    //swap with seperate HUD <-- todo
                    NF_ShowSprite(1,1,false);
                    NF_ShowSprite(1,2,false);
                    NF_ShowSprite(1,3,false);
                    NF_ShowSprite(1,4,false);
                    NF_DeleteTiledBg(1,2);
                    lcdSwap();
                    for (int i = 0; i < 10; i++)
                    {
                        NF_ShowSprite(1,i + 6,false);
                    }
                    scrollLogic(&ctx);
                    updateTiles(&ctx);

                    NF_ShowSprite(1,5,true);
                    if(ctx.player->hflip == false){
                        NF_MoveSprite(1, 5, 128, 96 - 16);
                    }else{
                        NF_MoveSprite(1, 5, 128 - 16, 96 - 16);
                    }

                    NF_ClearTextLayer(0, 0);
                    NF_SpriteOamSet(0);
                    NF_SpriteOamSet(1);
                    swiWaitForVBlank();
                    NF_UpdateVramMap(1, 3);
                    NF_ScrollBg(1, 3,coords.scrollX,coords.scrollY);
                    NF_ScrollBg(1,2,coords.cameraX % 16,coords.cameraY % 16);
                    NF_UpdateTextLayers();
                    oamUpdate(&oamMain);
                    oamUpdate(&oamSub);

                    playerAnim(&ctx);
                    playerPhysics(&ctx);
                    playerScroll(&ctx);

                    scrollLogic(&ctx);
                    //updateTiles(&ctx);
                    //updateOrigin(&ctx);


                    fadeIn(3,2);
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
                    fadeOut(3,4);
                    NF_CreateTiledBg(1,2,"grid");
                    showEditor(true);
                    lcdSwap();
                    ctx.coords->cameraX = ctx.editor->flagPosX - 64;//mysterious offset todo figure out
                    ctx.coords->cameraY = ctx.editor->flagPosY * 16 - 64;
                    ctx.coords->scrollX = 128;
                    ctx.coords->scrollY = 160;
                    updateTiles(&ctx);
                    updateOrigin(&ctx);
                    for (int i = 0; i < 10; i++)
                    {
                        NF_ShowSprite(1,i + 6,true);
                    }
                    scrollLogic(&ctx);
                    updateTiles(&ctx);
                    updateOrigin(&ctx);
                    state = EDITOR;
                    debugText(&ctx);
                    NF_ShowSprite(1,5,false);
                    NF_ShowSprite(1,0,true);
                    editorFrame(&ctx);
                    NF_MoveSprite(1,0,(ctx.editor->currentBlock * 20) + 30,3);
                    NF_SpriteOamSet(0);
                    NF_SpriteOamSet(1);
                    swiWaitForVBlank();
                    NF_UpdateVramMap(1, 3);
                    NF_ScrollBg(1, 3,coords.scrollX,coords.scrollY);
                    NF_ScrollBg(1,2,coords.cameraX % 16,coords.cameraY % 16);
                    NF_UpdateTextLayers();
                    oamUpdate(&oamMain);
                    oamUpdate(&oamSub);
                    fadeIn(3,2);
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
                NF_WriteText(0,0,0,0,"Preview");
                static bool gridHidden = false;
                static int row = 0;
                static int R = 9;
                static int G = 19;
                static int B = 28;

                if (ctx.input->buttonsDown & KEY_TOUCH || ctx.input->buttonsHeld & KEY_TOUCH)
                {
                    touchRead(&ctx.input->touchPos);

                }
                if (ctx.input->buttonsUp & KEY_TOUCH && ctx.input->touchPos.px > 16 && ctx.input->touchPos.px < 79)
                {
                    if (ctx.input->touchPos.py > 48 && ctx.input->touchPos.py < 63)
                    {
                        row = 0;
                        R++;
                        R = clampInt(R,0, 31);
                    }
                    else if (ctx.input->touchPos.py > 80 && ctx.input->touchPos.py < 95)
                    {

                        row = 1;
                        G++;
                        G = clampInt(G,0, 31);
                    }
                    else if (ctx.input->touchPos.py > 112 && ctx.input->touchPos.py < 127)
                    {
                        row = 2;
                        B++;
                        B = clampInt(B, 0, 31);
                    }

                }
                if (ctx.input->buttonsUp & KEY_TOUCH && ctx.input->touchPos.px > 176 && ctx.input->touchPos.px < 239)
                {
                    //row++;
                    //row = clampInt(row,0, 2);
                    if (ctx.input->touchPos.py > 48 && ctx.input->touchPos.py < 63)
                    {
                        row = 0;
                        R--;
                        R = clampInt(R,0, 31);
                    }
                    else if (ctx.input->touchPos.py > 80 && ctx.input->touchPos.py < 95)
                    {

                        row = 1;
                        G--;
                        G = clampInt(G,0, 31);
                    }
                    else if (ctx.input->touchPos.py > 112 && ctx.input->touchPos.py < 127)
                    {
                        row = 2;
                        B--;
                        B = clampInt(B, 0, 31);
                    }

                }

                if (ctx.input->touchPos.py >=  176)
                {
                    if (ctx.input->touchPos.px >= 0 && ctx.input->touchPos.px <= 63 )
                    {
                        R = 9;
                        G = 19;
                        B = 28;
                    }

                    if (ctx.input->touchPos.px >= 192 && ctx.input->touchPos.px <= 255 )
                    {

                        showEditor(true);
                        showChangePalWindow(false);
                        state = EDITOR;
                        gridHidden = false;

                    }
                }
                char change[16] = "CHANGE BG COLOUR";
                for (int i = 0; i < 16; i++)
                {
                    NF_SpriteFrame(1,45 + i,getLetterFrame(&change[i]));
                    //NF_MoveSprite(1,45 + i,8 * i, 0);
                }


                //NF_WriteText()


                BG_PALETTE_SUB[0] = RGB15(R, G, B);
                BG_PALETTE[0] = RGB15(R, G, B);

                char Rc[2] = "R";
                NF_SpriteFrame(1, 18, getLetterFrame(&Rc[0]));

                char rBuffer[4] = {0};
                snprintf(rBuffer, sizeof(rBuffer), "%d", R);
                if (R >= 10)
                {
                    NF_SpriteFrame(1, 19  + 1, getLetterFrame(&rBuffer[0]));
                    NF_SpriteFrame(1, 20 + 1, getLetterFrame(&rBuffer[1]));
                }
                else
                {
                    char ZeroC[2] = "0";

                    NF_SpriteFrame(1, 19  + 1, getLetterFrame(&ZeroC[0]));
                    NF_SpriteFrame(1, 20  + 1, getLetterFrame(&rBuffer[0]));
                }


                char Gc[2] = "G";
                NF_SpriteFrame(1, 18 + 4, getLetterFrame(&Gc[0]));

                char gBuffer[4] = {0};
                snprintf(gBuffer, sizeof(gBuffer), "%d", G);
                if (G >= 10)
                {
                    NF_SpriteFrame(1, 19 + 4  + 1, getLetterFrame(&gBuffer[0]));
                    NF_SpriteFrame(1, 20 + 4  + 1, getLetterFrame(&gBuffer[1]));
                }
                else
                {
                    char ZeroC[2] = "0";

                    NF_SpriteFrame(1, 19 + 4  + 1, getLetterFrame(&ZeroC[0]));
                    NF_SpriteFrame(1, 20 + 4  + 1, getLetterFrame(&gBuffer[0]));
                }


                char Bc[2] = "B";
                NF_SpriteFrame(1, 18 + 8, getLetterFrame(&Bc[0]));

                char bBuffer[4] = {0};
                snprintf(bBuffer, sizeof(bBuffer), "%d", B);
                if (B >= 10)
                {
                    NF_SpriteFrame(1, 19 + 8 + 1, getLetterFrame(&bBuffer[0]));
                    NF_SpriteFrame(1, 20 + 8 + 1, getLetterFrame(&bBuffer[1]));
                }
                else
                {
                    char ZeroC[2] = "0";

                    NF_SpriteFrame(1, 19 + 8 + 1, getLetterFrame(&ZeroC[0]));
                    NF_SpriteFrame(1, 20 + 8 + 1, getLetterFrame(&bBuffer[0]));
                }



                char space[2] = " ";
                int frameSpace = getLetterFrame(&space[0]);
                NF_SpriteFrame(1,19,frameSpace);
                NF_SpriteFrame(1,23,frameSpace);
                NF_SpriteFrame(1,27,frameSpace);
                    if (!gridHidden && state != EDITOR)
                    {
                        showEditor(false);
                        showChangePalWindow(true);
                        gridHidden = true;
                    }


                break;
            }

        case SHARE_LEVEL_HOST:
            hostMode();
            break;

        case SHARE_LEVEL_CLIENT:
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
