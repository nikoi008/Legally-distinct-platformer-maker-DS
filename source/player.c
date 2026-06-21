#include <nds.h>
#include <nf_lib.h>

#include "player.h"
#include "blocks.h"
#include "defines.h"

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
        NF_SpriteFrame(1,5,ctx->frame % 10);
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

    rectangle playerRect = {128,96,16,16};
    rectangle playerLeft = {128 - 16,96,16,16};
    rectangle playerRight = {128 + 32,96,16,16};
    rectangle playerTop = {};
    if(input->buttonsHeld & KEY_LEFT){  ctx->velocityX = -SPEED;ctx->hflip = true; NF_HflipSprite(1,5,ctx->hflip); } //todo probably dont use hflip sprite, as origin is also flipped. better to just make an extra 10 frames??
    if(input->buttonsHeld & KEY_RIGHT){ ctx->velocityX =  SPEED;ctx->hflip = false; NF_HflipSprite(1,5,ctx->hflip);}

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
