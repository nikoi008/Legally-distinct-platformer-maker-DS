#ifndef KEYBOARD_H
#define  KEYBOARD_H
#include "types.h"

void showKeyboard(bool show);

char keyboardPresses(gameContext *ctx);

char keyboardLoop(int maxChars, char *string,gameContext *ctx);

void displayTyped(char *string,int maxChars);

int getLetterFrame(char letter[1]);
#endif //LEGALLY_DISTINCT_PLATFORMER_MAKER_DS_KEYBOARD_H
