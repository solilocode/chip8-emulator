# chip8-emulator

Emulator of the Chip-8 virtual machine interpreter in modern C++.

*Installation*

1. Clone the project
2. Run `install.sh`
3. Use `./PlayChip8 <chip8-game.ch8>`
4. Whether a key is allowed (part of the emulator) is displayed in the terminal once it has been pressed

Requires SDL2 for the graphical rendering and input: https://wiki.libsdl.org/Installation

*Keyboard*

A Chip-8 game may make use of any of the following keys (see description of game, if any, for specifics):
```
1 2 3 4
Q W E R
A S D F
Z X C V
```
