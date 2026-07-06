#include <nds.h>
#include <nf_lib.h>

#include "editor.h"
#include "globals.h"
#include "defines.h"
#include "blocks.h"
#include "tilemap.h"
#include "keyboard.h"
#include <string.h>

void rectangleFillPreview(gameContext *ctx){ // causes huge lag when rectangles are large.. todo optimise??
    touchRead(&ctx->input->touchPos);                                                        // might just be emu lag??? desmume struggles but melonds is fine????? todo test on real ds
    ctx->input->touchTileX = coordsToTile(ctx->input->touchPos.px + ctx->coords->cameraX );
    ctx->input->touchTileY = coordsToTile (ctx->input->touchPos.py + ctx->coords->cameraY );
    updateTiles(ctx);
    int highestX = (ctx->input->touchTileX > ctx->editor->firstTouchX) ? ctx->input->touchTileX : ctx->editor->firstTouchX;
    int lowestX  = (ctx->input->touchTileX < ctx->editor->firstTouchX) ? ctx->input->touchTileX : ctx->editor->firstTouchX;
    int highestY = (ctx->input->touchTileY> ctx->editor->firstTouchY) ? ctx->input->touchTileY : ctx->editor->firstTouchY;
    int lowestY  = (ctx->input->touchTileY<ctx->editor->firstTouchY) ? ctx->input->touchTileY : ctx->editor->firstTouchY;
    for(int i = lowestY; i <= highestY; i++){
        for(int j = lowestX; j <= highestX; j++){
            int screenX = (j - ctx->coords->originTileX) * 2;
            int screenY = (i - ctx->coords->originTileY) * 2;
            if(screenX < 0 || screenX >= 64 || screenY < 0 || screenY >= 64) continue;
            NF_SetTileOfMap(1, TILE_LAYER, screenX,screenY,  blocks[ctx->editor->currentBlock].topLeftTile+ TRANSPARENT_BLOCK_OFFSET);
            NF_SetTileOfMap(1, TILE_LAYER, screenX+1,screenY,blocks[ctx->editor->currentBlock].topRightTile + TRANSPARENT_BLOCK_OFFSET);
            NF_SetTileOfMap(1, TILE_LAYER, screenX,screenY+1,blocks[ctx->editor->currentBlock].bottomLeftTile+ TRANSPARENT_BLOCK_OFFSET);
            NF_SetTileOfMap(1, TILE_LAYER, screenX+1, screenY+1,blocks[ctx->editor->currentBlock].bottomRightTile+ TRANSPARENT_BLOCK_OFFSET);
        }
    }
}

void fill(gameContext *ctx){
    ctx->editor->rectFillOn = false;
    int highestX = (ctx->input->touchTileX > ctx->editor->firstTouchX) ? ctx->input->touchTileX : ctx->editor->firstTouchX;
    int lowestX  = (ctx->input->touchTileX < ctx->editor->firstTouchX) ? ctx->input->touchTileX : ctx->editor->firstTouchX;
    int highestY = (ctx->input->touchTileY> ctx->editor->firstTouchY) ? ctx->input->touchTileY : ctx->editor->firstTouchY;
    int lowestY  = (ctx->input->touchTileY<ctx->editor->firstTouchY) ? ctx->input->touchTileY : ctx->editor->firstTouchY;
    for(int i = lowestY; i < highestY + 1; i++){
        for(int j = lowestX; j < highestX + 1; j++){
            TILE_MAP[i][j] = ctx->editor->currentBlock;
        }
    }
    updateTiles(ctx);
    ctx->editor->firstTouchX = -1;
    ctx->editor->firstTouchY = -1;
}
void updateLeftmostTile(gameContext *ctx)
{
    if (ctx->editor->firstTouchX >= 0 &&ctx->editor->leftMostX > ctx->editor->firstTouchX)
    {
        ctx->editor->leftMostX = ctx->editor->firstTouchX / 16;
        ctx->editor->leftMostY = ctx->editor->firstTouchY / 16;
    }
    if ( ctx->input->touchTileX >= 0 && ctx->editor->leftMostX > ctx->input->touchTileX )
    {
        ctx->editor->leftMostX = ctx->input->touchTileX;
        ctx->editor->leftMostY = ctx->input->touchTileY;
    }

}
void editorInputKeys(gameContext *ctx){
    if (ctx->input->buttonsHeld & KEY_LEFT && ctx->coords->cameraX >= 0){
        ctx->coords->scrollX -= SPEED;
        ctx->coords->cameraX -=SPEED;
    }
    if (ctx->input->buttonsHeld & KEY_RIGHT && ctx->coords->cameraX <= GRID_X * 16){
        ctx->coords->scrollX += SPEED;
        ctx->coords->cameraX += SPEED;
    }
    if (ctx->input->buttonsHeld & KEY_UP && ctx->coords->cameraY >= 0){
        ctx->coords->scrollY -= SPEED;
        ctx->coords->cameraY -= SPEED;
    }
    if (ctx->input->buttonsHeld & KEY_DOWN && ctx->coords->cameraY <= GRID_Y * 16){
        ctx->coords->scrollY += SPEED;
        ctx->coords->cameraY += SPEED;
    }
}

void editorFrame(gameContext *ctx)
{
    if (!ctx->input->keyboardOn){
        editorInputKeys(ctx);

        bool touchActive = (ctx->input->buttonsHeld & KEY_TOUCH) || (ctx->input->buttonsDown & KEY_TOUCH);
        if(touchActive){
            touchRead(&ctx->input->touchPos);
            ctx->input->touchTileX = coordsToTile(ctx->input->touchPos.px + ctx->coords->cameraX );
            ctx->input->touchTileY = coordsToTile (ctx->input->touchPos.py + ctx->coords->cameraY );
        }

        bool touchedToggleButton = touchActive && (ctx->input->touchPos.px >= 10 && ctx->input->touchPos.px <= 26 && ctx->input->touchPos.py >= 173);

        if(ctx->input->buttonsHeld & KEY_TOUCH && !ctx->editor->rectFillOn && !touchedToggleButton){
            if(ctx->input->touchPos.py >= HUD_Y_START && ctx->input->touchPos.py <= 192 - 22){//hud width
                addTile(ctx->input->touchTileX,ctx->input->touchTileY,ctx->editor->currentBlock,ctx->editor);
            }
            updateTiles(ctx);
        }
        if(ctx->input->buttonsDown & KEY_B && ctx->editor->currentBlock != 2){
            ctx->editor->rectFillOn = !ctx->editor->rectFillOn;
            ctx->editor->firstTouchX = -1;
            ctx->editor->firstTouchY = -1;
        }

        if (ctx->input->buttonsDown & KEY_TOUCH && !ctx->editor->rectFillOn && !touchedToggleButton){
            if(ctx->input->touchPos.py <= HUD_Y_START){//hud width
                if(ctx->input->touchPos.px /16  <= 1){
                    ctx->editor->currentBlock = abs((ctx->editor->currentBlock - 1)% TOTAL_BLOCKS);
                }
                else if(ctx->input->touchPos.px /16 >= 14){
                    ctx->editor->currentBlock = abs((ctx->editor->currentBlock + 1)% TOTAL_BLOCKS);
                }
                else{
                    //NF_MoveSprite(1,0,(ctxE.currentBlock * 20) + 30,3);
                    int touchBlock = (ctx->input->touchPos.px - 30)/ 20;
                    if(touchBlock < TOTAL_BLOCKS){ctx->editor->currentBlock = touchBlock;}
                }
            }
            updateTiles(ctx);
        }
        else if(ctx->editor->rectFillOn && (ctx->editor->firstTouchX < 0 || ctx->editor->firstTouchY < 0) && ctx->input->buttonsHeld & KEY_TOUCH && !touchedToggleButton){
            ctx->editor->firstTouchX = ctx->input->touchTileX;
            ctx->editor->firstTouchY = ctx->input->touchTileY;
        }
        else if(ctx->editor->rectFillOn && (ctx->editor->firstTouchX > 0 || ctx->editor->firstTouchY > 0) && ctx->input->buttonsHeld & KEY_TOUCH && !touchedToggleButton){
            rectangleFillPreview(ctx);
        }
        else if(ctx->editor->rectFillOn && (ctx->editor->firstTouchX > 0 || ctx->editor->firstTouchY > 0) && ctx->input->buttonsUp & KEY_TOUCH && !touchedToggleButton){
            fill(ctx);
        }

        if(ctx->input->buttonsDown & KEY_A){
            state = PLAY_SCREEN;

            //swap with seperate HUD <-- todo
            NF_ShowSprite(1,1,false);
            NF_ShowSprite(1,2,false);
            NF_ShowSprite(1,3,false);
            NF_ShowSprite(1,4,false);
            NF_DeleteTiledBg(1,2);
            lcdSwap();
        }
        //NF_CreateSprite(1,30,1,1,10,173);
        if (ctx->input->buttonsDown & KEY_TOUCH && touchedToggleButton)
        {
            ctx->editor->rectFillOn = !ctx->editor->rectFillOn;
            ctx->editor->firstTouchX = -1;
            ctx->editor->firstTouchY = -1;
        }
        NF_ShowSprite(1,30,ctx->editor->rectFillOn);
        if (ctx->input->buttonsDown & KEY_SELECT)
        {
            ctx->input->keyboardOn = true;
            showKeyboard(true);
            state = LEVEL_SAVE;
        }
    }
}