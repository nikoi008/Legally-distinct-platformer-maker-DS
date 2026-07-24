#ifndef SOURCE_CHANGE_PALETTE_H
#define SOURCE_CHANGE_PALETTE_H
#include <nds.h>

void setGreyBg(bool set);

void showChangePalWindow(bool show);

int clampInt(int val, int min, int max);

void changePaletteFrame(gameContext *ctx);

#endif
