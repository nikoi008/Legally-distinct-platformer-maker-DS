#ifndef TILEMAP_H
#define TILEMAP_H

#include "types.h"

void updateTiles(worldCoordinates *coords);

void saveLevel(char* name);

void loadLevel(char* name, worldCoordinates *coords);

#endif
