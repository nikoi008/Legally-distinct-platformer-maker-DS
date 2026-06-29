#ifndef TYPES_H
#define TYPES_H

#include <nds.h>

typedef enum{
    MAIN_MENU,
    EDITOR,
    PLAY_SCREEN
} gameStates;

typedef enum{
    IDLE,
    WALKING,
    JUMPING,
    FALLING,
}playerState;




typedef struct{
    u16 buttonsDown;
    u16 buttonsHeld;
    u16 buttonsUp;
    touchPosition touchPos;
    int touchTileX;
    int touchTileY;

}inputs;

typedef struct{
    int originTileX;
    int originTileY;
    int scrollX;
    int scrollY;
    int cameraX;
    int cameraY;
    int cameraTileX;
    int cameraTileY;
}worldCoordinates;

typedef struct{
    int currentBlock;
    int flagPosX;
    int flagPosY;
    bool rectFillOn;
    int firstTouchX;
    int firstTouchY;
}editorContext;

typedef struct{
    int playerX;
    int playerY;
    int velocityX;
    int velocityY;
    bool grounded;
    int frame;
    bool hflip;
    playerState currentState;

}playerContext;

typedef struct{
    int topLeftX;
    int topLeftY;
    int width;
    int height;
}rectangle;

typedef struct
{
    worldCoordinates *coords;
    playerContext *player;
    editorContext *editor;
    inputs *input;
}gameContext;

typedef struct{
    bool solid;
    void (*ifTouched)(gameContext*);
    int topLeftTile;
    int topRightTile;
    int bottomLeftTile;
    int bottomRightTile;
}block;
#endif
