// buttons.h - (C)2024 Dakota Thorpe.

// Standard Libs.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Wii Specific.
#include <gctypes.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

// Our #1 graphics library.
#include <grrlib.h>

#ifndef BUTTONS_H
#define BUTTONS_H

// Button structures.
typedef struct {
    int x; // X Cord
    int y; // Y Cord
} Point;

typedef struct {
    int w; // Width
    int h; // Height
} Size;

// Button settings.
/**
 * @param canHover can you hover? no? I didn't fucking think so.
 * @param canPress I'm gonna touch you.
 * @param morethan1Texture For when even ***you*** need friends.
 * @param clickFunction The function called when you click the button.
 * @param hoverFunction Must I need to FUCKING EXPLAIN THIS ONE?
 * @param hasHovered Has the user hovered over the button? Prevents API spamming.
 * @param hasPressed Same as the one above me, but for pressing :3
 * @param isHovering WOAHHH DUDE IM TRIPPIN BALLZ OVER HERE!!!
 * @param isPressed He touched you.
 * @param argc We're gonna fucking argue for X amount of times.
 * @param argv YOU FORGOT TO TAKE OUT THE FUCKING TRASH YOU IDIOT.
 * @param fontSize Font size.
*/
typedef struct
{
    bool canHover;                      // Enables hover texture.
    bool canPress;                      // Enables onclick texture.
    bool morethan1Texture;              // Disables seperate BG colors and uses seperate textures.
    void (*clickFunction)(int, char**); // Click function.
    void (*hoverFunction)(int, char**); // Hover function.
    bool hasHovered;                    // Prevents hover funtion spamming.
    bool hasPressed;                    // Prevents press funtion spamming.
    bool isHovering;
    bool isPressed;
    bool disabled;
    int argc;
    char** argv;
    int fontSize;
} spritedbtn_options_t;

// Button assets.
/**
 * @param btnTexture The normal button texture.
 * @param btnHoverTexture The texture on hover (optional).
 * @param btnDownTexture The texture while held down (optional).
 * 
 * @param color Normal main texture color.
 * @param hoverColor Color of main texture on hover.
 * @param onclickColor Color of main texture when held down.
 * @param textColor Color of the text in the button.
 * 
 * @param w Width of button.
 * @param h Height of button.
*/
typedef struct
{
    GRRLIB_texImg* btnTexture;          // Button Texture
    GRRLIB_texImg* btnHoverTexture;     // Button Hover Texture (optional)
    GRRLIB_texImg* btnDownTexture;      // Button Pressed Texture (optional)
    GRRLIB_ttfFont* font;               // Button text font.
    u32 color;          // Color (BG)
    u32 hoverColor;     // Hover color (optional)
    u32 onclickColor;   // Press color (optional)
    u32 textColor;      // Color (FG)
    int w;
    int h;
} spritedbtn_assets_t;

// Customizable button.
/**
 * @param pnt The point where the button is placed at.
 * @param settings The settings of the button.
 * @param assets The buttons required assets.
 * @param str The string to be put in the button.
*/
typedef struct
{
    Point pnt; // Position.
    spritedbtn_options_t settings;  // Button settings
    spritedbtn_assets_t assets;     // Button assets.
    char* str;
} spritedbtn_t;

// Functions.
spritedbtn_t            CreateButton(int x, int y, char* str, spritedbtn_options_t options, spritedbtn_assets_t assets);
spritedbtn_assets_t     GetStdBtnAssets(u32 bgColor, u32 txtColor, GRRLIB_ttfFont* font);
spritedbtn_options_t    GetStdBtnOptions(void (*clickFunction)(int, char**), void (*hoverFunction)(int, char**), int fontSize);
Size                    GetButtonSize(spritedbtn_t btn);

void renderSpritedButton(spritedbtn_t btn);
void checkButtonStatus(spritedbtn_t* btn, int mx, int my, Size cSize, s32 pressed);

char* tf_s(bool i); // Returns a bool as a string.

#endif