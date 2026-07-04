#include <nds.h>
#include <filesystem.h>

#include <nf_lib.h>

#include "init.h"
#include "defines.h"

void initialise(){
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);
    consoleDemoInit();
    swiWaitForVBlank();

    nitroFSInit(NULL);
    NF_SetRootFolder("NITROFS");
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);


    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(0);
    NF_InitTiledBgSys(1);

    NF_InitSpriteBuffers();
    NF_InitSpriteSys(1);

    NF_InitTextSys(0);
    NF_LoadTextFont("bg/font", "normal", 256, 256, 0);
    NF_CreateTextLayer(0, 0, 0, "normal");
    NF_DefineTextColor(0,0,BLACK,0,0,0);
    BG_PALETTE_SUB[0] = RGB15(9, 19, 28);
    BG_PALETTE[0] = RGB15(9, 19, 28);

}
