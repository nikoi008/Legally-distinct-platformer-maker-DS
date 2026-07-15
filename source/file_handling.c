#include "types.h"
#include "globals.h"
#include "file_handling.h"
#include <filesystem.h>
#include <fat.h>
#include <sys/stat.h>
#include <nf_lib.h>
#include "tilemap.h"
#include <dirent.h>
#include "change_palette.h"

void showDirs(gameContext *ctx)
{
    static char levels[512][16];
    static int maxEntries = 0;
    static bool alreadyRead = false;

    if (!alreadyRead)
    {
        maxEntries = 0;
        DIR *dr = opendir("fat:/YouMakeLevels/");
        struct dirent *de;
        while (dr != NULL && (de = readdir(dr)) != NULL)
        {
            if (strchr(de->d_name, '.') == NULL)
            {
                if (maxEntries >= 512) break; 
                strncpy(levels[maxEntries], de->d_name, 15);
                levels[maxEntries][15] = '\0';
                maxEntries++;
            }
        }
        if (dr != NULL) closedir(dr);
        alreadyRead = true;
    }

    static int touchPosition = 0;
    static int position = 0;

    if (ctx->input->buttonsUp & KEY_DOWN) position++;
    if (ctx->input->buttonsUp & KEY_UP) position--;
    if (ctx->input->buttonsUp & KEY_TOUCH) touchPosition = 0;

    int maxPos = (maxEntries > 12) ? (maxEntries - 12) : 0;
    position = clampInt(position, 0, maxPos);

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "pos %d", position);
    NF_WriteText(0, 0, 24, 0, buffer);

    for (int i = 0; i < 12; i++)
    {
        int idx = i + position;
        if (idx >= maxEntries) break; 
        NF_WriteText(0, 0, 1, 2 + (i * 2), levels[idx]);
    }

    NF_WriteText(0, 0, 24, clampInt(touchPosition * 2, 2, 20), "selected");
}
void testWriteSizes()
{
    FILE *f;
    size_t n;

    f = fopen("fat:/YouMakeLevels/test1k.bin", "wb");
    n = fwrite(TILE_MAP, 1, 1024, f);
    fclose(f);

    f = fopen("fat:/YouMakeLevels/test64k.bin", "wb");
    n = fwrite(TILE_MAP, 1, 65536, f);
    fclose(f);

    f = fopen("fat:/YouMakeLevels/test512k.bin", "wb");
    n = fwrite(TILE_MAP, 1, 524288, f);
    fclose(f);
}

void saveLevel(char* name){
    FILE *ptr = fopen(name, "wb");
    if (!ptr) return;
    fwrite(TILE_MAP, sizeof(u8), GRID_X * GRID_Y, ptr);
    fclose(ptr);
}

void loadLevel(char* name,gameContext *ctx){
    FILE *ptr = fopen(name, "rb");
    if (!ptr) return;
    fread(TILE_MAP, sizeof(u8), GRID_X * GRID_Y, ptr);

    for (int i = 0; i < GRID_Y; i++)
    {
        for (int j = 0; j < GRID_X; j++)
        {
            if (TILE_MAP[i][j] == 2)
            {
                ctx->editor->flagPosX = j;
                ctx->editor->flagPosY = i;
            }

        }
    }
    fclose(ptr);

    updateTiles(ctx);
}



