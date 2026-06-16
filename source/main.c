
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
    u16 buttonsUp;
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
    bool rectFillOn;
}editorContext;

typedef struct{
    int playerX;
    int playerY;
    int velocityX;
    int velocityY;
    bool grounded;
    
}playerContext;

typedef struct{
    int topLeftX;
    int topLeftY;
    int width;
    int height;
}rectangle;
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
        TILE_MAP[ctx->flagPosY][ctx->flagPosX] = 0;
        ctx->flagPosX = tileX;
        ctx->flagPosY = tileY;
    }
    TILE_MAP[tileY][tileX] = currentBlock;
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

int firstTouchX = -1; //todo add in editorcontext once done
int firstTouchY = -1;
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

    if(input->buttonsHeld & KEY_TOUCH && !ctxE->rectFillOn){
        touchRead(&input->touchPos);
        input->touchTileX = coordsToTile(input->touchPos.px + coords->cameraX );
        input->touchTileY = coordsToTile (input->touchPos.py + coords->cameraY );
        
        if(input->touchPos.py >= 22){//hud width
            addTile(input->touchTileX,input->touchTileY,ctxE->currentBlock,ctxE);
        }
        updateTiles(coords); 
    }
    if(input->buttonsDown & KEY_B){
        ctxE->rectFillOn = true;
    }

    if (input->buttonsDown & KEY_TOUCH && !ctxE->rectFillOn){
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
    else if(ctxE->rectFillOn && (firstTouchX < 0 || firstTouchY < 0) && input->buttonsHeld & KEY_TOUCH){
        touchRead(&input->touchPos);
        firstTouchX = coordsToTile(input->touchPos.px + coords->cameraX );
        firstTouchY = coordsToTile (input->touchPos.py + coords->cameraY );
    }
    else if(ctxE->rectFillOn && (firstTouchX > 0 || firstTouchY > 0) && input->buttonsHeld & KEY_TOUCH){
        touchRead(&input->touchPos);
        input->touchTileX = coordsToTile(input->touchPos.px + coords->cameraX );
        input->touchTileY = coordsToTile (input->touchPos.py + coords->cameraY );
        updateTiles(coords);
        int highestX = (input->touchTileX >firstTouchX) ? input->touchTileX : firstTouchX;
        int lowestX  = (input->touchTileX < firstTouchX) ? input->touchTileX : firstTouchX;
        int highestY = (input->touchTileY> firstTouchY) ? input->touchTileY : firstTouchY;
        int lowestY  = (input->touchTileY<firstTouchY) ? input->touchTileY : firstTouchY;
        for(int i = lowestY; i <= highestY; i++){
            for(int j = lowestX; j <= highestX; j++){
                int screenX = (j - coords->originTileX) * 2;
                int screenY = (i - coords->originTileY) * 2;
                if(screenX < 0 || screenX >= 64 || screenY < 0 || screenY >= 64) continue;
                NF_SetTileOfMap(1, TILE_LAYER, screenX,screenY,  blocks[ctxE->currentBlock].topLeftTile+64); //probably should define as an offset
                NF_SetTileOfMap(1, TILE_LAYER, screenX+1,screenY,blocks[ctxE->currentBlock].topRightTile +64);
                NF_SetTileOfMap(1, TILE_LAYER, screenX,screenY+1,blocks[ctxE->currentBlock].bottomLeftTile+64);
                NF_SetTileOfMap(1, TILE_LAYER, screenX+1, screenY+1,blocks[ctxE->currentBlock].bottomRightTile+64);
            }
        }
        

    }
    else if(ctxE->rectFillOn && (firstTouchX > 0 || firstTouchY > 0) && input->buttonsUp & KEY_TOUCH){
        ctxE->rectFillOn = false;
        int highestX = (input->touchTileX >firstTouchX) ? input->touchTileX : firstTouchX;
        int lowestX  = (input->touchTileX < firstTouchX) ? input->touchTileX : firstTouchX;
        int highestY = (input->touchTileY> firstTouchY) ? input->touchTileY : firstTouchY;
        int lowestY  = (input->touchTileY<firstTouchY) ? input->touchTileY : firstTouchY;
        for(int i = lowestY; i < highestY + 1; i++){
            for(int j = lowestX; j < highestX + 1; j++){
                TILE_MAP[i][j] = ctxE->currentBlock;
            }
        }
        updateTiles(coords);
        firstTouchX = -1;
        firstTouchY = -1;
    }

    if(input->buttonsDown & KEY_A){
        state = PLAY_SCREEN;
        
        //unload grid, swap with seperate HUD <-- todo
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
    coords->originTileX = coords->cameraTileX - 8;
    coords->originTileY = coords->cameraTileY - 10;
}

void debugText(worldCoordinates *coords, inputs *input,editorContext *ctx, playerContext *pctx){
    NF_ClearTextLayer(0, 0); 
    char buffer[64];
    
    snprintf(buffer,sizeof(buffer),"scrollX px %d scrollY px %d",coords->scrollX,coords->scrollY);
    NF_WriteText(0,0,1,1,buffer);

    char camCord[64];
    snprintf(camCord,sizeof(camCord),"camX %d camY %d",coords->cameraX,coords->cameraY);
    NF_WriteText(0,0,1,3,camCord);

    char tilecoords[64];
    snprintf(tilecoords,sizeof(tilecoords),"tileX %d tileY %d",input->touchTileX,input->touchTileY);
    NF_WriteText(0,0,1,4,tilecoords);

    char ocord[64];
    snprintf(ocord,sizeof(ocord),"originX %d originY %d",coords->originTileX,coords->originTileY);
    NF_WriteText(0,0,1,5,ocord);


    char fcord[64];
    snprintf(fcord,sizeof(fcord),"flagX %d flagY %d",ctx->flagPosX,ctx->flagPosY);
    NF_WriteText(0,0,1,7,fcord);

    char pCords[64];
    snprintf(pCords,sizeof(pCords),"playerX %d playerY %d",pctx->playerX,pctx->playerY);
    NF_WriteText(0,0,1,6,pCords);

    char firstTouchCords[64];
    snprintf(firstTouchCords,sizeof(firstTouchCords),"firstT %d firstT %d",firstTouchX,firstTouchY);
    NF_WriteText(0,0,1,8,firstTouchCords);

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
bool checkCollision(rectangle rectA, rectangle rectB){
    return !(rectA.topLeftX > rectB.topLeftX + rectB.width ||
             rectA.topLeftX + rectA.width < rectB.topLeftX ||
             rectA.topLeftY + rectA.height < rectB.topLeftY ||
             rectA.topLeftY > rectB.topLeftY + rectB.height); // returns true if overlapping
}

#define JUMP_FORCE 16
#define GRAVITY 2
void playerPhysics(playerContext *ctx, inputs *input, worldCoordinates *coords){
    ctx->velocityX = 0;
    if(input->buttonsHeld & KEY_LEFT)  ctx->velocityX = -SPEED;
    if(input->buttonsHeld & KEY_RIGHT) ctx->velocityX =  SPEED;

    if(input->buttonsDown & KEY_B && ctx->grounded){
        ctx->velocityY = -JUMP_FORCE;
        ctx->grounded = false;
    } else if(!tileSolid(ctx->playerX/16, (ctx->playerY/16)+1)){
        ctx->velocityY += GRAVITY;
        ctx->grounded = false;
    } else {
        ctx->velocityY = 0;
        ctx->grounded = true;
    }

    ctx->playerX += ctx->velocityX;
    ctx->playerY += ctx->velocityY;
}

void playerScroll(worldCoordinates *coords, playerContext *ctxP){
    coords->cameraX = ctxP->playerX - 128;
    coords->cameraY = ctxP->playerY - 96;
    coords->cameraTileX = coordsToTile(coords->cameraX);
    coords->cameraTileY = coordsToTile(coords->cameraY);
    coords->scrollX = 128 + (coords->cameraX % 16);
    coords->scrollY = 160 + (coords->cameraY % 16);
    updateOrigin(coords);
    updateTiles(coords);
}
int main(int argc, char **argv)
{

    srand(time(NULL));
    initialise();

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
    NF_VramSpriteGfx(1,1,1,false);
    NF_VramSpritePal(1,1,1);
    NF_CreateSprite(1,0,1,1,29,3);

    initBlocks();  
    editorContext ctxE = {};
    playerContext ctxP = {};
    ctxP.grounded = true;
    ctxE.currentBlock = 1;
    ctxE.rectFillOn = false;
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
            case EDITOR:
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
                playerPhysics(&ctxP, &input, &coords);
                playerScroll(&coords, &ctxP);
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