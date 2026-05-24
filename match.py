#include <stdio.h>
#include <time.h>

#include <nds.h>
#include <filesystem.h>
#include <maxmod9.h>
#include "simplenfl.h"
#include <stdbool.h>
#include <nf_lib.h>
#include <maxmod9.h>
#include "soundbank.h"
#include <fat.h>
#include <string.h>

#define GRID_ROWS 16
#define GRID_COLS 8

typedef enum
{
    TITLE,
    SELECT_SAVE,
    INIT_MATCH,
    GAME_MATCH,
    WIN
} States;

typedef enum
{
    NEUTRAL,
    UP,
    LEFT,
    DOWN,
    RIGHT
} directions;

int grid[GRID_ROWS][GRID_COLS] = {0};
int spriteIdGrid[GRID_ROWS][GRID_COLS];
int score = 0;  
bool flushed = false;

void initGrid()
{
    int temp;
    for (int i = 0; i < GRID_ROWS; i++) 
    {
        for (int j = 0; j < GRID_COLS; j++) 
        {
            
            do {
                temp = rand() % 7;
            } 
            while ((j >= 2 && grid[i][j-1] == temp && grid[i][j-2] == temp) || (i >= 2 && grid[i-1][j] == temp && grid[i-2][j] == temp));   
            grid[i][j] = temp;
        }
    }
}

int totalMatches = 0;
bool matchGrid[8][8];

typedef enum
{
    HORIZONTAL,
    VERTICAL
} matchtype;
void drawScore(){
    
    NF_Set3dSpriteFrame(200, (score / 1000) % 10);
    NF_Set3dSpriteFrame(201,(score / 100) % 10);
    NF_Set3dSpriteFrame(202,(score / 10) % 10);
    NF_Set3dSpriteFrame(203,score % 10);
}
void animatePop(int length, int row, int col, matchtype type)
{
    int ids[length];

    int frames = 16;
    for (int i = 0; i < length; i++)
    {
        if (type == HORIZONTAL)
        {
            ids[i] = spriteIdGrid[row][col + i];
        }
        else
        {
            ids[i] = spriteIdGrid[row + i][col];
        }
    }
    flushed = true;
    for (int i = 0; i < frames; i++)
    {
        for (int j = 0; j < length; j++)
        {
            NF_Set3dSpriteFrame(ids[j], 8 + (i / (frames / 4)));
        }
        NF_SpriteOamSet(0);
        NF_SpriteOamSet(1);
        NF_Update3dSpritesGfx();
        NF_Draw3dSprites();
        glFlush(0);
        swiWaitForVBlank();
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }
    score += 10 * length;

    for (int i = 0; i < frames; i++)
    {
        for (int j = 0; j < length; j++)
        {
            NF_Set3dSpriteFrame(ids[j], 12);
        }
        NF_SpriteOamSet(0);
        NF_SpriteOamSet(1);
        NF_Update3dSpritesGfx();
        NF_Draw3dSprites();
        glFlush(0);
        swiWaitForVBlank();
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }
}

void drawGridBottom()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            int tileValue = grid[j + 8][i];
            int spriteID = spriteIdGrid[j + 8][i];

            NF_Move3dSprite(spriteID, 34 + (i * 24), (j * 24) + 2);

            if (tileValue == -1)
            {
                NF_Set3dSpriteFrame(spriteID, 0);
            }
            else
            {
                NF_Set3dSpriteFrame(spriteID, tileValue);
            }
        }
    }
}

bool findMatches()
{
    totalMatches = 0;
    bool found = false;
    int length = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            matchGrid[i][j] = false;
        }
    }

    for (int cols = 0; cols < GRID_COLS; cols++)
    {
        for (int rows = 8; rows < GRID_ROWS - 2; rows++)
        {
            int currentTile = grid[rows][cols];

            if (currentTile != -1 && currentTile == grid[rows + 1][cols] && currentTile == grid[rows + 2][cols])
            {
                int matchGridY = rows - 8;
                matchGrid[matchGridY][cols] = true;
                matchGrid[matchGridY + 1][cols] = true;
                matchGrid[matchGridY + 2][cols] = true;
                found = true;
                totalMatches += 1;
                length = 3;

                if (rows + 3 < GRID_ROWS && currentTile == grid[rows + 3][cols])
                {
                    matchGrid[matchGridY + 3][cols] = true;
                    length = 4;

                    if (rows + 4 < GRID_ROWS && currentTile == grid[rows + 4][cols])
                    {
                        matchGrid[matchGridY + 4][cols] = true;
                        length = 5;
                    }
                }

                animatePop(length, rows, cols, VERTICAL);
                rows += length - 1;
            }
        }
    }

    for (int rows = 8; rows < GRID_ROWS; rows++)
    {
        for (int cols = 0; cols < GRID_COLS - 2; cols++)
        {
            int currentTile = grid[rows][cols];

            if (currentTile != -1 && currentTile == grid[rows][cols + 1] && currentTile == grid[rows][cols + 2])
            {
                int matchGridY = rows - 8;
                matchGrid[matchGridY][cols] = true;
                matchGrid[matchGridY][cols + 1] = true;
                matchGrid[matchGridY][cols + 2] = true;
                found = true;
                totalMatches += 1;
                length = 3;

                if (cols + 3 < GRID_COLS && currentTile == grid[rows][cols + 3])
                {
                    matchGrid[matchGridY][cols + 3] = true;
                    length = 4;

                    if (cols + 4 < GRID_COLS && currentTile == grid[rows][cols + 4])
                    {
                        matchGrid[matchGridY][cols + 4] = true;
                        length = 5;
                    }
                }

                animatePop(length, rows, cols, HORIZONTAL);
                cols += length - 1;
            }
        }
    }
    return found;
}

void animateSpriteSwipe(Swipe SwipeDir, int gridX, int gridY)
{
    int targetX = gridX;
    int targetY = gridY;

    if (SwipeDir == SwipeLeft) targetX--;
    if (SwipeDir == SwipeRight) targetX++;
    if (SwipeDir == SwipeUp) targetY--;
    if (SwipeDir == SwipeDown) targetY++;

    if (targetX < 0 || targetX > 7 || targetY < 8 || targetY > 15) return;

    int sprite1 = spriteIdGrid[gridY][gridX];
    int sprite2 = spriteIdGrid[targetY][targetX];

    int neighbourStartX = 34 + (gridX * 24);
    int neighbourStartY = ((gridY - 8) * 24) + 2;

    int targetXStart = 34 + (targetX * 24);
    int targetYStart = ((targetY - 8) * 24) + 2;

    int frames = 15;
    for (int i = 1; i <= frames; i++)
    {
        int currentXNeighbour = neighbourStartX + ((targetXStart - neighbourStartX) * i / frames);
        int currentYNeighbour = neighbourStartY + ((targetYStart - neighbourStartY) * i / frames);
        int currentXTarget = targetXStart + ((neighbourStartX - targetXStart) * i / frames);
        int currentYTarget = targetYStart + ((neighbourStartY - targetYStart) * i / frames);

        NF_Move3dSprite(sprite1, currentXNeighbour, currentYNeighbour);
        NF_Move3dSprite(sprite2, currentXTarget, currentYTarget);
        NF_Draw3dSprites();
        glFlush(0);
        swiWaitForVBlank();
        NF_Update3dSpritesGfx();
    }
}

bool possibleMatches()
{
    for (int row = 8; row < GRID_ROWS; row++)
    {
        for (int col = 0; col < GRID_COLS; col++)
        {
            if (col + 1 < GRID_COLS && grid[row][col] == grid[row][col + 1])
            {
                if (col + 2 < GRID_COLS && row + 1 < GRID_ROWS && row - 1 >= 8)
                {
                    if (grid[row][col] == grid[row][col + 2] || grid[row][col] == grid[row + 1][col + 1] || grid[row][col] == grid[row + 1][col - 1]) 
                        return true;
                }
            }

            if (col + 2 < GRID_COLS && grid[row][col] != grid[row][col + 1] && grid[row][col + 1] == grid[row][col + 2])
            {
                if (col - 1 >= 0 && row + 1 < GRID_ROWS && row - 1 >= 8)
                {
                    if (grid[row][col + 1] == grid[row - 1][col] || grid[row + 1][col] == grid[row][col + 1] || (col - 2 >= 0 && grid[row][col - 1] == grid[row][col - 2])) 
                        return true;
                }
            }

            if (col + 2 < GRID_COLS && grid[row][col] == grid[row][col + 2] && grid[row][col] != grid[row][col + 1])
            {
                if (row - 1 >= 8 && row + 1 < GRID_ROWS)
                {
                    if (grid[row][col] == grid[row + 1][col + 1] || (col - 1 >= 0 && grid[row][col] == grid[row - 1][col - 1])) 
                        return true;
                }
            }

            if (row + 2 < GRID_ROWS && grid[row][col] == grid[row + 2][col] && grid[row][col] != grid[row + 1][col])
            {
                if (col - 1 >= 0 && col + 1 < GRID_COLS)
                {
                    if (grid[row][col] == grid[row + 1][col - 1] || grid[row][col] == grid[row + 1][col + 1]) 
                        return true;
                }
            }

            if (row + 2 < GRID_ROWS && grid[row][col] == grid[row + 1][col] && grid[row][col] != grid[row + 2][col])
            {
                if (row + 3 < GRID_ROWS && col - 1 >= 0 && col + 1 < GRID_COLS)
                {
                    if (grid[row][col] == grid[row + 3][col] || grid[row][col] == grid[row + 2][col - 1] || grid[row][col] == grid[row + 2][col + 1]) 
                        return true;
                }
            }

            if (row + 2 < GRID_ROWS && grid[row + 1][col] != grid[row][col] && grid[row + 1][col] == grid[row + 2][col])
            {
                if (row - 1 >= 8 && col - 1 >= 0 && col + 1 < GRID_COLS)
                {
                    if (grid[row + 1][col] == grid[row][col - 1] || grid[row + 1][col] == grid[row][col + 1] || grid[row - 1][col] == grid[row + 1][col]) 
                        return true;
                }
            }
        }
    }
    return false;
}

bool findSwipeMatches()
{
    int length = 0;
    for (int cols = 0; cols < GRID_COLS; cols++)
    {
        for (int rows = 8; rows < GRID_ROWS - 2; rows++)
        {
            int currentTile = grid[rows][cols];
            if (currentTile != -1 && currentTile == grid[rows + 1][cols] && currentTile == grid[rows + 2][cols])
            {
                return true;
            }
        }
    }
    for (int rows = 8; rows < GRID_ROWS; rows++)
    {
        for (int cols = 0; cols < GRID_COLS - 2; cols++)
        {
            int currentTile = grid[rows][cols];
            if (currentTile != -1 && currentTile == grid[rows][cols + 1] && currentTile == grid[rows][cols + 2])
            {
                return true;
            }
        }
    }
    return false;
}

bool swipeBlocks(int gridX, int gridY, Swipe swipeDir)
{
    if (gridX > 7 || gridX < 0 || gridY > 15 || gridY < 7) return false;
 
    switch (swipeDir)
    {
    case (SwipeLeft):
    {
        if (gridX - 1 < 0) return false;
        int temp = grid[gridY][gridX];
        grid[gridY][gridX] = grid[gridY][gridX - 1];
        grid[gridY][gridX - 1] = temp;
        animateSpriteSwipe(SwipeLeft, gridX, gridY);
        drawGridBottom();
        if (findSwipeMatches())
        {
            break;
        }
        else
        {
            grid[gridY][gridX - 1] = grid[gridY][gridX];
            grid[gridY][gridX] = temp;
            animateSpriteSwipe(SwipeRight, gridX - 1, gridY);
            drawGridBottom();
        }
        break;
    }
    case (SwipeRight):
    {
        if (gridX + 1 > 7) return false;
        int temp = grid[gridY][gridX];
        grid[gridY][gridX] = grid[gridY][gridX + 1];
        grid[gridY][gridX + 1] = temp;
        animateSpriteSwipe(SwipeRight, gridX, gridY);
        drawGridBottom();
        if (findSwipeMatches())
        {
            break;
        }
        else
        {
            grid[gridY][gridX + 1] = grid[gridY][gridX];
            grid[gridY][gridX] = temp;
            animateSpriteSwipe(SwipeLeft, gridX + 1, gridY);
            drawGridBottom();
        }
        break;
    }
    case (SwipeUp):
    {
        if (gridY - 1 < 0) return false;
        int temp = grid[gridY][gridX];
        grid[gridY][gridX] = grid[gridY - 1][gridX];
        grid[gridY - 1][gridX] = temp;
        animateSpriteSwipe(SwipeUp, gridX, gridY);
        drawGridBottom();
        if (findSwipeMatches())
        {
            break;
        }
        else
        {
            grid[gridY - 1][gridX] = grid[gridY][gridX];
            grid[gridY][gridX] = temp;
            animateSpriteSwipe(SwipeDown, gridX, gridY - 1);
            drawGridBottom();
        }
        break;
    }
    case (SwipeDown):
    {
        if (gridY + 1 > 15) return false;
        int temp = grid[gridY][gridX];
        grid[gridY][gridX] = grid[gridY + 1][gridX];
        grid[gridY + 1][gridX] = temp;
        animateSpriteSwipe(SwipeDown, gridX, gridY);
        drawGridBottom();
        if (findSwipeMatches())
        {
            break;
        }
        else
        {
            grid[gridY + 1][gridX] = grid[gridY][gridX];
            grid[gridY][gridX] = temp;
            animateSpriteSwipe(SwipeUp, gridX, gridY + 1);
            drawGridBottom();
        }
        break;
    }
    }
    flushed = true;
    return true;
}

void applyMatches()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (matchGrid[i][j] == true)
            {
                grid[i + 8][j] = -1;
                NF_Set3dSpriteFrame(spriteIdGrid[i + 8][j], 8);
            }
        }
    }
}

typedef struct
{
    int emptyX;
    int emptyY;
} falling;
void applyGravitySilent() {
    for (int col = 0; col < GRID_COLS; col++) {
        for (int row = 15; row > 0; row--) {
            if (grid[row][col] == -1) {
                for (int swapRow = row - 1; swapRow >= 0; swapRow--) {
                    if (grid[swapRow][col] != -1) {
                        grid[row][col] = grid[swapRow][col];
                        grid[swapRow][col] = -1;
                        break; 
                    }
                }
            }
        }
    }
}
void applyGravity()
{
    bool movedSomething = true;
    int frames = 4;
    while (movedSomething)
    {
        movedSomething = false;

        for (int row = 15; row > 0; row--)
        {
            for (int col = 0; col < GRID_COLS; col++)
            {
                if (grid[row][col] == -1 && grid[row - 1][col] != -1)
                {
                    if (row > 8)
                    {
                        for (int i = 1; i <= frames; i++)
                        {
                            int yOffset = (24 * i / frames);
                            int startY = ((row - 1 - 8) * 24) + 2;
                            int targetY = ((row - 8) * 24) + 2;

                            NF_Move3dSprite(spriteIdGrid[row - 1][col], 34 + (col * 24), startY + yOffset);
                            NF_Set3dSpriteFrame(spriteIdGrid[row][col], 8);
                            NF_Move3dSprite(spriteIdGrid[row][col], 34 + (col * 24), targetY - yOffset);

                            NF_Draw3dSprites();
                            glFlush(0);
                            swiWaitForVBlank();
                            NF_Update3dSpritesGfx();
                        }

                        grid[row][col] = grid[row - 1][col];
                        grid[row - 1][col] = -1;

                        int tempId = spriteIdGrid[row][col];
                        spriteIdGrid[row][col] = spriteIdGrid[row - 1][col];
                        spriteIdGrid[row - 1][col] = tempId;

                        movedSomething = true;
                        flushed = true;
                    }
                    else if (row == 8)
                    {
                        int topStartY = (7 * 24) + 2;
                        int bottomTargetY = 2;
                        int bottomStartY = bottomTargetY - 24;

                        int xPos = 34 + (col * 24);
                        NF_Set3dSpriteFrame(spriteIdGrid[row][col], grid[row - 1][col]);

                        for (int i = 1; i <= frames; i++)
                        {
                            int yOffset = (24 * i / frames);

                            NF_MoveSprite(1, spriteIdGrid[row - 1][col], xPos, topStartY + yOffset);

                            NF_Move3dSprite(spriteIdGrid[row][col], xPos, bottomStartY + yOffset);

                            NF_SpriteOamSet(1);
                            NF_Draw3dSprites();
                            glFlush(0);
                            swiWaitForVBlank();
                            oamUpdate(&oamSub);
                            NF_Update3dSpritesGfx();
                        }

                        grid[row][col] = grid[row - 1][col];
                        grid[row - 1][col] = -1;

                        NF_MoveSprite(1, spriteIdGrid[row - 1][col], xPos, topStartY);

                        movedSomething = true;
                        flushed = true;
                    }
                    else if (row < 8)
                    {
                        for (int i = 1; i <= frames; i++)
                        {
                            int yOffset = (24 * i / frames);

                            int startY = ((row - 1) * 24) + 2;
                            int targetY = (row * 24) + 2;

                            NF_SpriteFrame(1, spriteIdGrid[row][col], 8);
                            NF_MoveSprite(1, spriteIdGrid[row - 1][col], 34 + (col * 24), startY + yOffset);
                            NF_MoveSprite(1, spriteIdGrid[row][col], 34 + (col * 24), targetY - yOffset);

                            NF_SpriteOamSet(0);
                            NF_SpriteOamSet(1);
                            swiWaitForVBlank();
                            oamUpdate(&oamMain);
                            oamUpdate(&oamSub);
                            NF_Draw3dSprites();
                            glFlush(0);
                        }

                        grid[row][col] = grid[row - 1][col];
                        grid[row - 1][col] = -1;

                        int tempId = spriteIdGrid[row][col];
                        spriteIdGrid[row][col] = spriteIdGrid[row - 1][col];
                        spriteIdGrid[row - 1][col] = tempId;

                        movedSomething = true;
                    }
                }
            }
        }
    }
}

void fillEmptySpaces()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            if (grid[j][i] == -1)
            {
                grid[j][i] = rand() % 7;
            }
        }
    }
}

void drawGridTop()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            int tileValue = grid[j][i];
            int spriteID = spriteIdGrid[j][i];

            NF_MoveSprite(1, spriteID, 34 + (i * 24), (j * 24) + 2);

            if (tileValue == -1)
            {
                NF_SpriteFrame(1, spriteID, 0);
            }
            else
            {
                int frame = tileValue;
                if (frame > 6) frame = 6;
                NF_SpriteFrame(1, spriteID, frame);
            }
        }
    }
}

void shuffle()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 8; j < 16; j++)
        {
            grid[j][i] = -1;
        }
        for (int i = 0; i < 3; i++)
        {
            applyGravitySilent();
            fillEmptySpaces();
        }
    }
}

bool checkForSave()
{
    FILE *save = fopen("fat:/M3save.sav", "r");
    if (save)
    {
        fclose(save);
        return true;
    }
    else
    {
        save = fopen("fat:/M3save.sav", "w");
        if (save) fclose(save);
        return false;
    }
}

void saveGame()
{
    FILE *save = fopen("fat:/M3save.sav", "w");
    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            fprintf(save, "%d ", grid[i][j]);
        }
        fprintf(save, "\n");
    }
    fclose(save);
}

void loadGame()
{
    FILE *save = fopen("fat:/M3save.sav", "r");
    if (save == NULL)
    {
        return;
    }
    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            fscanf(save, "%d", &grid[i][j]);
        }
    }
    fclose(save);
}

void drawHighlightGrid(int gridX, int gridY, int spriteId, bool aPressed, directions pointingDir, uint16_t *pkeys_held)
{
    if (*pkeys_held & KEY_START)
    {
       // NF_CreateSprite(1, 2990000, 1, 1, 0, 0);
    }
    if (aPressed)
    {
        //NF_ShowSprite(1, 68, true);
    }
    else
    {
       // NF_ShowSprite(1, 68, false);
    }
    NF_MoveSprite(1, spriteId, (gridX * 24) + 34, (gridY * 24) + 2);
    if (aPressed)
    {
        switch (pointingDir)
        {
        case (UP):
         //   NF_MoveSprite(1, 68, (gridX * 24) + 34, (gridY * 24) + 2 - 24);
            break;
        case (LEFT):
           // NF_MoveSprite(1, 68, (gridX * 24) + 34 - 24, (gridY * 24) + 2);
            break;
        case (DOWN):
          //  NF_MoveSprite(1, 68, (gridX * 24) + 34, (gridY * 24) + 2 + 24);
            break;
        case (RIGHT):
           // NF_MoveSprite(1, 68, (gridX * 24) + 34 + 24, (gridY * 24) + 2);
            break;
        case (NEUTRAL):
           // NF_MoveSprite(1, 68, (gridX * 24) + 34, (gridY * 24) + 2);
        }
    }
}

int main()
{
    uint16_t keys_held;
    touchPosition touch_pos;
    srand(time(NULL));
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);
    
    
    
    swiWaitForVBlank();
    nitroFSInit(NULL);
    NF_SetRootFolder("NITROFS");
    NF_Set3D(0, 0);
    NF_Set2D(1, 0);
    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(0);
    NF_InitTiledBgSys(1);
    NF_InitSpriteBuffers();
    NF_InitSpriteSys(0);
    NF_InitSpriteSys(1);
    NF_InitSpriteBuffers();
    NF_Init3dSpriteSys();
    mmInitDefault("nitro:/soundbank.bin");
    bool fall = true;
    NF_LoadTiledBg("bg/title_bottom", "title_bottom", 256, 256);
    NF_LoadTiledBg("bg/title_top", "title_top", 256, 256);
    soundEnable();
    fatInitDefault();
    if (!checkForSave()) initGrid();
    else loadGame();
    int semitones = 0;
    const uint16_t combo_pitches[] = {1024, 1085, 1149, 1218, 1290, 1366, 1448, 1534, 1625, 1722, 1825, 1933, 2048};
    int gridx = 0;
    int gridy = 0;
    bool touched = false;
    uint16_t keys_up;
    uint16_t keys_down;
    bool swiped = false;
    States state = TITLE;
    mmLoad(MOD_TITLE);
    mmStart(MOD_TITLE, MM_PLAY_LOOP);
    int topGridX = 0;
    int topGridY = 0;
    bool isAPressed = false;
    directions currentDir = NEUTRAL;
    NF_CreateTiledBg(0, 1, "title_top");
    NF_CreateTiledBg(1, 0, "title_bottom");
    bool start = false;
    bool endless = false;
    while (1)
    {
        scanKeys();
        keys_down = keysDown();
        keys_held = keysHeld();
        keys_up = keysUp();
        switch (state)
        {
        case (TITLE):
        {
            glFlush(0);
            if (keys_down & KEY_TOUCH)
            {
                start = true;
            }
            if (start)
            {
                state = INIT_MATCH;

                NF_UnloadTiledBg("title_top");
                NF_UnloadTiledBg("title_bottom");
                NF_DeleteTiledBg(1, 0);
                NF_DeleteTiledBg(0, 1);
                mmUnload(MOD_TITLE);
            }
            break;
        }
        case (INIT_MATCH):
        {


            mmLoadEffect(SFX_POP);
            mmLoad(MOD_GAME);
            NF_LoadSpriteGfx("sprite/sprite3d", 0, 32, 32);
            NF_LoadSpritePal("sprite/sprite3d", 0); 
            NF_Vram3dSpriteGfx(0, 0, false);
            NF_Vram3dSpritePal(0, 0);
            NF_Sort3dSprites();
            NF_LoadSpriteGfx("sprite/sprite2d", 1, 32, 32);
            NF_LoadSpritePal("sprite/sprite2d", 1);
            NF_VramSpritePal(1, 1, 1);
            NF_VramSpriteGfx(1, 1, 1, false);
            NF_LoadTiledBg("bg/match_grid", "match_grid", 256, 256);
            NF_LoadTiledBg("bg/endless","endless",256,256);
            NF_CreateTiledBg(0, 3, "match_grid");
            NF_CreateTiledBg(1, 3, "match_grid");
            NF_LoadSpriteGfx("sprite/numbers",2,8,8);
            NF_LoadSpritePal("sprite/numbers",2);
            NF_Vram3dSpriteGfx(2,2,false);
            NF_Vram3dSpritePal(2,2);
            lcdSwap();

            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    int x = 36 + (i * 24);
                    int y = (j * 24) + 5;
                    spriteIdGrid[j + 8][i] = (i * 8) + j + 64;
                    NF_Create3dSprite((i * 8) + j + 64, 0, 0, x, y);
                    NF_Set3dSpriteFrame((i * 8) + j + 64, grid[i + 8][j]);
                }
            }
            NF_Create3dSprite(200,2,2,6,0);
            NF_Set3dSpriteFrame(200,0);
            NF_Create3dSprite(201,2,2,12,0);
             NF_Set3dSpriteFrame(201,0);
            NF_Create3dSprite(202,2,2,18,0);
             NF_Set3dSpriteFrame(202,0);
            NF_Create3dSprite(203,2,2,24,0);
             NF_Set3dSpriteFrame(203,0);
            NF_Sort3dSprites();

            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    int x = 34 + (i * 24);

                    int y = (j * 24) + 2;
                    spriteIdGrid[j][i] = (i * 8) + j;
                    NF_CreateSprite(1, ((i * 8) + j), 1, 1, x, y);
                    NF_SpriteFrame(1, ((i * 8) + j), grid[j][i]);
                }
            }
          //  findMatches();
          //  applyGravitySilent();
            mmStart(MOD_GAME, MM_PLAY_LOOP);
            mmSetModuleVolume(256);
            state = GAME_MATCH;
            break;
        }

        case (GAME_MATCH):
        {
            if (keys_held & KEY_TOUCH && !touched)
            {
                touchRead(&touch_pos);
                gridx = (int)((touch_pos.px - 36) / 24);
                gridy = 8 + (int)((touch_pos.py - 5) / 24);
                touched = true;
            }
            if (keys_up & KEY_TOUCH && touched)
            {
                touched = false;
            }

            NF_Draw3dSprites();
            if (getSwipeGesture(SwipeLeft)) { swiped = swipeBlocks(gridx, gridy, SwipeLeft); }
            if (getSwipeGesture(SwipeUp)) { swiped = swipeBlocks(gridx, gridy, SwipeUp); }
            if (getSwipeGesture(SwipeDown)) { swiped = swipeBlocks(gridx, gridy, SwipeDown); }
            if (getSwipeGesture(SwipeRight)) { swiped = swipeBlocks(gridx, gridy, SwipeRight); }
            if(!possibleMatches()){
                shuffle();
            }
            bool hasMatches = findMatches();

            if (hasMatches)
            {
                mm_sfxhand h = mmEffect(SFX_POP);
                mmEffectRate(h, combo_pitches[totalMatches % 13]);
                applyMatches();
                fall = false;
            }

            if (keys_down & KEY_A)
            {
                if (isAPressed == false)
                {
                    isAPressed = true;
                }
                else
                {
                    isAPressed = false;
                }
            }
            if (keys_down & KEY_UP)
            {
                if (!isAPressed)
                {
                    topGridY--;
                }
                else
                {
                    if (currentDir == DOWN)
                    {
                        currentDir = NEUTRAL;
                    }
                    else
                    {
                        currentDir = UP;
                    }
                }
            }
            if (keys_down & KEY_DOWN)
            {
                if (!isAPressed)
                {
                    topGridY++;
                }
                else
                {
                    if (currentDir == UP)
                    {
                        currentDir = NEUTRAL;
                    }
                    else
                    {
                        currentDir = DOWN;
                    }
                }
            }
            if (keys_down & KEY_LEFT)
            {
                if (!isAPressed)
                {
                    topGridX--;
                }
                else
                {
                    if (currentDir == RIGHT)
                    {
                        currentDir = NEUTRAL;
                    }
                    else
                    {
                        currentDir = LEFT;
                    }
                }
            }
            if (keys_down & KEY_RIGHT)
            {
                if (!isAPressed)
                {
                    topGridX++;
                }
                else
                {
                    if (currentDir == LEFT)
                    {
                        currentDir = NEUTRAL;
                    }
                    else
                    {
                        currentDir = RIGHT;
                    }
                }
            }
            drawScore();
            drawHighlightGrid(topGridX, topGridY, 67, isAPressed, currentDir, &keys_held);
            if (!fall)
            {
                applyGravity();
                fillEmptySpaces();

                if (!findMatches())
                {
                    fall = true;
                }
                else
                {
                    applyMatches();
                }
            }
            if(score >= 1000 && !endless){
                state = WIN;
                NF_DeleteTiledBg(0, 3);
                
            }
            drawGridTop();
            drawGridBottom();
            if (hasMatches)
            {
                saveGame();
            }
            break;
        }
        case(WIN):
        {
            NF_CreateTiledBg(0, 1, "endless");
            NF_ShowBg(0, 0);
            for(int i = 0; i < 8; i++){
                for(int j = 0; j < 8; j++){
                    NF_Show3dSprite(spriteIdGrid[i + 8][j],false);
                    NF_SpriteFrame(1,spriteIdGrid[i][j],8);
                }
            }
            mmStop();
            mmLoadEffect(SFX_CHEER);
            mmEffect(SFX_CHEER);
            
            NF_SpriteFrame(1,spriteIdGrid[1][1],2);
            NF_SpriteFrame(1,spriteIdGrid[1][6],2);
            NF_SpriteFrame(1,spriteIdGrid[4][1],3);
            NF_SpriteFrame(1,spriteIdGrid[5][1],3);
            NF_SpriteFrame(1,spriteIdGrid[6][2],3);
            NF_SpriteFrame(1,spriteIdGrid[6][3],3);
            NF_SpriteFrame(1,spriteIdGrid[6][4],3);
            NF_SpriteFrame(1,spriteIdGrid[6][5],3);
            NF_SpriteFrame(1,spriteIdGrid[6][2],3);
            NF_SpriteFrame(1,spriteIdGrid[5][6],3);
            NF_SpriteFrame(1,spriteIdGrid[4][6],3);
            if(keys_down & KEY_TOUCH){
                state = GAME_MATCH;
                mmStart(MOD_GAME, MM_PLAY_LOOP);
                mmUnloadEffect(SFX_CHEER);
                NF_UnloadTiledBg("endless");
                NF_DeleteTiledBg(0,1);
                NF_CreateTiledBg(0, 3, "match_grid");
                for(int i = 0; i < 8; i++){
                    for(int j = 0; j < 8; j++){
                            NF_Show3dSprite(spriteIdGrid[i + 8][j],true);
                        }
                    }
                }
                endless = true;
                break;
            }
        }
        if (flushed)
            {
                flushed = false;
            }
            else
            {
                glFlush(0);
            }
            
        NF_SpriteOamSet(0);
        NF_SpriteOamSet(1);
        swiWaitForVBlank();
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
        NF_Update3dSpritesGfx();
    }
}