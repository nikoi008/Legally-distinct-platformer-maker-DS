
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <nds.h>
#include <filesystem.h>

#include <nf_lib.h>
// ON DS  ONE TILE IS 8X8!!!!!! 
//bg size is 512 hence 512/8 = 64
#define SPEED 4        // Scroll speed
#define MAP_X 32       // Map width (animated blocks are 2x2, 128 / 2 = 64)
#define MAP_Y 32        // Map height (animated blocks are 2x2, 64 / 2 = 32)
#define GRID_X 1024
#define GRID_Y 1024 //16kx16k may be a bit overkill
#define ROW_LENGTH 16
#define TOTAL_BLOCKS 4
#define TILE_LAYER 3
#define BLACK 0 
typedef enum{
    EDITOR,
    PLAY_SCREEN
} gameStates;
gameStates state = EDITOR;

typedef struct{
    bool solid;
    void (*ifTouched)(void);
    int topLeftTile;
    int topRightTile;
    int bottomLeftTile;
    int bottomRightTile;
}block;
block blocks[TOTAL_BLOCKS] = {};

typedef struct{
    u16 buttonsDown;
    u16 buttonsHeld;
    touchPosition touchPos;
    int touchTileX;
    int touchTileY;
}inputs;

typedef struct{
    int originTileX;
    int originTileY;
    int scrollX;
    int scrollY;
    int cameraX;
    int cameraY;
    int cameraTileX;
    int cameraTileY;
}worldCoordinates;

typedef struct{
    int currentBlock;
    int flagPosX;
    int flagPosY;
}editorContext;

typedef struct{
    int playerX;
    int playerY;
}playerContext;

void initBlocks(){
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

    blocks[3].solid = true;//spike
    blocks[3].ifTouched = NULL;
    blocks[3].topLeftTile = 9;
    blocks[3].topRightTile = 10;
    blocks[3].bottomLeftTile = 11;
    blocks[3].bottomRightTile = 12;
}


u8 TILE_MAP[GRID_Y][GRID_X]; // 512 pixels, 32 tiles, every frame is 2x2 (16/8) tiles

inline int coordsToTile(int coord){
    return coord / 16;  
}


void addTile(int tileX, int tileY, int currentBlock, editorContext *ctx){
    if(tileX < 0 || tileX >= GRID_X || tileY < 0 || tileY >= GRID_Y) return;
    if( (ctx->flagPosX >= 0 || ctx->flagPosY >= 0) && ctx->currentBlock == 2){
        TILE_MAP[ctx->flagPosY + 10][ctx->flagPosX + 8] = 0;
        ctx->flagPosX = tileX;
        ctx->flagPosY = tileY;
    }
    TILE_MAP[tileY + 10][tileX + 8] = currentBlock;
}

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
void initialise(){
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);
    consoleDemoInit();
    swiWaitForVBlank();

    nitroFSInit(NULL);
    NF_SetRootFolder("NITROFS");
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);


    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(0);       
    NF_InitTiledBgSys(1);   

    NF_InitSpriteBuffers();  
    NF_InitSpriteSys(1);

    NF_InitTextSys(0);
    NF_LoadTextFont("fnt/default", "normal", 256, 256, 0);
    NF_CreateTextLayer(0, 0, 0, "normal");
    NF_DefineTextColor(0,0,BLACK,0,0,0);
    BG_PALETTE_SUB[0] = RGB15(9, 19, 28); 
    BG_PALETTE[0] = RGB15(9, 19, 28);

}
inline bool tileSolid(int tileX,int tileY){
    return blocks[TILE_MAP[tileY][tileX]].solid;
}
void doInputsEditor(worldCoordinates *coords,inputs *input,editorContext *ctxE){
    if (input->buttonsHeld & KEY_LEFT){ 
        coords->scrollX -= SPEED; 
        coords->cameraX -=SPEED; 
    }
    if (input->buttonsHeld & KEY_RIGHT){ 
        coords->scrollX += SPEED; 
        coords->cameraX += SPEED; 
    }  
    if (input->buttonsHeld & KEY_UP){
        coords->scrollY -= SPEED; 
        coords->cameraY -= SPEED; 
    }
    if (input->buttonsHeld & KEY_DOWN){
        coords->scrollY += SPEED;
        coords->cameraY += SPEED;  
    }

    if(input->buttonsHeld & KEY_TOUCH){
        touchRead(&input->touchPos);
        input->touchTileX = coordsToTile(input->touchPos.px + coords->cameraX );
        input->touchTileY = coordsToTile (input->touchPos.py + coords->cameraY );
        
        if(input->touchPos.py >= 22){//hud width
            addTile(input->touchTileX,input->touchTileY,ctxE->currentBlock,ctxE);
        }
        updateTiles(coords); 
    }

    if (input->buttonsDown & KEY_TOUCH){
        touchRead(&input->touchPos);

        input->touchTileX = coordsToTile(input->touchPos.px + coords->cameraX );
        input->touchTileY = coordsToTile (input->touchPos.py + coords->cameraY );
        
        if(input->touchPos.py <= 22){//hud width
            if(input->touchPos.px /16  <= 1 || input->touchPos.px /16 >= 14){
                ctxE->currentBlock = (ctxE->currentBlock + 1)% TOTAL_BLOCKS;
                
            }
        }
        updateTiles(coords); 
    }

    if(input->buttonsDown & KEY_A){
        state = PLAY_SCREEN;
        
        //unload grid, swap with seperate HUD
        //NF_ClearTextLayer(1, 0); 
        NF_ShowSprite(1,1,false);
        NF_ShowSprite(1,2,false);
        NF_ShowSprite(1,3,false);
        NF_ShowSprite(1,4,false);
        NF_DeleteTiledBg(1,2);
        lcdSwap();
    }
}
void updateOrigin(worldCoordinates *coords){
    coords->originTileX = coords->cameraTileX;
    coords->originTileY = coords->cameraTileY;
}

void debugText(worldCoordinates *coords, inputs *input,editorContext *ctx){
    NF_ClearTextLayer(0, 0); 
    char buffer[64];
    
    snprintf(buffer,sizeof(buffer),"X px %d Y px %d",coords->scrollX,coords->scrollY);
    NF_WriteText(0,0,1,1,buffer);

    char camCord[64];
    snprintf(camCord,sizeof(camCord),"camX %d camY %d",coords->cameraX,coords->cameraY);
    NF_WriteText(0,0,1,3,camCord);

    char tilecoords[64];
    snprintf(tilecoords,sizeof(tilecoords),"tileX %d tileY %d",input->touchTileX,input->touchTileY);
    NF_WriteText(0,0,1,4,tilecoords);

    char ocord[64];
    snprintf(ocord,sizeof(ocord),"oX %d oY %d",coords->originTileX,coords->originTileY);
    NF_WriteText(0,0,1,5,ocord);


    char fcord[64];
    snprintf(fcord,sizeof(fcord),"fX %d fY %d",ctx->flagPosX,ctx->flagPosY);
    NF_WriteText(0,0,1,6,fcord);
}

void scrollLogic(worldCoordinates *coords){
    int jitteroffsetX = coords->cameraX % 16;
    int jitterOffsetY = coords->cameraY % 16;
    coords->cameraTileX = coordsToTile(coords->cameraX);
    coords->cameraTileY = coordsToTile(coords->cameraY);
    int sX = coords->scrollX;
    int sY = coords->scrollY;
    if(sX < 16 || sX > 240 /*512-16-256*/ || sY < 16 || sY > 304 /*512 - 192 - 16*/){
        coords->scrollX = 128 + jitteroffsetX;
        coords->scrollY = 160 + jitterOffsetY;
        updateOrigin(coords);
        updateTiles(coords); 

    }
}

void playerGravity(playerContext *ctx){
    if(tileSolid((ctx->playerX/16) + 8,((ctx->playerY / 16) + 10)+1)){
        
        ctx->playerY -=1;
        return;
    }
    else{
        ctx->playerY += 1;
    }
}
void doInputsPlayer(inputs *input,playerContext *ctxP){
    if(input->buttonsHeld & KEY_LEFT){ctxP->playerX -= 2;}
    if(input->buttonsHeld & KEY_RIGHT){ctxP->playerX += 2;}
    if(input->buttonsDown & KEY_B){ctxP->playerY -= 20;}
}
int main(int argc, char **argv)
{

    srand(time(NULL));
    initialise();

    NF_LoadTilesForBg("bg/tiles", "tiles", 512, 512, 0, TOTAL_BLOCKS + 20);//todo dont just add +10, m
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
    NF_VramSpriteGfx(1,1,1,false);
    NF_VramSpritePal(1,1,1);
    NF_CreateSprite(1,0,1,1,29,3);


    //NF_LoadSpriteGfx("bg/sprite",2,32,32);
    //NF_LoadSpritePal("bg/sprite",2);
    //NF_VramSpriteGfx(1,2,2,false);
    //NF_VramSpritePal(1,2,2);

    initBlocks();  
    editorContext ctxE = {};
    playerContext ctxP = {};
    ctxE.currentBlock = 1;
    worldCoordinates coords = {};
    coords.cameraY = ( GRID_Y * 16 )/2;
    //coords.scrollX = 128;  
    //coords.scrollY = 160; DONT DO THIS
   /*/ int touchTileX = 0;
    int touchTileY = 0;
    int scrollX = 128;//should scroll values be 0? they desync addtile
    int scrollY = 160;
    int cameraX = ((GRID_X* 16)-256) / 2;   
    int cameraY = ((GRID_Y * 16) - 192) / 2;     
    int cameraTileX = cameraX / 16;
    int cameraTileY = cameraY / 16;

    int originTileX = cameraTileX;
    int originTileY = cameraTileY;*/ //keepign for future reference in case anything breaks later
    inputs input = {};
    int gridX = 0;
    int gridY = 0;
    while (1)
    {

        scanKeys(); 
        input.buttonsDown = keysDown();
        input.buttonsHeld = keysHeld();
        

        switch(state){
            case EDITOR:
                doInputsEditor(&coords,&input,&ctxE);
                if(!PLAY_SCREEN){debugText(&coords, &input,&ctxE);}
                else{
                    ctxP.playerX = ctxE.flagPosX*16;
                    ctxP.playerY = ctxE.flagPosY*16;
                    //todo make sure that player camera is also centered to flagPos
                }
                scrollLogic(&coords); 
                NF_MoveSprite(1,0,(ctxE.currentBlock * 18) + 29,3);
                gridX = coords.cameraX % 16;
                gridY = coords.cameraY % 16;

                break;

            
            case PLAY_SCREEN:
                doInputsPlayer(&input,&ctxP);
                NF_MoveSprite(1,0,ctxP.playerX,ctxP.playerY);
                playerGravity(&ctxP);
            
                


        }
    
        NF_SpriteOamSet(0);
        NF_SpriteOamSet(1);
        swiWaitForVBlank();
        NF_UpdateVramMap(1, 3);
        NF_ScrollBg(1, 3,coords.scrollX,coords.scrollY);
        NF_ScrollBg(1,2,gridX,gridY);
        NF_UpdateTextLayers();
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }
    return 0;
    
}