#ifndef CARHORN_DEFS_H
#define CARHORN_DEFS_H

// Standard Libs.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

// Wii Specific.
#include <gctypes.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

// Our #1 graphics library.
#include <grrlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define COL_BLACK 0x000000FF
#define COL_WHITE 0xFFFFFFFF
#define COL_RED     0xFF0000FF
#define COL_GREEN   0x00FF00FF
#define COL_BLUE    0x0000FFFF
#define COL_CYAN    0x00FFFFFF
#define COL_MAGENTA 0xFF00FFFF
#define COL_YELLOW  0xFFFF00FF
#define COL_ORANGE  0xFFA500FF
#define COL_PINK    0xFFC0CBFF
#define COL_PURPLE  0x800080FF
#define COL_LIME    0xBFFF00FF
#define COL_TEAL    0x008080FF
#define COL_AQUA    0x00CED1FF
#define COL_LAVENDER 0xE6E6FAFF
#define COL_MAROON  0x800000FF
#define COL_NAVY    0x000080FF
#define COL_TURQUOISE 0x40E0D0FF
#define COL_GOLD    0xFFD700FF

#endif