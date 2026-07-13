#ifndef SOURCE_TRANSITIONS_H
#define SOURCE_TRANSITIONS_H

#include <nf_lib.h>
#include <nds.h>
#include "types.h"
#include "player.h"
#include "globals.h"
#include "editor.h"
#include "debug.h"
#include "camera.h"
#include "tilemap.h"
#include "blocks.h"

void fadeOut(int screen, int frames);
void fadeIn(int screen, int frames);
void editorToPlayScreen(gameContext *ctx);
void playScreenToEditor(gameContext *ctx);

#endif //SOURCE_TRANSITIONS_H
