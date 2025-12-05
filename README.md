# NES Art Maker 

A small NES art program built using cc65, running on a simple NROM-256 ROM layout.
It includes a canvas you can draw on with multiple tools, a palette system with blue/green/red regions, and a random noise generator.

## Features:
  Cursor-based drawing system
  
  Tools: brush, eraser, line, rectangle, flood fill
  
  Multi-palette support (blue, green, red regions at same time)
  Random noise generator (A+B combo)
  
  Canvas + UI bar built with CHR tiles

## Files  
nes.cfg straight from cc65 sample nes.cfg

crt0.s  startup code / iNES header, based on cc65 NES startup examples

main.c Main game logic

ppu.c / ppu.h

  Low-level PPU helpers.
  
  Functions for:
    Waiting for vblank
    
  Loading background palette

  Writing to VRAM
    
  Clearing OAM
    
  Drawing the cursor sprite
  
tiles.s

  Pattern table graphics.
  Contains the actual 8×8 CHR tile data for:
  
  Canvas colors
  
  UI elements
  
  Cursor sprite
  
input.c / input.h

  Joypad wrapper
  Reads $4016, stores previous frame state, and exposes helpers like input_pressed()
  
canvas.c / canvas.h

  g_canvas tile buffer
  
  Pixel setters
  
  Tile redraw functions
  
  Full 32×30 nametable upload
  
  Attribute table builder for per-region palettes

## Build Instructions
Install cc65 through https://cc65.github.io/getting-started.html

Run in terminal:

cl65 -t nes -C nes.cfg -o game.nes crt0.s main.c ppu.c input.c canvas.c tiles.s

Or through provided Makefile or build.ps1 with Windows through Powershell

Or simply install it in the "bin" folder
Load game.nes into emulator, preferably Mesen

Can be played on original NES hardware with a flash cartridge. 

Simply burn the game.nes file into SD card, then insert SD card into flash cart, and insert flast cart into NES.

## Controls
D-Pad:	Move cursor

A: Use current tool

B:	Cancel drag (line/rect)

START:	Cycle tools

SELECT:	Cycle brush colors (blue → green → red)

A + B:	Generate random noise

START + SELECT:	Clear drawing area

UI: Top two rows, top left shows color palette, squares on the second row to the right show the tool. Dark: brush, No square: eraser, Medium: Line, Light: Rectangle, Dark: Fill

## Postmortem
As someone with no familiarity with 6502 assembly, choosing to make a program in this language was not the smartest idea. Creating something with any artistic substance with my lack of knowledge was a huge hurdle to go over in the few weeks we had to make this. It is impressive to me what I was able to do, but from an outsider's perspective, my program might seem simple and lacking. A  majority of my time was spent researching and reading documentation on how NES programs are to be setup. Once I got the C code and main game logic, it was not so bad. Many bugs that arose were not due to my code's logic, but due to the limitations on NES hardware. Read the Doc at the bottom to see the list of challenges I faced because of the limitations of the NES. If I were to do this again, I would choose not to place such a heavy restriction on myself with the limited time I had. If there were more time to do this, I could have made something with more substance and fun that would look like an actual game. I should have chosen a more modern console that is easier to learn or that I have more knowledge of.

## Artistic Statement
I chose to make this paint program on the NES because I have always been fascinated by old computer hardware. The bare bones of what a computer is seem simple, but there is so much depth and complexity to what they can achieve, even with all their slowness and low memory. I have always enjoyed mechanical systems, such as the ones in cars and bicycles, and making a game on a simple computer, such as the NES, made me feel how I do when I work on my car or bicycle. I had to truly understand how the hardware worked on the NES to solve the problems I faced when making this game. Also, I always wanted to make an NES game since I was a kid because I thought the console was "super cool" and was one of my first introductions to gaming. I am somewhat happy with what I made, but I felt like I could have done more.

## Credits
Compiler: https://cc65.github.io/

Information on hardware and NES developer community: https://www.nesdev.org/

Useful videos on setting up development environment plus hardware explanations: https://www.youtube.com/@NesHacker/videos

### Notes on challenges I faced
https://docs.google.com/document/d/1ZmzFfWVHCw7YIlpXInaJjBO7U9oi_nbgsbmdkurYeyQ/edit?usp=sharing
