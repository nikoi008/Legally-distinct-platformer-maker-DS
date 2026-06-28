#include <nds.h>
#include <nf_lib.h>

#include "player.h"
#include "blocks.h"
#include "defines.h"
#include "globals.h"

bool checkCollision(rectangle rectA, rectangle rectB){
    return !(rectA.topLeftX > rectB.topLeftX + rectB.width ||
             rectA.topLeftX + rectA.width < rectB.topLeftX ||
             rectA.topLeftY + rectA.height < rectB.topLeftY ||
             rectA.topLeftY > rectB.topLeftY + rectB.height); // returns true if overlapping
}

void playerAnim(playerContext *ctx){
    if(ctx->currentState == IDLE){
        NF_SpriteFrame(1,5,0);
    }else{
        ctx->frame++;
        ctx->frame = ctx->frame * 2;
        NF_SpriteFrame(1,5,ctx->frame % 10);
    }

}

void xCollision(playerContext *player,bool leftDirection){
    int predictedPlace = player->playerX + ((leftDirection == true) ? -1 : 1 + 16);
    if(tileSolid(predictedPlace/16,player->playerY /  16)){
        player->velocityX = 0;
        if(blocks[TILE_MAP[predictedPlace / 16][player->playerY / 16]].ifTouched != NULL){
            blocks[TILE_MAP[predictedPlace / 16][player->playerY / 16]].ifTouched();
        }
    }
}

 
void yCollision(playerContext *player){ //TODO implement aabb because this will bite me in the back when enemies are implemented
    if(player->velocityY > 0){

        int predictedBottomY = player->playerY + player->velocityY + 15;

        if(tileSolid(player->playerX / 16,predictedBottomY / 16) || tileSolid((player->playerX + 15) / 16, predictedBottomY / 16)){
            player->playerY = (predictedBottomY / 16) * 16 - 16;
            player->velocityY = 0;
            player->grounded = true;

            if(blocks[TILE_MAP[player->playerX / 16][predictedBottomY / 16 / 16]].ifTouched != NULL){
                blocks[TILE_MAP[player->playerX / 16][predictedBottomY/ 16]].ifTouched();
            }
        }

    } else if(player->velocityY < 0){

        int predictedTopY = player->playerY + player->velocityY;
        if(tileSolid(player->playerX / 16, predictedTopY / 16) || tileSolid((player->playerX + 15) / 16, predictedTopY / 16)){
            player->playerY = ((predictedTopY / 16) + 1) * 16;
            player->velocityY = 0;

            if(blocks[TILE_MAP[player->playerX / 16][predictedTopY / 16 / 16]].ifTouched != NULL){
                blocks[TILE_MAP[player->playerX / 16][predictedTopY / 16]].ifTouched();
            }
            
        }
    }
}

void playerPhysics(playerContext *ctx, inputs *input, worldCoordinates *coords){
    ctx->velocityX = 0;
    u16 bH = input->buttonsHeld;
    if(bH & KEY_LEFT || bH & KEY_RIGHT){
        ctx->currentState = WALKING;
    }else{
        ctx->currentState = IDLE;
    }

    if(input->buttonsHeld & KEY_LEFT){  
        ctx->velocityX = -SPEED;ctx->hflip = true; 
        NF_HflipSprite(1,5,ctx->hflip); 
        xCollision(ctx,true);
    } 
    if(input->buttonsHeld & KEY_RIGHT){
        ctx->velocityX =  SPEED;
        ctx->hflip = false;
        NF_HflipSprite(1,5,ctx->hflip);
        xCollision(ctx,false);
    }

    if(input->buttonsDown & KEY_B && ctx->grounded){
        ctx->velocityY = -JUMP_FORCE;
        ctx->grounded = false;
    } else if(!tileSolid(ctx->playerX/16, (ctx->playerY/16)+1)){
        ctx->velocityY += GRAVITY; // todo when falling too fast clips through blocks
        ctx->grounded = false;
    } else {
        ctx->velocityY = 0;
        ctx->grounded = true;
        
    }
    yCollision(ctx);
    ctx->playerX += ctx->velocityX;
    ctx->playerY += ctx->velocityY;
}
