#ifndef INPUT_H
#define INPUT_H

#include "types.h"

void rectangleFillPreview(editorContext *ctxE,inputs *input,worldCoordinates *coords);

void fill(editorContext *ctxE,inputs *input,worldCoordinates *coords);

void editorInputKeys(worldCoordinates *coords,inputs *input);

void editorFrame(worldCoordinates *coords, inputs *input, editorContext *ctxE);

#endif
