/**
 * @file osk.c
 * @author Dakota Thorpe
 * @copyright &copy;2024
 * This provides on screen keyboard functionality for coreEngine.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <grrlib.h>
#include <asndlib.h>
#include <math.h>

#include "rend/audio.h"
#include "rend/buttons.h"
#include "rend/osk.h"
#include "misc/carhorn_defs.h"

#include "font_ttf.h"

#include "cursor_png.h"
#include "std_key_png.h"
#include "std_arrow_left_png.h"
#include "std_arrow_right_png.h"
#include "std_arrow_up_png.h"
#include "std_arrow_down_png.h"
#include "wiibg_jpg.h"

#include "button_click_pcm.h"
#include "button_over_pcm.h"

int __osk_num_validInputs;
int __osk_num_selecInput;

void __osk_hoverFunction(int argc, char** argv) {
    playSfx(button_over_pcm, button_over_pcm_size, ASND_GetFirstUnusedVoice());
}

void __osk_num_onRightArrow(int argc, char** argv) {
    // Play onclick sound.
    playSfx(button_click_pcm, button_click_pcm_size, ASND_GetFirstUnusedVoice());

    // Increment selected char.
    //if(__osk_num_selecInput < __osk_num_validInputs-1) {
    __osk_num_selecInput++;
}

void __osk_num_onLeftArrow(int argc, char** argv) {
    // Play onclick sound.
    playSfx(button_click_pcm, button_click_pcm_size, ASND_GetFirstUnusedVoice());

    // Increment selected char.
    if(__osk_num_selecInput > 0) {
        __osk_num_selecInput--;
    } else {
        __osk_num_selecInput = 0;
    }
}

// Number input.
int getNumInput(char* msg) {
    // valid shi
    //char validEntrys[] = {'0','1','2','3','4','5','6','7','8','9'};
    //int validInputs = sizeof(validEntrys);
    //__osk_num_validInputs = validInputs;

    // Infared
    ir_t ir;

    // Cursor
    GRRLIB_texImg* cursor = GRRLIB_LoadTexture(cursor_png);
    Size cSize;
    cSize.w = 1;
    cSize.h = cursor->h;

    // Background image.
    GRRLIB_texImg* bgImg = GRRLIB_LoadTexture(wiibg_jpg);

    // Font.
    GRRLIB_ttfFont* globalFont = GRRLIB_LoadTTF(font_ttf, font_ttf_size);

    // Create an incremental key.
    spritedbtn_t keybtn = CreateButton(0,0, "BN", GetStdBtnOptions(NULL, __osk_hoverFunction, 20), GetStdBtnAssets(COL_WHITE, COL_WHITE, globalFont));
    spritedbtn_t leftBtn = CreateButton(0,0, "", GetStdBtnOptions(__osk_num_onLeftArrow, __osk_hoverFunction, 20), GetStdBtnAssets(COL_WHITE, COL_WHITE, globalFont));
    spritedbtn_t rightBtn = CreateButton(0,0, "", GetStdBtnOptions(__osk_num_onRightArrow, __osk_hoverFunction, 20), GetStdBtnAssets(COL_WHITE, COL_WHITE, globalFont));

    // Change key texture.
    keybtn.assets.btnTexture = GRRLIB_LoadTexture(std_key_png);
    keybtn.assets.w = keybtn.assets.btnTexture->w;
    keybtn.assets.h = keybtn.assets.btnTexture->h;

    // reposition main key.
    Size keyBtnSize = GetButtonSize(keybtn);
    keybtn.pnt.x = (SCREEN_WIDTH / 2) - (keyBtnSize.w / 2);
    keybtn.pnt.y = (SCREEN_HEIGHT / 2) - (keyBtnSize.h / 2);

    // Retexture the side keys.
    leftBtn.assets.btnTexture = GRRLIB_LoadTexture(std_arrow_left_png);
    leftBtn.assets.w = leftBtn.assets.btnTexture->w;
    leftBtn.assets.h = leftBtn.assets.btnTexture->h;
    rightBtn.assets.btnTexture = GRRLIB_LoadTexture(std_arrow_right_png);
    rightBtn.assets.w = rightBtn.assets.btnTexture->w;
    rightBtn.assets.h = rightBtn.assets.btnTexture->h;

    // Position the side keys.
    /* Left key */
    leftBtn.pnt.x = ((SCREEN_WIDTH/2)-(leftBtn.assets.w/2)) - keybtn.assets.w;
    leftBtn.pnt.y = keybtn.pnt.y;
    /* Right key */
    rightBtn.pnt.x = ((SCREEN_WIDTH/2)-(rightBtn.assets.w/2)) + keybtn.assets.w;
    rightBtn.pnt.y = keybtn.pnt.y;

    // Reset vars.
    __osk_num_selecInput=0;

    // main loop
    while(true) {
        // Scan for button presses.
        WPAD_ScanPads();
        WPAD_IR(WPAD_CHAN_0, &ir);
        s32 pressed = WPAD_ButtonsDown(WPAD_CHAN_0);

        // Check for home button press.
        if(pressed & WPAD_BUTTON_HOME)
        {
            break;
        }

        // Draw BG.
        GRRLIB_DrawImg(0,0, bgImg, 0, 1,1, COL_WHITE);

        // Draw the message.
        GRRLIB_PrintfTTF(0,0, globalFont, msg, 28, COL_WHITE);

        // Change selected number text.
        char* selKey = malloc(100);
        //sprintf(selKey, "%c", validEntrys[__osk_num_selecInput]);
        sprintf(selKey, "%d", __osk_num_selecInput);
        keybtn.str = selKey;

        // Render main key button.
        renderSpritedButton(keybtn);
        renderSpritedButton(leftBtn);
        renderSpritedButton(rightBtn);

        // Check keys.
        checkButtonStatus(&keybtn, ir.x,ir.y, cSize, pressed);
        checkButtonStatus(&leftBtn, ir.x,ir.y, cSize, pressed);
        checkButtonStatus(&rightBtn, ir.x,ir.y, cSize, pressed);

        // Render.
        GRRLIB_DrawImg(ir.x,ir.y, cursor, 0, 1,1, COL_WHITE);
        GRRLIB_Render();

        // Free allocated mem.
        free(selKey);
    }
    return __osk_num_selecInput;
}