# You Make! DS

# What is this?
You Make! DS is a homebrew game for the Nintendo DS inspired by Mario Maker. 

It features a level editor where the limit is your creativity (as long as your creativity fits within 250 thousand tiles), a way to play your level, as well as saving your level and being able to load it back up when you feel like working on it again!

It also features a way to share levels locally that is currently in development but an early version of it is currently shipped with the game.

This project taught me a ton about programming for the DS, as well as the fact that it made me write better C code and expose myself with different parts of the language that I previously was too scared to try.

# How to play on a DS

Download the .nds file found in the Releases tab, and put it in a flashcart of choice. Make sure that it is dldi patched if not already

# How to play on an emulator

I have included the windows release of melonDS with a config file that contains an existing empty SD card image as well as configured controls

The control scheme is [e](images/controls.png)
# Libraries used
This project is entirely written in C using blocksDS as the main development library which contains DSWifi, the wifi library I used, and NFLib as the graphics library

# License
This game is licensed under the MIT License

