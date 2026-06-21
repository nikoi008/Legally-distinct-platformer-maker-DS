#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"

void updateOrigin(worldCoordinates *coords);

void scrollLogic(worldCoordinates *coords);

void playerScroll(worldCoordinates *coords, playerContext *ctxP);

#endif
