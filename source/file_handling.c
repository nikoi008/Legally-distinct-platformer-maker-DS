#include "types.h"
#include "globals.h"
#include "file_handling.h"
#include <filesystem.h>
#include <fat.h>
#include <sys/stat.h>
#include <nf_lib.h>
#include "tilemap.h"
#include <dirent.h>

void showDirs(gameContext *ctx)
{
    static char levels[512][16];
    static int maxEntries = 0;
    static bool alreadyRead = false;
    static int position = 0;
    static int frame = 0;

    if (!alreadyRead)
    {
        DIR *dr = opendir("fat:/YouMakeLevels/");
        struct dirent *de;
        while (dr != NULL && (de = readdir(dr)) != NULL)
        {
            strncpy(levels[maxEntries], de->d_name, 15);
            levels[maxEntries][15] = '\0';
            maxEntries++;
        }
        if (dr != NULL) closedir(dr);
        alreadyRead = true;
    }

    frame = (frame + 1) % 60;

    if (ctx->input->buttonsUp & KEY_UP)
    {
        position--;
        if (position < 0) position = 0;
    }
    if (ctx->input->buttonsUp & KEY_DOWN)
    {
        position++;
        if (position >= maxEntries) position = maxEntries - 1;
    }

    int textRowCounter = 0;
    for (int i = 0; i < maxEntries; i++)
    {
        if (strchr(levels[i], '.') != NULL) continue;
        if (i < position) continue;

        bool isSelected = (i == position);
        bool blinkOn = (frame <= 30);

        if (!isSelected || blinkOn)
        {
            NF_WriteText(0, 0, 1, (textRowCounter % 32) + 1, levels[i]);
        }

        if (isSelected)
        {
            NF_WriteText(0, 0, 1, 0, levels[i]);

            if (ctx->input->buttonsUp & KEY_A)
            {
                state = EDITOR;
                char dir[64] = "fat:/YouMakeLevels/";
                strcat(dir, levels[i]);
                loadLevel(dir, ctx);
            }
        }

        textRowCounter += 2;
    }
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



