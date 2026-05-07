// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: NightFox & Co., 2009-2011
//
// Basic text example.
// http://www.nightfoxandco.com

#include <stdio.h>

#include <nds.h>
#include <filesystem.h>

#include <nf_lib.h>

int main(int argc, char **argv)
{
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


    NF_InitTextSys(0);          
    NF_LoadTextFont("fnt/default", "normal", 256, 256, 0); 
    NF_CreateTextLayer(0, 0, 0, "normal");

    
    NF_WriteText(0, 0, 10, 10, "Hello macondo!");

    NF_UpdateTextLayers();

    while (1)
    {


        NF_UpdateTextLayers();
        swiWaitForVBlank();
    }

    return 0;
}