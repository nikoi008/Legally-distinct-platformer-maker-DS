#include "types.h"
#include <nf_lib.h>
#include "change_palette.h"
#include "editor.h"
#include "keyboard.h"
#include "globals.h"

void setGreyBg(bool set)
{
    if (set)
    {
        for (int x = 0; x < 512 / 8; x++)
        {
            for (int y = 0; y < 512/ 8; y++)
            {
                NF_SetTileOfMap(1, TILE_LAYER, x,y,TRANSPARENT_BLOCK_OFFSET + TRANSPARENT_BLOCK_OFFSET); //changing bg registers easier?
            }
        }
    }
    else
    {
        for (int x = 0; x < 512 / 8; x++)
        {
            for (int y = 0; y < 512/ 8; y++)
            {
                NF_SetTileOfMap(1, TILE_LAYER, x,y,0); //changing bg registers easier?
            }
        }
    }

}


void showChangePalWindow(bool show)
{
    if (show)
    {
        for (int i = 0; i < 4; i++)
        {
            NF_ShowSprite(1, 18 + i, true);
            NF_ShowSprite(1, 18 + 4 + i, true);
            NF_ShowSprite(1, 18 + 8 + i, true);
            NF_MoveSprite(1, 18 + i, 96 + (16 * i), 48);
            NF_MoveSprite(1, 18 + i + 4, 96 + (16 * i), 80);
            NF_MoveSprite(1, 18 + i + 8, 96 + (16 * i), 112);

        }
        for (int i = 0; i < 16; i++)
        {
            NF_MoveSprite(1,45 +i,8 * i,0);
        }
    }
    else
    {
        for (int i = 0; i < 12; i++)
        {
            NF_MoveSprite(1,i + 18,300,300);

        }

        for (int i = 0; i < 16; i++)
        {
            NF_MoveSprite(1,45 +i,300,300);
        }
    }
    NF_ShowSprite(1,39,show);
    NF_ShowSprite(1,40,show);
    NF_ShowSprite(1,41,show);
    NF_ShowSprite(1,42,show);
    NF_ShowSprite(1,43,show);
    NF_ShowSprite(1,44,show);

    NF_ShowSprite(1,63,show);
    NF_ShowSprite(1,64,show);
    setGreyBg(show);

}

int clampInt(int val,int min, int max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void changePaletteFrame(gameContext *ctx)
{
    NF_WriteText(0,0,0,0,"Preview");
    static bool gridHidden = false;
    static int R = 9;
    static int G = 19;
    static int B = 28;

    if (ctx->input->buttonsDown & KEY_TOUCH || ctx->input->buttonsHeld & KEY_TOUCH)
    {
        touchRead(&ctx->input->touchPos);

    }
    if (ctx->input->buttonsUp & KEY_TOUCH && ctx->input->touchPos.px > 16 && ctx->input->touchPos.px < 79)
    {
        if (ctx->input->touchPos.py > 48 && ctx->input->touchPos.py < 63)
        {
            R++;
            R = clampInt(R,0, 31);
        }
        else if (ctx->input->touchPos.py > 80 && ctx->input->touchPos.py < 95)
        {
            G++;
            G = clampInt(G,0, 31);
        }
        else if (ctx->input->touchPos.py > 112 && ctx->input->touchPos.py < 127)
        {
            B++;
            B = clampInt(B, 0, 31);
        }

    }
    if (ctx->input->buttonsUp & KEY_TOUCH && ctx->input->touchPos.px > 176 && ctx->input->touchPos.px < 239)
    {
        if (ctx->input->touchPos.py > 48 && ctx->input->touchPos.py < 63)
        {
            R--;
            R = clampInt(R,0, 31);
        }
        else if (ctx->input->touchPos.py > 80 && ctx->input->touchPos.py < 95)
        {
            G--;
            G = clampInt(G,0, 31);
        }
        else if (ctx->input->touchPos.py > 112 && ctx->input->touchPos.py < 127)
        {
            B--;
            B = clampInt(B, 0, 31);
        }

    }

    if (ctx->input->touchPos.py >=  176)
    {
        if (ctx->input->touchPos.px >= 0 && ctx->input->touchPos.px <= 63 )
        {
            R = 9;
            G = 19;
            B = 28;
        }

        if (ctx->input->touchPos.px >= 192 && ctx->input->touchPos.px <= 255 )
        {

            showEditor(true);
            showChangePalWindow(false);
            state = EDITOR;
            gridHidden = false;

        }
    }
    char change[16] = "CHANGE BG COLOUR";
    for (int i = 0; i < 16; i++)
    {
        NF_SpriteFrame(1,45 + i,getLetterFrame(&change[i]));
        //NF_MoveSprite(1,45 + i,8 * i, 0);
    }


    BG_PALETTE_SUB[0] = RGB15(R, G, B);
    BG_PALETTE[0] = RGB15(R, G, B);

    char Rc[2] = "R";
    NF_SpriteFrame(1, 18, getLetterFrame(&Rc[0]));

    char rBuffer[4] = {0};
    snprintf(rBuffer, sizeof(rBuffer), "%d", R);
    if (R >= 10)
    {
        NF_SpriteFrame(1, 19  + 1, getLetterFrame(&rBuffer[0]));
        NF_SpriteFrame(1, 20 + 1, getLetterFrame(&rBuffer[1]));
    }
    else
    {
        char ZeroC[2] = "0";

        NF_SpriteFrame(1, 19  + 1, getLetterFrame(&ZeroC[0]));
        NF_SpriteFrame(1, 20  + 1, getLetterFrame(&rBuffer[0]));
    }


    char Gc[2] = "G";
    NF_SpriteFrame(1, 18 + 4, getLetterFrame(&Gc[0]));

    char gBuffer[4] = {0};
    snprintf(gBuffer, sizeof(gBuffer), "%d", G);
    if (G >= 10)
    {
        NF_SpriteFrame(1, 19 + 4  + 1, getLetterFrame(&gBuffer[0]));
        NF_SpriteFrame(1, 20 + 4  + 1, getLetterFrame(&gBuffer[1]));
    }
    else
    {
        char ZeroC[2] = "0";

        NF_SpriteFrame(1, 19 + 4  + 1, getLetterFrame(&ZeroC[0]));
        NF_SpriteFrame(1, 20 + 4  + 1, getLetterFrame(&gBuffer[0]));
    }


    char Bc[2] = "B";
    NF_SpriteFrame(1, 18 + 8, getLetterFrame(&Bc[0]));

    char bBuffer[4] = {0};
    snprintf(bBuffer, sizeof(bBuffer), "%d", B);
    if (B >= 10)
    {
        NF_SpriteFrame(1, 19 + 8 + 1, getLetterFrame(&bBuffer[0]));
        NF_SpriteFrame(1, 20 + 8 + 1, getLetterFrame(&bBuffer[1]));
    }
    else
    {
        char ZeroC[2] = "0";

        NF_SpriteFrame(1, 19 + 8 + 1, getLetterFrame(&ZeroC[0]));
        NF_SpriteFrame(1, 20 + 8 + 1, getLetterFrame(&bBuffer[0]));
    }



    char space[2] = " ";
    int frameSpace = getLetterFrame(&space[0]);
    NF_SpriteFrame(1,19,frameSpace);
    NF_SpriteFrame(1,23,frameSpace);
    NF_SpriteFrame(1,27,frameSpace);
        if (!gridHidden && state != EDITOR)
        {
            showEditor(false);
            showChangePalWindow(true);
            gridHidden = true;
        }
}