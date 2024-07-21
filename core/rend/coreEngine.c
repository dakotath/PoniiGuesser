// coreEngine.c (C)2024 Dakota Thorpe.
/**
 * @file coreEngine.c
 * @copyright &copy; 2024
 * @author Dakota Thorpe.
 * Core Engine base functions.
*/
// Standard Libs.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Wii Specific.
#include <asndlib.h>
#include <wiiuse/wpad.h>
#include <wiisocket.h>
#include <gctypes.h>
#include <gccore.h>
#include <fat.h>

// Our #1 graphics library.
#include <grrlib.h>

// includes
#include "rend/coreEngine.h"
#include "misc/carhorn_defs.h"

// Error bg.
#include "err_jpg.h"

// Font.
#include "font_ttf.h"

/**
 * @author Dakota Thorpe
 * @paragraph APPINIT_P0 Initializes Wii Hardware.
*/
void CoreEngine_Init() {
    // Init audio.
    ASND_Init();

    // Init Graphics.
    GRRLIB_Init();

    // Init Network.
    wiisocket_init();

    // Init Wii Remote.
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR); // Set the WPAD API IR format. Only call on startup.

    // Init SD card.
    s32 fat = fatInitDefault();
    if(fat != 1) {
        showErrorScreen("SD Error.");
    }

    // debug Log.
    #ifdef DEBUG
        globalLog = fopen("globallog.log", "w");
        if(globalLog != NULL) canLog = true;
    #endif
}

/**
 * @author Dakota Thorpe
 * @paragraph ses_p0 Shows the error screen.
 * 
 * @param errorText The error text to be displayed.
*/
void showErrorScreen(char* errorText) {
    // ERROR bg.
    GRRLIB_texImg* error_background = GRRLIB_LoadTexture(err_jpg);
    GRRLIB_ttfFont* globalFont = GRRLIB_LoadTTF(font_ttf, font_ttf_size);

    while(true) {
        WPAD_ScanPads();
        s32 pressed = WPAD_ButtonsDown(WPAD_CHAN_0);

        // Render.
        GRRLIB_DrawImg(0,0, error_background, 0, 1,1, COL_WHITE);
        GRRLIB_PrintfTTF(0,0, globalFont, errorText, 28, COL_WHITE);
        GRRLIB_Render();

        // Check for home
        if(pressed & WPAD_BUTTON_HOME) {
            break;
        }
    }

    exit(0);
}