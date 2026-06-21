#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

bool checkCollision(rectangle rectA, rectangle rectB);

void playerAnim(playerContext *ctx);

void playerPhysics(playerContext *ctx, inputs *input, worldCoordinates *coords);

#endif
