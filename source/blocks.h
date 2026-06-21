#ifndef BLOCKS_H
#define BLOCKS_H

#include "types.h"

void initBlocks(void);

int coordsToTile(int coord);

bool tileSolid(int tileX, int tileY);

void addTile(int tileX, int tileY, int currentBlock, editorContext *ctx);

#endif 
