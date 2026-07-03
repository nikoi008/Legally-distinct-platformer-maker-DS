#ifndef KEYBOARD_H
#define  KEYBOARD_H
#include "types.h"

void showKeyboard(bool show);

char keyboardPresses(gameContext *ctx);

char keyboardLoop(int maxChars, char *string,gameContext *ctx);

#endif //LEGALLY_DISTINCT_PLATFORMER_MAKER_DS_KEYBOARD_H
