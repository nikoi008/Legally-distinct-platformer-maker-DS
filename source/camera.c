#include "camera.h"
#include "blocks.h"
#include "tilemap.h"

void updateOrigin(worldCoordinates *coords){
    coords->originTileX = coords->cameraTileX - 8;
    coords->originTileY = coords->cameraTileY - 10;
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
