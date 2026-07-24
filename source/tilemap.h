#ifndef TILEMAP_H
#define TILEMAP_H

#include "types.h"

void updateTiles(gameContext *ctx);

void saveLevel(char *name);

void loadLevel(char *name, gameContext *ctx);

#endif
