#include <nds.h>
#include <nf_lib.h>

#include "player.h"
#include "blocks.h"
#include "defines.h"
#include "globals.h"
#include "types.h"

bool checkCollision(rectangle rectA, rectangle rectB){
    return !(rectA.topLeftX > rectB.topLeftX + rectB.width ||
             rectA.topLeftX + rectA.width < rectB.topLeftX ||
             rectA.topLeftY + rectA.height < rectB.topLeftY ||
             rectA.topLeftY > rectB.topLeftY + rectB.height); // returns true if overlapping
}

void playerAnim(gameContext *ctx){
    if(ctx->player->currentState == IDLE){
        NF_SpriteFrame(1,5,0);
    }else{
        ctx->player->frame++;
        ctx->player->frame = ctx->player->frame;
        NF_SpriteFrame(1,5,ctx->player->frame % 10);
    }

}

void xCollision(gameContext *ctx,bool leftDirection){
    int predictedPlace = ctx->player->playerX + ((leftDirection == true) ? -1 : 1 + 16);

    if(tileSolid(predictedPlace / 16, ctx->player->playerY / 16)){
        ctx->player->velocityX = 0;

        if(blocks[TILE_MAP[ctx->player->playerY / 16 ][predictedPlace / 16]].ifTouched != NULL){
            blocks[TILE_MAP[ctx->player->playerY / 16][predictedPlace / 16]].ifTouched();
        }
    }
}

 
void yCollision(gameContext *ctx){ //TODO implement aabb because this will bite me in the back when enemies are implemented
    if(ctx->player->velocityY > 0){

        int predictedBottomY = ctx->player->playerY + ctx->player->velocityY + 15;

        if(tileSolid(ctx->player->playerX / 16,predictedBottomY / 16) || tileSolid((ctx->player->playerX + 15) / 16, predictedBottomY / 16)){
            ctx->player->playerY = (predictedBottomY / 16) * 16 - 16;
            ctx->player->velocityY = 0;
            ctx->player->grounded = true;

            if(blocks[TILE_MAP[predictedBottomY / 16 ][ctx->player->playerX / 16]].ifTouched != NULL){
                blocks[TILE_MAP[predictedBottomY / 16 ][ctx->player->playerX / 16]].ifTouched();
            }
        }

    } else if(ctx->player->velocityY < 0){

        int predictedTopY = ctx->player->playerY + ctx->player->velocityY;
        if(tileSolid(ctx->player->playerX / 16, predictedTopY / 16) || tileSolid((ctx->player->playerX + 15) / 16, predictedTopY / 16)){
            ctx->player->playerY = ((predictedTopY / 16) + 1) * 16;
            ctx->player->velocityY = 0;

            if(blocks[TILE_MAP[predictedTopY / 16 ][ctx->player->playerX / 16]].ifTouched != NULL){
               blocks[TILE_MAP[predictedTopY / 16 ][ctx->player->playerX / 16]].ifTouched();
            }
            
        }
    }
}

void playerPhysics(gameContext *ctx){

    ctx->player->velocityX = 0;

    if(ctx->input->buttonsHeld & KEY_LEFT || ctx->input->buttonsHeld & KEY_RIGHT){
        ctx->player->currentState = WALKING;
    }else{
        ctx->player->currentState = IDLE;
    }

    if(ctx->input->buttonsHeld & KEY_LEFT){
        ctx->player->velocityX = -SPEED;ctx->player->hflip = true;
        NF_HflipSprite(1,5,ctx->player->hflip);
        xCollision(ctx,true);
    } 
    if(ctx->input->buttonsHeld & KEY_RIGHT){
        ctx->player->velocityX =  SPEED;
        ctx->player->hflip = false;
        NF_HflipSprite(1,5,ctx->player->hflip);
        xCollision(ctx,false);
    }

    if(ctx->input->buttonsDown & KEY_B && ctx->player->grounded){
        ctx->player->velocityY = -JUMP_FORCE;
        ctx->player->grounded = false;
    } else if(!tileSolid(ctx->player->playerX/16, (ctx->player->playerY/16)+1)){
        ctx->player->velocityY += GRAVITY; // todo when falling too fast clips through blocks
        ctx->player->grounded = false;
    } else {
        ctx->player->velocityY = 0;
        ctx->player->grounded = true;
        
    }
    yCollision(ctx);
    ctx->player->playerX += ctx->player->velocityX;
    ctx->player->playerY += ctx->player->velocityY;
}
