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

void displayArrows()
{
    for(int x = 4; x <= 7; x ++){
        for(int y = 0; y <= 5; y++){
            NF_SetTileOfMap(0,3,((256 / 8) - 7) + x, y, x + (32 * y));
        }
    }

    for(int x = 8; x <= 11; x++){
        for(int y = 0; y <= 5; y++){
            NF_SetTileOfMap(0,3,((256 / 8) -11) + x, ((192 / 8) - 5) + y, x + (32 * y));
        }
    }
}
void displaySelected(int selected){
    selected += 2;
    for(int i = 0; i < 256 / 8 ; i++){
        NF_SetTileOfMap(0,3,i,selected,2);
        NF_SetTileOfMap(0,3,i,selected - 1,2);
        NF_SetTileOfMap(0,3,i,selected + 1,2);
        for(int j = 0; j < 256 /8; j++){
            if( j != selected && j != selected + 1 && j != selected - 1){
                NF_SetTileOfMap(0,3,i,j,0);
            }
        }
    }
    NF_SetTileOfMap(0,3,20,20,3);
}

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
    static char selectedLevel[16] = "";

    if (ctx->input->buttonsUp & KEY_DOWN) position++;
    if (ctx->input->buttonsUp & KEY_UP) position--;

    int maxPos = ((maxEntries > 12) ? (maxEntries - 12) : 0) + 1;
    position = clampInt(position, 0, maxPos);

    int visibleCount = maxEntries - position;
    if (visibleCount > 12) visibleCount = 12;
    if (visibleCount < 0)  visibleCount = 0;

    if (visibleCount > 0)
        touchPosition = clampInt(touchPosition, 0, visibleCount - 1);
    else
        touchPosition = 0;

    if (ctx->input->buttonsDown & KEY_TOUCH)
    {
        touchRead(&ctx->input->touchPos);

        int tileRow = ctx->input->touchPos.py / 8;
        int mid = (tileRow - 2) / 2;

        if (mid < 0) mid = 0;
        if (visibleCount > 0 && mid > visibleCount - 1) mid = visibleCount - 1;

        if (visibleCount > 0)
            touchPosition = mid;
    }

    int selectedIndex = position + touchPosition;
    if (selectedIndex >= 0 && selectedIndex < maxEntries)
    {
        strncpy(selectedLevel, levels[selectedIndex], 15);
        selectedLevel[15] = '\0';
    }

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
    displaySelected(clampInt(touchPosition * 2, 0, 20));
    NF_WriteText(0, 0, 24, 22, selectedLevel);
    displayArrows();
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



