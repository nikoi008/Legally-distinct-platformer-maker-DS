#include <stdio.h>

#include <nf_lib.h>

#include "debug.h"

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
    snprintf(firstTouchCords,sizeof(firstTouchCords),"firstT %d firstT %d",ctx->firstTouchX,ctx->firstTouchY);
    NF_WriteText(0,0,1,8,firstTouchCords);

    //char tilePredicted[32];
  //  snprintf(tilePredicted,sizeof(tilePredicted),"%d",)

}
