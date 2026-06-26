#include <nds.h>
#include <nf_lib.h>

#include "editor.h"
#include "globals.h"
#include "defines.h"
#include "blocks.h"
#include "tilemap.h"


void rectangleFillPreview(editorContext *ctxE,inputs *input,worldCoordinates *coords){
    touchRead(&input->touchPos);
    input->touchTileX = coordsToTile(input->touchPos.px + coords->cameraX );
    input->touchTileY = coordsToTile (input->touchPos.py + coords->cameraY );
    updateTiles(coords);
    int highestX = (input->touchTileX >ctxE->firstTouchX) ? input->touchTileX : ctxE->firstTouchX;
    int lowestX  = (input->touchTileX < ctxE->firstTouchX) ? input->touchTileX : ctxE->firstTouchX;
    int highestY = (input->touchTileY> ctxE->firstTouchY) ? input->touchTileY : ctxE->firstTouchY;
    int lowestY  = (input->touchTileY<ctxE->firstTouchY) ? input->touchTileY : ctxE->firstTouchY;
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

void fill(editorContext *ctxE,inputs *input,worldCoordinates *coords){
    ctxE->rectFillOn = false;
    int highestX = (input->touchTileX >ctxE->firstTouchX) ? input->touchTileX : ctxE->firstTouchX;
    int lowestX  = (input->touchTileX < ctxE->firstTouchX) ? input->touchTileX : ctxE->firstTouchX;
    int highestY = (input->touchTileY> ctxE->firstTouchY) ? input->touchTileY : ctxE->firstTouchY;
    int lowestY  = (input->touchTileY<ctxE->firstTouchY) ? input->touchTileY : ctxE->firstTouchY;
    for(int i = lowestY; i < highestY + 1; i++){
        for(int j = lowestX; j < highestX + 1; j++){
            TILE_MAP[i][j] = ctxE->currentBlock;
        }
    }
    updateTiles(coords);
    ctxE->firstTouchX = -1;
    ctxE->firstTouchY = -1;
}
#define HUD_Y_START 22
void editorInputKeys(worldCoordinates *coords,inputs *input){
    if (input->buttonsHeld & KEY_LEFT && coords->cameraX >= 0){
        coords->scrollX -= SPEED;
        coords->cameraX -=SPEED;
    }
    if (input->buttonsHeld & KEY_RIGHT && coords->cameraX <= GRID_X * 16){
        coords->scrollX += SPEED;
        coords->cameraX += SPEED;
    }
    if (input->buttonsHeld & KEY_UP && coords->cameraY >= 0){
        coords->scrollY -= SPEED;
        coords->cameraY -= SPEED;
    }
    if (input->buttonsHeld & KEY_DOWN && coords->cameraY <= GRID_Y * 16){
        coords->scrollY += SPEED;
        coords->cameraY += SPEED;
    }
}
void editorFrame(worldCoordinates *coords,inputs *input,editorContext *ctxE){

    editorInputKeys(coords,input);

    if(input->buttonsHeld & KEY_TOUCH && !ctxE->rectFillOn){
        touchRead(&input->touchPos);
        input->touchTileX = coordsToTile(input->touchPos.px + coords->cameraX );
        input->touchTileY = coordsToTile (input->touchPos.py + coords->cameraY );

        if(input->touchPos.py >= HUD_Y_START){//hud width
            addTile(input->touchTileX,input->touchTileY,ctxE->currentBlock,ctxE);
        }
        updateTiles(coords);
    }
    if(input->buttonsDown & KEY_B && ctxE->currentBlock != 2){
        ctxE->rectFillOn = !ctxE->rectFillOn;
    }

    if (input->buttonsDown & KEY_TOUCH && !ctxE->rectFillOn){
        touchRead(&input->touchPos);

        input->touchTileX = coordsToTile(input->touchPos.px + coords->cameraX );
        input->touchTileY = coordsToTile (input->touchPos.py + coords->cameraY );

        if(input->touchPos.py <= HUD_Y_START){//hud width
            if(input->touchPos.px /16  <= 1){
                ctxE->currentBlock = abs((ctxE->currentBlock - 1)% TOTAL_BLOCKS);
            }
            if(input->touchPos.px /16 >= 14){
                ctxE->currentBlock = abs((ctxE->currentBlock + 1)% TOTAL_BLOCKS);
            }
        }
        updateTiles(coords);
    }
    else if(ctxE->rectFillOn && (ctxE->firstTouchX < 0 || ctxE->firstTouchY < 0) && input->buttonsHeld & KEY_TOUCH){
        touchRead(&input->touchPos);
        ctxE->firstTouchX = coordsToTile(input->touchPos.px + coords->cameraX );
        ctxE->firstTouchY = coordsToTile (input->touchPos.py + coords->cameraY );
    }
    else if(ctxE->rectFillOn && (ctxE->firstTouchX > 0 || ctxE->firstTouchY > 0) && input->buttonsHeld & KEY_TOUCH){
        rectangleFillPreview(ctxE,input,coords);
    }
    else if(ctxE->rectFillOn && (ctxE->firstTouchX > 0 || ctxE->firstTouchY > 0) && input->buttonsUp & KEY_TOUCH){
        fill(ctxE,input,coords);
    }

    if(input->buttonsDown & KEY_A){
        state = PLAY_SCREEN;

        //swap with seperate HUD <-- todo
        NF_ShowSprite(1,1,false);
        NF_ShowSprite(1,2,false);
        NF_ShowSprite(1,3,false);
        NF_ShowSprite(1,4,false);
        NF_DeleteTiledBg(1,2);
        lcdSwap();
    }

    if(input->buttonsDown & KEY_X){
        saveLevel("fat:/YouMakeLevels/level.txt");
    }

    if(input->buttonsDown & KEY_Y){
        loadLevel("fat:/YouMakeLevels/level.txt",coords);
    }
}
