#include "keyboard.h"
#include "types.h"
#include "globals.h"
#include <nf_lib.h>
#include "player.h"

#define SPRITE_HIDE_COORD 300 // only use if sprite has rotscale, otherwise use showsprite
void showKeyboard(bool show)
{
    if (show)
    {
        //
        for (int i = 0; i < 12; i++)
        {
            NF_MoveSprite(1,i + 18,(18 * i) + 23,45);

        }
        NF_MoveSprite(1,17,128 + 37,80);
        NF_MoveSprite(1,16,37,80);

    }
    else
    {
        NF_MoveSprite(1,17,SPRITE_HIDE_COORD,SPRITE_HIDE_COORD);
        NF_MoveSprite(1,16,SPRITE_HIDE_COORD,SPRITE_HIDE_COORD);
        for (int i = 0; i < 12; i++)
        {
            NF_MoveSprite(1,i + 18,SPRITE_HIDE_COORD,SPRITE_HIDE_COORD);

        }
    }


}
char keyboardPresses(gameContext *ctx)
{
    static const rectangle layer1Bounds = {40,80,204 - 40, 104 - 80};
    static const rectangle layer2Bounds = {40,104, 204 - 40 ,126 - 104};
    static const rectangle layer3Bounds = {48,128,198 - 48, 148 - 128};
    static const rectangle layer4Bounds = {64,152,180 - 64, 170 - 152};
    static const rectangle returnBounds1 = {203,80,218 - 203, 146 - 80};
    static const rectangle returnBounds2 = {195,125,218 - 195,146 - 125};
    static const rectangle backspaceBounds = {180,147,217 - 180,168 - 147};


    static const char layer1[10] = {'0','1','2','3','4','5','6','7','8','9'}; // rect bounds from x40 y80 to x204 y 104
    static const char layer2[10] = {'Q','W','E','R','T','Y','U','I','O','P'}; // x40 to y104 x204 y126
    static const char layer3[9] = {'A','S','D','F','G','H','J','K','L'}; //x48 y128 to x198 y148
    static const char layer4[7] = {'Z','X','C','V','B','N','M'}; //x64 y 152 to x180 y170

    char keyPressed[3];

    int  tX = ctx->input->touchPos.px;
    int tY = ctx->input->touchPos.py;
    rectangle tRect = {tX,tY,1,1};
    if (checkCollision(layer1Bounds,tRect))
    {
        nocashMessage("layer 1");
        char key[2];
        key[0] = layer1[(tX - 41) / 16];
        //key[1] = '\0';
        //nocashMessage(key);
        return key[0];

    }

    else if (checkCollision(layer2Bounds,tRect))
    {
        nocashMessage("layer 2");
        //Q 41 W 57,E 73
        //algo is (tx - 41) / 16
        char key[2];
        key[0] = layer2[(tX - 41) / 16];
        key[1] = '\0';
        // nocashMessage(key);
        return key[0];

    }

    else if (checkCollision(layer3Bounds,tRect))
    {
        nocashMessage("layer 3");
        //algo is (tx - 48) / 16
        char key[2];
        key[0] = layer3[(tX - 48) / 16];
        return key[0];
    }
    else if (checkCollision(layer4Bounds,tRect))
    {
        //algo is (tx - 64) /16
        nocashMessage("layer 4");
        char key[2];
        key[0] = layer4[(tX - 64) / 16];
        return key[0];
    }

    else if (tX > 37 && tX < 218 && tY > 168 && tY < 191)
    {
        return ' ';
    }

    else if (checkCollision(returnBounds1,tRect) || checkCollision(returnBounds2,tRect))
    {
        return '+';
    }
    else if (checkCollision(backspaceBounds,tRect))
    {
        return '-';
    }
    return '@';
}

int getLetterFrame(char letter[1])
{
    if (letter[0] >= 'A' && letter[0] <= 'Z')
    {
        return letter[0] - 65;
    }
    if (letter[0] == ' ')
    {
        return 36;
    }
    else if (letter[0] >= '0' && letter[0] <= '9')
    {
        return (letter[0] - 48) + 26;
    }
    return 36;
}

void clearDisplay(int maxChars){
    for (int i = 0; i < maxChars; i++)
    {
        char space = ' ';
        NF_SpriteFrame(1,18 + i, getLetterFrame(&space));
    }
}
void displayTyped(char *string,int maxChars)
{
    for (int i = 0; i < maxChars; i++)
    {
        char letter[1];
        letter[0] = string[i];
        NF_SpriteFrame(1,18 + i, getLetterFrame(&letter[0]));
    }

}
char keyboardLoop(int maxChars, char *string,gameContext *ctx)
{
    static const rectangle cancelBtn = {0,192 - 16,16,16};
    rectangle tRect = {ctx->input->touchPos.px,ctx->input->touchPos.py,1,1};
    static int letterPos = -1;
    char letter[1];

    if (ctx->input->buttonsHeld & KEY_TOUCH)
    {
        touchRead(&ctx->input->touchPos);
    }

    if (ctx->input->buttonsUp & KEY_TOUCH)
    {
        letter[0] = keyboardPresses(ctx);
        if (letter[0] == '-')
        {
            if (letterPos > 0)
            {
                letterPos--;
                string[letterPos] = '\0';
            }

        }
        else if (letter[0] == '@')
        {

        }
        else if (letter[0] == '+')
        {
            letterPos = -1;
            for(int i = 0; i < maxChars; i++){
                string[i] = ' ';
            }
            clearDisplay(maxChars);
            string[0] = '\0';
            return '+';
        }
        else
        {
            if (letterPos < maxChars)
            {
                string[letterPos] = letter[0];
                letterPos++;
            }
        }

        if (checkCollision(tRect,cancelBtn))
        {
            
            letterPos = -1;
            for(int i = 0; i < maxChars; i++){
                string[i] = ' ';
            }         
            clearDisplay(maxChars);
            string[0] = '\0';

            return '*';

        }
    }
    nocashMessage(string);
    char buffer[16];
    snprintf(buffer,sizeof(buffer),"%d",letterPos);
    nocashMessage(buffer);
    displayTyped(string,maxChars);
    return '?';



}

