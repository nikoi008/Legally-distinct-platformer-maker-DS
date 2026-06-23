#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

bool checkCollision(rectangle rectA, rectangle rectB);

void playerAnim(playerContext *ctx);

void playerPhysics(playerContext *ctx, inputs *input, worldCoordinates *coords);

void xCollision(playerContext *player,bool leftDirection);

void yCollision(playerContext *player);

#endif
