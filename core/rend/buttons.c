// buttons.c - (C)2024 Dakota Thorpe.

/**
 * @file buttons.c
 * @author Dakota Thorpe
 * @copyright &copy; 2024
*/

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

// core.
#include "std_btn_png.h"
#include "rend/buttons.h"

// Log.
#ifdef DEBUG
    FILE* globalLog;
    bool canLog;
#endif

// Normal button
/**
 * @author Dakota Thorpe
 * @paragraph cb_p1 This function returns a sprited button.
 * 
 * @param x The X cordinate of the button.
 * @param y The Y cordinate of the button.
 * @param str The string that should be put inside the button.
 * @param options Internal settings for the button. (Ex: Does it have seperate textures, can you hover).
 * @param assets Internal button assets (Ex: main texture, hover texture).
 * 
 * @returns The button you just fucking created DUHHH.
*/
spritedbtn_t CreateButton(int x, int y, char* str, spritedbtn_options_t options, spritedbtn_assets_t assets) {
    spritedbtn_t newButton;

    // Log some shit.
    #ifdef DEBUG
        if(canLog) {
            fprintf(globalLog, "Creating a new button with these settings:\n");
            fprintf(globalLog, "\tPoint - X:%d, Y:%d.\n", x,y);
            fprintf(globalLog, "\tColors - Color (BG): 0x%8x. Color (Text): 0x%8x. Color (click): 0x%8x. Color (Hover): 0x%8x.\n", assets.color, assets.textColor, assets.onclickColor, assets.hoverColor);
            fprintf(globalLog, "\tSettings - canHover: %s, canPress: %s, morethan1Texture: %s.\n", tf_s(options.canHover), tf_s(options.canPress), tf_s(options.morethan1Texture));
        }
    #endif

    // Setup the point.
    Point point;
    point.x = x;
    point.y = y;
    newButton.pnt = point;
    newButton.assets = assets;
    newButton.settings = options;
    newButton.str = str;

    // Return.
    return newButton;
}

// Standard button settings.
/**
 * @author Dakota Thorpe
 * @paragraph gsba_p1 This function returns standard assets for buttons.
 * 
 * @param bgColor Background color of the normal texture.
 * @param txtColor Color of the button text.
 * @param font Font of the button.
 * 
 * @returns THE FUCKING ASSETS NULL-BRAINS!
*/
spritedbtn_assets_t GetStdBtnAssets(u32 bgColor, u32 txtColor, GRRLIB_ttfFont* font) {
    spritedbtn_assets_t stdAssets;

    // Nullify
    stdAssets.btnDownTexture = NULL;
    stdAssets.btnHoverTexture = NULL;
    stdAssets.btnTexture = NULL;
    stdAssets.font = NULL;

    stdAssets.btnTexture = GRRLIB_LoadTexture(std_btn_png);
    stdAssets.color = bgColor;
    stdAssets.textColor = txtColor;
    stdAssets.hoverColor = 0xAAAAAAFF;

    stdAssets.w = stdAssets.btnTexture->w;
    stdAssets.h = stdAssets.btnTexture->h;

    // Setup font.
    stdAssets.font = font;
    return stdAssets;
}

/**
 * @author Dakota Thorpe
 * @paragraph gsbo_p1 This function returns your standard button options.
 * 
 * @param clickFunction Callback for when the button is clicked on.
 * @param hoverFunction Callback for when you hover over the button.
 * @param fontSize Button font size.
 * 
 * @returns Apple Jack's cider.
*/
spritedbtn_options_t GetStdBtnOptions(void (*clickFunction)(int, char**), void (*hoverFunction)(int, char**), int fontSize) {
    spritedbtn_options_t stdOptions;
    stdOptions.canHover = true;
    stdOptions.canPress = true;
    stdOptions.morethan1Texture = false;
    stdOptions.clickFunction = clickFunction;
    stdOptions.hoverFunction = hoverFunction;
    stdOptions.fontSize = fontSize;
    return stdOptions;
}

/**
 * @author Dakota Thorpe.
 * @paragraph gbs_p0 This function returns the base size of your button.
 * 
 * @param btn The button to get the size of.
 * 
 * @returns Rainbow Dash.
*/
Size GetButtonSize(spritedbtn_t btn) {
    Size bSize;
    bSize.w = btn.assets.btnTexture->w;
    bSize.h = btn.assets.btnTexture->h;
    return bSize;
}

/**
 * @author Dakota Thorpe.
 * @paragraph rsb_p0 Renders the button onto the screen.
 * 
 * @param btn The button to render.
*/
void renderSpritedButton(spritedbtn_t btn) {
    int textLengthInPixels;
    int textHeightInPixels;

    int generalBtnWidth = btn.assets.btnTexture->w;
    int generalBtnHeight = btn.assets.btnTexture->h;

    textLengthInPixels = GRRLIB_WidthTTF(btn.assets.font, btn.str, btn.settings.fontSize);
    textHeightInPixels = btn.settings.fontSize;

    // Draw sprite shit.
    GRRLIB_DrawImg(btn.pnt.x,btn.pnt.y, btn.assets.btnTexture, 0, 1,1, btn.assets.color);

    // Draw other shit.
    if(btn.settings.morethan1Texture) {
        if(btn.settings.isHovering) {
            GRRLIB_DrawImg(btn.pnt.x,btn.pnt.y, btn.assets.btnHoverTexture, 0, 1,1, btn.assets.color);    
        }
        if(btn.settings.isPressed) {
            GRRLIB_DrawImg(btn.pnt.x,btn.pnt.y, btn.assets.btnDownTexture, 0, 1,1, btn.assets.color);    
        }
    } else { // No seperate textures.
        if(btn.settings.isHovering) {
            GRRLIB_DrawImg(btn.pnt.x,btn.pnt.y, btn.assets.btnTexture, 0, 1,1, btn.assets.hoverColor);    
        }
        if(btn.settings.isPressed) {
            GRRLIB_DrawImg(btn.pnt.x,btn.pnt.y, btn.assets.btnTexture, 0, 1,1, btn.assets.onclickColor);    
        }
    }

    // Place text in btn.
    int txtX = btn.pnt.x + (generalBtnWidth / 2) - (textLengthInPixels / 2);
    int txtY = btn.pnt.y + (generalBtnHeight / 2) - (textHeightInPixels / 2);

    GRRLIB_PrintfTTF(txtX, txtY, btn.assets.font, btn.str, btn.settings.fontSize, btn.assets.textColor);
}

/**
 * @author Dakota Thorpe.
 * @paragraph cbs_p0 This function preforms checks on the button, and handles calling it's callbacks.
 * 
 * @param btn The button to check.
 * @param mx Current mouse X position.
 * @param my Current mouse Y position.
 * @param cSize The area of space that your cursor takes up.
 * @param pressed The pressed state of the Wii Remote.
*/
void checkButtonStatus(spritedbtn_t* btn, int mx, int my, Size cSize, s32 pressed) {
    // Calculate the boundaries of the cursor rectangle
    int cursorLeft = mx;
    int cursorRight = mx + cSize.w;
    int cursorTop = my;
    int cursorBottom = my + cSize.h;

    // Calculate the boundaries of the button
    int btnLeft = btn->pnt.x;
    int btnRight = btn->pnt.x + btn->assets.w;
    int btnTop = btn->pnt.y;
    int btnBottom = btn->pnt.y + btn->assets.h;

    // Check for collision (if any part of the cursor is touching the button)
    if (cursorRight >= btnLeft && cursorLeft <= btnRight &&
        cursorBottom >= btnTop && cursorTop <= btnBottom)
    {
        btn->settings.isHovering = true;
        if(!btn->settings.hasHovered) {
            btn->settings.hasHovered = true;
            
            // Call hover function.
            if(btn->settings.hoverFunction != NULL) {
                btn->settings.hoverFunction(0, NULL);

                // Debug log.
                #ifdef DEBUG
                    if(canLog) {
                        fprintf(globalLog, "User hovered over a buton with text: \"%s\".\n", btn->str);
                    }
                #endif
            }
        }
    } else {
        btn->settings.isHovering = false;
        btn->settings.hasHovered = false;
    }

    // Check for collision and press (if any part of the cursor is touching the button)
    if (cursorRight >= btnLeft && cursorLeft <= btnRight &&
        cursorBottom >= btnTop && cursorTop <= btnBottom &&
        btn->settings.hasPressed == false && (pressed & WPAD_BUTTON_A))
    {
        btn->settings.isPressed = true;
        if(!btn->settings.hasPressed) {
            btn->settings.hasPressed = true;
            
            // Call press function.
            if(btn->settings.clickFunction != NULL) {
                btn->settings.clickFunction(btn->settings.argc, btn->settings.argv);

                // Debug log.
                #ifdef DEBUG
                    if(canLog) {
                        fprintf(globalLog, "User clicked a buton with text: \"%s\".\n", btn->str);
                    }
                #endif
            }
        }
    } else {
        btn->settings.isPressed = false;
        btn->settings.hasPressed = false;
    }
}

// True False String.
char* tf_s(bool i) {
    if(i == true) {
        return "true";
    } else {
        return "false";
    }
}