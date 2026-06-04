
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <nds.h>
#include <filesystem.h>

#include <nf_lib.h>
// ON DS  ONE TILE IS 8X8!!!!!! 
//bg size is 512 hence 512/8 = 64
#define SPEED 2         // Scroll speed
#define MAP_X 32       // Map width (animated blocks are 2x2, 128 / 2 = 64)
#define MAP_Y 32        // Map height (animated blocks are 2x2, 64 / 2 = 32)
#define GRID_X 1024
#define GRID_Y 1024 //16kx16k may be a bit overkill
#define ROW_LENGTH 16
#define TOTAL_BLOCKS 5
#define TILE_LAYER 3

typedef enum{
    EDITOR,
    PLAY_SCREN
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

inputs input = {};

void initBlocks(){
    blocks[0].solid = false;
    blocks[0].ifTouched = NULL;
    blocks[0].topRightTile = 0;
    blocks[0].topLeftTile = 0;
    blocks[0].bottomLeftTile = 0;
    blocks[0].bottomRightTile = 0;

    blocks[1].solid = true;
    blocks[1].ifTouched = NULL;
    blocks[1].topRightTile = 1;
    blocks[1].topLeftTile = 1;
    blocks[1].bottomLeftTile = 2;
    blocks[1].bottomRightTile = 2;

    blocks[2].solid = true;
    blocks[2].ifTouched = NULL;
    blocks[2].topRightTile = 2;
    blocks[2].topLeftTile = 2;
    blocks[2].bottomLeftTile = 2;
    blocks[2].bottomRightTile = 2;

    blocks[3].solid = true;
    blocks[3].ifTouched = NULL;
    blocks[3].topRightTile = 3;
    blocks[3].topLeftTile = 3;
    blocks[3].bottomLeftTile =3;
    blocks[3].bottomRightTile = 3;

    blocks[4].solid = true; //debug block to change later
    blocks[4].ifTouched = NULL;
    blocks[4].topRightTile = 6;
    blocks[4].topLeftTile = 5;
    blocks[4].bottomLeftTile = 7;
    blocks[4].bottomRightTile = 8;

}


u8 TILE_MAP[GRID_Y][GRID_X]; // 512 pixels, 32 tiles, every frame is 2x2 (16/8) tiles

inline int coordsToTile(int coord){
    return coord / 16;  
}


void addTile(int tileX, int tileY){
    if(tileX < 0 || tileX >= GRID_X || tileY < 0 || tileY >= GRID_Y) return;
    if(TILE_MAP[tileY + 10][tileX + 8] >= TOTAL_BLOCKS - 1)
        TILE_MAP[tileY + 10][tileX + 8] = 0;
    else
        TILE_MAP[tileY + 10][tileX + 8]++;
}

void updateTiles(int originTileX, int originTileY)
{
    for(int y = originTileY; y < originTileY + MAP_Y; y++){
        for(int x = originTileX; x < originTileX + MAP_X; x++){
            int coordX = x - originTileX;
            int coordY = y - originTileY;
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
    NF_UpdateVramMap(1, 3);
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

    NF_InitTextSys(0);
    NF_LoadTextFont("fnt/default", "normal", 256, 256, 0);
    NF_CreateTextLayer(0, 0, 0, "normal");
    BG_PALETTE_SUB[0] = RGB15(31, 31, 31); 
}

void doInputs(int *cameraX, int *cameraY,int *scrollX, int* scrollY,int* originTileX, int* originTileY,inputs *input){
    if (input->buttonsHeld & KEY_LEFT){ 
        *scrollX -= SPEED; 
        *cameraX -= SPEED; 
    }
    if (input->buttonsHeld & KEY_RIGHT){ 
        *scrollX += SPEED; 
        *cameraX += SPEED; 
    }  
    if (input->buttonsHeld & KEY_UP){
        *scrollY -= SPEED; 
        *cameraY -= SPEED; 
    }
    if (input->buttonsHeld & KEY_DOWN){
        *scrollY += SPEED;
        *cameraY += SPEED;  
    }
    
    if (input->buttonsDown & KEY_TOUCH){
        touchRead(&input->touchPos);

        input->touchTileX = coordsToTile(input->touchPos.px + *cameraX );
        input->touchTileY = coordsToTile (input->touchPos.py + *cameraY );

        addTile(input->touchTileX,input->touchTileY); 
        updateTiles(*originTileX, *originTileY); 
    }
    int valX = input->touchPos.px + *cameraX ;
    int valY = input->touchPos.py + *cameraY;
    char buffer[64];
        snprintf(buffer,sizeof(buffer),"t+c X %d, Y %d",valX,valY );
        NF_WriteText(0,0,1,6,buffer);
        
}
void updateOrigin(int *originTileX, int *originTileY, int *cameraTileX,int *cameraTileY){
    *originTileX = *cameraTileX;
    *originTileY = *cameraTileY;
}
int main(int argc, char **argv)
{

    srand(time(NULL));
    initialise();

    
    NF_LoadTilesForBg("bg/tiles", "tiles", 512, 512, 0, TOTAL_BLOCKS + 10);
    NF_CreateTiledBg(1, TILE_LAYER, "tiles");
    //touchPosition touch_pos;
    initBlocks();

    int touchTileX = 0;
    int touchTileY = 0;
    int scrollX = 128;//should scroll values be 0? they desync addtile
    int scrollY = 160;
    int cameraX = ((GRID_X* 16)-256) / 2;   
    int cameraY = ((GRID_Y * 16) - 192) / 2;     
    int cameraTileX = cameraX / 16;
    int cameraTileY = cameraY / 16;

    int originTileX = cameraTileX;
    int originTileY = cameraTileY;

    while (1)
    {
        scanKeys(); 
        input.buttonsDown = keysDown();
        input.buttonsHeld = keysHeld();
        

        switch(state){
            case EDITOR:
                NF_ClearTextLayer(0, 0); 
                doInputs(&cameraX,&cameraY,&scrollX,&scrollY,&originTileX,&originTileY,&input);
                char buffer[64];
                
                snprintf(buffer,sizeof(buffer),"X px %d Y px %d",scrollX,scrollY);
                NF_WriteText(0,0,1,1,buffer);

                char camCord[64];
                snprintf(camCord,sizeof(camCord),"camX %d camY %d",cameraX,cameraY);
                NF_WriteText(0,0,1,3,camCord);

                char tilecoords[64];
                snprintf(tilecoords,sizeof(tilecoords),"tileX %d tileY %d",input.touchTileX,input.touchTileY);
                NF_WriteText(0,0,1,4,tilecoords);

                char ocord[64];
                snprintf(ocord,sizeof(ocord),"oX %d oY %d",originTileX,originTileY);
                NF_WriteText(0,0,1,5,ocord);
                int jitteroffsetX = cameraX % 16;
                int jitterOffsetY = cameraY % 16;
                cameraTileX = cameraX / 16;
                cameraTileY = cameraY / 16;
                if(scrollX < 16 || scrollX > 240 /*512-16-256*/ || scrollY < 16 || scrollY > 304 /*512 - 192 - 16*/){
                    scrollX = 128 + jitteroffsetX ;
                    scrollY = 160 + jitterOffsetY;
                    updateOrigin(&originTileX,&originTileY,&cameraTileX,&cameraTileY);
                    updateTiles(originTileX, originTileY); 

                }
                break;


        }
         

        

        NF_ScrollBg(1, 3, scrollX, scrollY);
     
            swiWaitForVBlank();

        NF_UpdateTextLayers();  
    }
    return 0;
    
}