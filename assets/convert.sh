#!/bin/bash

BLOCKSDS="${BLOCKSDS:-/opt/blocksds/core/}"
GRIT=$BLOCKSDS/tools/grit/grit


$GRIT animatedHUD.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT tileSprites.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT player.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT highlight.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT key.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT letters.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT uiAssets.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT bottomHud.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT incrementbtn.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT return.png -ftB -fh! -gTFF00FF -gt -gB8 -m!



$GRIT HUD.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT GRID.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT font.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -m!
$GRIT tiles.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT screen0Hud.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT title.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs


mv *.pal *.img *.map ../nitrofiles/bg
