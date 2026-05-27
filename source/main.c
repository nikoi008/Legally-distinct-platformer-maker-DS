
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
#define GRID_X 64
#define GRID_Y 64
#define ROW_LENGTH 16
#define TOTAL_BLOCKS 4
#define TILE_LAYER 3

typedef enum{
    EDITOR,
    PLAY_SCREN
} gameStates;
gameStates state = EDITOR;

typedef struct{
    //char* name;
    //int blockID; necessary?
    bool solid;
    void (*ifTouched)(void);
    int topLeftTile;
    int topRightTile;
    int bottomLeftTile;
    int bottomRightTile;
}block;

block blocks[TOTAL_BLOCKS] = {};


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

}

int WATER_SPEED;
int TILE_MAP[GRID_Y][GRID_X]; // 512 pixels, 32 tiles, every frame is 2x2 (16/8) tiles

inline int coordsToTile(int coord){
    return coord / 16;
}


void addTile(int tileX, int tileY){
    //if(tileX > MAP_X || tileY > MAP_Y || tileX < 0 || tileY < 0){
    //    return;
    //} probably uselesss for now
    if(TILE_MAP[tileY][tileX] >= TOTAL_BLOCKS - 1){
        TILE_MAP[tileY][tileX] = 0;
    }
    else{
       TILE_MAP[tileY][tileX]++; 
    }

}
void UpdateTiles()
{
    for(int y = 0; y < MAP_Y; y++){      
        for(int x = 0; x < MAP_X; x++){
            NF_SetTileOfMap(1, TILE_LAYER, x*2,y*2,blocks[TILE_MAP[y][x]].topLeftTile); //tl
            NF_SetTileOfMap(1, TILE_LAYER, x*2+1,y*2,blocks[TILE_MAP[y][x]].topRightTile);  //tr
            NF_SetTileOfMap(1, TILE_LAYER, x*2, y*2+1,blocks[TILE_MAP[y][x]].bottomLeftTile); //bl
            NF_SetTileOfMap(1, TILE_LAYER, x*2+1,y*2+1,blocks[TILE_MAP[y][x]].bottomRightTile); //br
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
}


int main(int argc, char **argv)
{

    srand(time(NULL));
    initialise();

    
    NF_LoadTilesForBg("bg/tiles", "tiles", 512, 512, 0, TOTAL_BLOCKS);

    //NF_CreateTiledBg(0, TILE_LAYER, "tiles");
    NF_CreateTiledBg(1, TILE_LAYER, "tiles");
    //InitAnimatedBg();
    touchPosition touch_pos;
    initBlocks();

    int touchTileX = 0;
    int touchTileY = 0;
    int scrollX = 128;
    int scrollY = 160;
    int minimumX = 0;
    

    while (1)
    {
        scanKeys(); 
        u16 buttonsDown = keysDown();
        u16 buttonsHeld = keysHeld();
        

        switch(state){
            case EDITOR:
                if (buttonsDown & KEY_TOUCH){
                    touchRead(&touch_pos);

                    touchTileX = coordsToTile(touch_pos.px + scrollX);
                    touchTileY = coordsToTile(touch_pos.py + scrollY);

                    addTile(touchTileX,touchTileY);
                    UpdateTiles();
                }
                if (buttonsHeld & KEY_LEFT)  { scrollX -= SPEED; if (scrollX < 0)    scrollX = 0; }
                if (buttonsHeld & KEY_RIGHT) { scrollX += SPEED; if (scrollX > 255)  scrollX = 255; } // 
                if (buttonsHeld & KEY_UP)    { scrollY -= SPEED; if (scrollY < 0)    scrollY = 0; }
                if (buttonsHeld & KEY_DOWN)  { scrollY += SPEED; if (scrollY > 319)   scrollY = 319; } // 
                char buffer[64];
                NF_ClearTextLayer(0, 0); 
                snprintf(buffer,sizeof(buffer),"X px %d Y px %d",scrollX,scrollY);
                NF_WriteText(0,0,1,1,buffer);

                if(scrollX < 16 || scrollX > 240 /*512-16-256*/ || scrollY < 16 || scrollY > 304 /*512 - 192  - 16*/){
                    NF_WriteText(0,0,1,2,"T");
                } else {
                    NF_WriteText(0,0,1,2,"N");
                }
                break;


        }
         

        

        NF_ScrollBg(1, 3, scrollX, scrollY);
        swiWaitForVBlank();
        NF_UpdateTextLayers();  
    }
    return 0;
    
}
