#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

bool checkCollision(rectangle rectA, rectangle rectB);

void playerAnim(gameContext *ctx);

void playerPhysics(gameContext *ctx);

void xCollision(gameContext *ctx,bool leftDirection);

void yCollision(gameContext *ctx);

#endif
