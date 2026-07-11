#ifndef INPUT_H
#define INPUT_H

#include "types.h"

void rectangleFillPreview(gameContext *ctx);

void fill(gameContext *ctx);

void editorInputKeys(gameContext *ctx);

void updateLeftmostTile(gameContext *ctx);

void editorFrame(gameContext *ctx);

void setGreyBg(bool set);

void showChangePalWindow(bool show);

void showEditor(bool show);

#endif
