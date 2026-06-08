#!/bin/bash

BLOCKSDS="${BLOCKSDS:-/opt/blocksds/core/}"
GRIT=$BLOCKSDS/tools/grit/grit


$GRIT animatedSpriteHud.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

$GRIT HUD.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT GRID.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT tiles.png -ftB -fh! -gTFF00FF -gt -gB8 -m!


mv *.pal *.img *.map ../nitrofiles/bg
