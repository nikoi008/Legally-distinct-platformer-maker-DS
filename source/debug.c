#include <stdio.h>

#include <nf_lib.h>

#include "debug.h"

void debugText(gameContext *ctx)
{
    char buffer[64];

    snprintf(buffer,sizeof(buffer),"scrollX px %d scrollY px %d",ctx->coords->scrollX,ctx->coords->scrollY);
    NF_WriteText(0,0,1,1,buffer);

    char camCord[64];
    snprintf(camCord,sizeof(camCord),"camX %d camY %d",ctx->coords->cameraX,ctx->coords->cameraY);
    NF_WriteText(0,0,1,3,camCord);

    char tilecoords[64];
    snprintf(tilecoords,sizeof(tilecoords),"tileX %d tileY %d",ctx->input->touchTileX,ctx->input->touchTileY);
    NF_WriteText(0,0,1,4,tilecoords);

    char ocord[64];
    snprintf(ocord,sizeof(ocord),"originX %d originY %d",ctx->coords->originTileX,ctx->coords->originTileY);
    NF_WriteText(0,0,1,5,ocord);


    char fcord[64];
    snprintf(fcord,sizeof(fcord),"flagX %d flagY %d",ctx->editor->flagPosX,ctx->editor->flagPosY);
    NF_WriteText(0,0,1,7,fcord);

    char pCords[64];
    snprintf(pCords,sizeof(pCords),"playerX %d playerY %d",ctx->player->playerX,ctx->player->playerY);
    NF_WriteText(0,0,1,6,pCords);

    char firstTouchCords[64];
    snprintf(firstTouchCords,sizeof(firstTouchCords),"firstT %d firstT %d",ctx->editor->firstTouchX,ctx->editor->firstTouchY);
    NF_WriteText(0,0,1,8,firstTouchCords);


  char dcurrentBlock[32];
  snprintf(dcurrentBlock,sizeof(dcurrentBlock),"current block %d",ctx->editor->currentBlock);
  NF_WriteText(0,0,1,9,dcurrentBlock);

    char leftmosts[32];
    snprintf(leftmosts,sizeof(leftmosts),"lm %d %d",ctx->editor->leftMostX,ctx->editor->leftMostY);
    NF_WriteText(0,0,1,10,leftmosts);

}
