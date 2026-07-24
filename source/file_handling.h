#ifndef LEGALLY_DISTINCT_PLATFORMER_MAKER_DS_FILE_HANDLING_H
#define LEGALLY_DISTINCT_PLATFORMER_MAKER_DS_FILE_HANDLING_H

void showDirs(gameContext *ctx);

void testWriteSizes();

void saveLevel(char *name);

void loadLevel(char *name, gameContext *ctx);

#endif //LEGALLY_DISTINCT_PLATFORMER_MAKER_DS_FILE_HANDLING_H
