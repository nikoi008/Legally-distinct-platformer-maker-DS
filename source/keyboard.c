#include "keyboard.h"
#include "types.h"
#include "globals.h"
#include <nf_lib.h>

void showKeyboard(bool show)
{
    if (show)
    {
        //
        for (int i = 0; i < 12; i++)
        {
            NF_MoveSprite(1,i + 18,18 * i,45);

        }
        NF_MoveSprite(1,17,128 + 37,80);
        NF_MoveSprite(1,16,37,80);

    }
    else
    {
        NF_MoveSprite(1,17,300,300);
        NF_MoveSprite(1,16,300,300);
        for (int i = 0; i < 12; i++)
        {
            NF_MoveSprite(1,i + 18,300,300);

        }
    }


}
char keyboardPresses(gameContext *ctx)
{
    static const char layer1[10] = {'0','1','2','3','4','5','6','7','8','9'}; // rect bounds from x40 y80 to x204 y 104
    static const char layer2[10] = {'Q','W','E','R','T','Y','U','I','O','P'}; // x40 to y104 x204 y126
    static const char layer3[9] = {'A','S','D','F','G','H','J','K','L'}; //x48 y128 to x198 y148
    static const char layer4[7] = {'Z','X','C','V','B','N','M'}; //x64 y 152 to x180 y170

    char keyPressed[3];

   // if (ctx->input-> buttonsUp & KEY_TOUCH)
   // {
        int  tX = ctx->input->touchPos.px;
        int tY = ctx->input->touchPos.py;

        if (tX > 40 && tX < 204 && tY > 80 && tY < 104)
        {
            nocashMessage("layer 1");
            char key[2];
            key[0] = layer1[(tX - 41) / 16];
            //key[1] = '\0';
            //nocashMessage(key);
            return key[0];

        }

        else if (tX > 40 && tX < 204 && tY > 104 && tY < 126)
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

        else if (tX > 48 && tX < 198 && tY > 128 && tY < 148)
        {
            nocashMessage("layer 3");
            //64,80
            //algo is (tx - 64) / 16
            //+ 16????????
            // literally no clue why i have to add 16 but we ball
            char key[2];
            key[0] = layer3[(tX - 48) / 16];
            //key[1] = '\0';
            //nocashMessage(key);
            return key[0];
        }

        else if (tX > 64 && tX < 180  && tY > 152 && tY < 170)
        {
            //80,96
            //algo is (tx - 80) /16 mysterious +16 offset strikes again

            nocashMessage("layer 4");
            char key[2];
            key[0] = layer4[(tX - 64) / 16];
            //key[1] = '\0';
            //nocashMessage(key);
            return key[0];
        }

        else if (tX > 37 && tX < 218 && tY > 168 && tY < 191)
        {
            //nocashMessage("space");
            return ' ';
        }

        else if ((tX > 203 && tX < 218 && tY > 80 && tY < 146) || (tX > 195 && tX < 218 && tY > 125 && tY < 146))
        {
            //203 80 218 146
            //195 125
            //nocashMessage("return");
            return '+';
        }

        else if (tX > 180 && tX < 217 && tY > 147 && tY < 168)
        {
            //180 147, 217 168
            //nocashMessage("backspace");
            return '-';
        }
    //}
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

    static int letterPos = 0;
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
    }
    nocashMessage(string);
    char buffer[16];
    snprintf(buffer,sizeof(buffer),"%d",letterPos);
    nocashMessage(buffer);
    displayTyped(string,maxChars);
    return '?';



}

