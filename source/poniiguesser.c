// carhorn.c - (c)2024 Dakota Thorpe.

// Standard Libs.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>

// Wii Specific.
#include <asndlib.h>
#include <gctypes.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>

// Our #1 graphics library.
#include <grrlib.h>
#include <pngu.h>

// OGG Player lib.
#include "oggplayer.h"

// core.
#include "rend/osk.h"
#include "rend/audio.h"
#include "rend/buttons.h"
#include "rend/coreEngine.h"
#include "misc/carhorn_defs.h"

// Built in Data.
#include "font_ttf.h"

#include "std_arrow_left_png.h"
#include "std_arrow_right_png.h"
#include "std_btn_png.h"
#include "cursor_png.h"
#include "wiibg_jpg.h"

#include "button_over_pcm.h"
#include "button_click_pcm.h"

#include "app_funcs.h"
#include "misc/utils.h"
#include "misc/networking.h"

// Main loop callback vars.
bool readytoGuess = false;
bool yesClicked = false;
bool noClicked = false;

// Other screens.
void gameStatus(GRRLIB_texImg* guessedImg, response_t answer);

// Exit function.
void onclickExit(int argc, char** argv) {
    exit(0);
}

// Yesno
void onYesClick(int argc, char** argv) {
    yesClicked = true; // Set the callback variable.
}
void onNoClick(int argc, char** argv) {
    noClicked = true; // Set the callback variable.
}


// Guess press.
void onGuessClick(int argc, char** argv) {
    readytoGuess = true; // Set the callback variable.
}

// Main code.
int main(int argc, char** argv)
{
    // Init.
    CoreEngine_Init();
    Networking_Init();

    // Variables.
    ir_t ir;
    GRRLIB_ttfFont* globalFont;

    // Font.
    globalFont = GRRLIB_LoadTTF(font_ttf, font_ttf_size);

    // Cursor
    GRRLIB_texImg* cursor = GRRLIB_LoadTexture(cursor_png);
    Size cSize;
    cSize.w = cursor->w;
    cSize.h = cursor->h;

    // Exit button.
    spritedbtn_t exitBtn = CreateButton(0,1, "Exit", GetStdBtnOptions(onclickExit,NULL, 20), GetStdBtnAssets(COL_WHITE, COL_BLACK, globalFont));
    const Size exitBtnSize = GetButtonSize(exitBtn);
    exitBtn.pnt.y = SCREEN_HEIGHT - exitBtnSize.h; // Place exit button at bottom left.

    // Guess button.
    spritedbtn_t guessBtn = CreateButton(0,1, "Guess", GetStdBtnOptions(onGuessClick,NULL, 20), GetStdBtnAssets(COL_GREEN, COL_BLACK, globalFont));
    const Size guessBtnSize = GetButtonSize(guessBtn);
    guessBtn.pnt.x = SCREEN_WIDTH - guessBtnSize.w; // Place guess button at bottom r.
    guessBtn.pnt.y = SCREEN_HEIGHT - guessBtnSize.h; // Place guess button at bottom r.

    // Yes/No button
    spritedbtn_t yesBtn = CreateButton(0,1, "Yes", GetStdBtnOptions(onYesClick,NULL, 20), GetStdBtnAssets(COL_GREEN, COL_BLACK, globalFont));
    spritedbtn_t noBtn = CreateButton(0,1, "No", GetStdBtnOptions(onNoClick,NULL, 20), GetStdBtnAssets(COL_RED, COL_BLACK, globalFont));
    const Size yesBtnSize = GetButtonSize(yesBtn);
    const Size noBtnSize = GetButtonSize(noBtn);
    yesBtn.pnt.x = 0;
    yesBtn.pnt.y = SCREEN_HEIGHT - yesBtnSize.h;
    noBtn.pnt.x = SCREEN_WIDTH - noBtnSize.w;
    noBtn.pnt.y = SCREEN_HEIGHT - noBtnSize.h;

    int season = 0;//getNumInput("Select the Season guess:");
    int episode = 0;//getNumInput("Select the Episode guess:");

    // Get an image ID.
    char* imgId = get_image_id(); // Prone to fuck up.

    // Download the image.
    downloadImage(imgId);

    // Try to load in the frame.
    FILE* frameLoad = fopen("sd:/ponyFrame.png", "rb");
    if(frameLoad == NULL) {
        showErrorScreen("FILE* frameLoad was NIL");
    }

    // Try to load it into ram.
    int frmSize;
    fseek(frameLoad, 0, SEEK_END);
    frmSize = ftell(frameLoad);
    fseek(frameLoad, 0, SEEK_SET);

    // Create a buffer for it.
    void* frmBuff = malloc(frmSize);
    if(frmBuff == NULL) {
        showErrorScreen("Could not allocate buffer for frame.");
    }

    // Read into buffer.
    fread(frmBuff, frmSize, 1, frameLoad);

    // Close file.
    fclose(frameLoad);

    // Quick fix for grrlib issue.
    PNGUPROP imgProp;
    IMGCTX ctx = PNGU_SelectImageFromBuffer(frmBuff);
    PNGU_GetImageProperties(ctx, &imgProp);
    int pngW,pngH;
    pngW = imgProp.imgWidth;
    pngH = imgProp.imgHeight;
    GRRLIB_texImg *my_texture = calloc(1, sizeof(GRRLIB_texImg));
    if (my_texture == NULL) {
        return NULL;
    }
    my_texture->data = PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, &pngW, &pngH);
    if (my_texture->data != NULL) {
        my_texture->w = pngW;
        my_texture->h = pngH;
        //my_texture->format = GX_TF_RGBA8;
        GRRLIB_SetHandle( my_texture, 0, 0 );
        if (imgProp.imgWidth != pngW || imgProp.imgHeight != pngH) {
            // PNGU has resized the texture
            //memset(my_texture->data, 0, (my_texture->h * my_texture->w) << 2);
        }
        GRRLIB_FlushTex( my_texture );
    }
    PNGU_ReleaseImageContext(ctx);

    // Free old buffer?
    free(frmBuff);
    
    // Main Loop.
    while(true)
    {
        // Scan for button presses.
        WPAD_ScanPads();
        WPAD_IR(WPAD_CHAN_0, &ir);

        s32 pressed = WPAD_ButtonsDown(WPAD_CHAN_0);

        // Check for home button press.
        if(pressed & WPAD_BUTTON_HOME)
        {
            break;
        }

        // Pony Frame
        // Calculate position
        int frmX = (SCREEN_WIDTH / 2) - ((my_texture->w * 0.5) / 2);
        int frmY = (SCREEN_HEIGHT / 2) - ((my_texture->h * 0.5) / 2);
        GRRLIB_DrawImg(frmX,frmY, my_texture, 0, 0.5,0.5, COL_WHITE);

        // Welcome
        GRRLIB_PrintfTTF(0,0, globalFont, "Welcome to PONiiGuesser Wii!", 30, COL_WHITE);

        // Exit button.
        renderSpritedButton(exitBtn);
        checkButtonStatus(&exitBtn, ir.x,ir.y, cSize, pressed);

        // Guess button.
        renderSpritedButton(guessBtn);
        checkButtonStatus(&guessBtn, ir.x,ir.y, cSize, pressed); 

        // update.
        GRRLIB_DrawImg(ir.x,ir.y, cursor, 0, 1,1, 0xFFFFFFFF);
        GRRLIB_Render();

        // Main loop callback checks.
        if(readytoGuess) {
            // Get input
            season = getNumInput("Select the Season guess:");
            episode = getNumInput("Select the Episode guess:");
            response_t isCorr;

            // Confirm.
            while (true)
            {
                WPAD_ScanPads();
                WPAD_IR(WPAD_CHAN_0, &ir);

                pressed = WPAD_ButtonsDown(WPAD_CHAN_0);

                // Image.
                GRRLIB_DrawImg(frmX,frmY, my_texture, 0, 0.5,0.5, COL_WHITE);

                // Confirm text.
                char* confirmText = malloc(100);
                sprintf(confirmText, "You guessed: S%dE%d. Is this your final?", season, episode);
                GRRLIB_PrintfTTF(0,0, globalFont, confirmText, 30, COL_WHITE);
                free(confirmText);

                // Yes/No button.
                renderSpritedButton(yesBtn);
                renderSpritedButton(noBtn);
                checkButtonStatus(&yesBtn, ir.x,ir.y, cSize, pressed); 
                checkButtonStatus(&noBtn, ir.x,ir.y, cSize, pressed);

                // Render.
                GRRLIB_DrawImg(ir.x,ir.y, cursor, 0, 1,1, 0xFFFFFFFF);
                GRRLIB_Render();

                if(noClicked) {
                    season = getNumInput("Select the Season guess:");
                    episode = getNumInput("Select the Episode guess:");
                }

                if(yesClicked) {
                    isCorr = checkCorrect(season, episode, imgId);
                    break;
                }

                // Reset vars.
                yesClicked = false;
                noClicked = false;
            }

            // Answer.
            gameStatus(my_texture, isCorr);
        }

        // Reset callback vars.
        readytoGuess = false;
        yesClicked = false;
        noClicked = false;
    }
}

void gameStatus(GRRLIB_texImg* guessedImg, response_t answer) {
    // Vars.
    ir_t ir;
    GRRLIB_ttfFont* globalFont;

    // Font.
    globalFont = GRRLIB_LoadTTF(font_ttf, font_ttf_size);

    // The waiting BG.
    GRRLIB_texImg* bgImg = GRRLIB_LoadTexture(wiibg_jpg);

    // Cursor
    GRRLIB_texImg* cursor = GRRLIB_LoadTexture(cursor_png);
    Size cSize;
    cSize.w = cursor->w;
    cSize.h = cursor->h;

    // Frame pos
    int frmX = (SCREEN_WIDTH / 2) - ((guessedImg->w * 0.5) / 2);
    int frmY = (SCREEN_HEIGHT / 2) - ((guessedImg->h * 0.5) / 2);

    // Yes/No button
    spritedbtn_t yesBtn = CreateButton(0,1, "Yes", GetStdBtnOptions(onYesClick,NULL, 20), GetStdBtnAssets(COL_GREEN, COL_BLACK, globalFont));
    spritedbtn_t noBtn = CreateButton(0,1, "No", GetStdBtnOptions(onNoClick,NULL, 20), GetStdBtnAssets(COL_RED, COL_BLACK, globalFont));
    const Size yesBtnSize = GetButtonSize(yesBtn);
    const Size noBtnSize = GetButtonSize(noBtn);
    yesBtn.pnt.x = 0;
    yesBtn.pnt.y = SCREEN_HEIGHT - yesBtnSize.h;
    noBtn.pnt.x = SCREEN_WIDTH - noBtnSize.w;
    noBtn.pnt.y = SCREEN_HEIGHT - noBtnSize.h;

    // Main Loop.
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

        // Image.
        GRRLIB_DrawImg(frmX,frmY, guessedImg, 0, 0.5,0.5, COL_WHITE);

        // Screen code.
        if(answer.correct == true) {
            GRRLIB_PrintfTTF(0,0, globalFont, "You were Correct!", 27, COL_WHITE);
        } else {
            GRRLIB_PrintfTTF(0,0, globalFont, "You were wrong.", 27, COL_WHITE);
        }

        // More info.
        char* fInfo = malloc(100);
        sprintf(fInfo, "S%dE%d, Seek: %.6f, ETS: %d.", answer.season,answer.episode, answer.seekTime, answer.expiryTs);
        GRRLIB_PrintfTTF(0,30, globalFont, fInfo, 27, COL_WHITE);
        free(fInfo);
    
        // Play again.
        GRRLIB_PrintfTTF(0,30+27, globalFont, "Want to play again?", 27, COL_WHITE);

        // Render buttons
        renderSpritedButton(yesBtn);
        renderSpritedButton(noBtn);
        checkButtonStatus(&yesBtn, ir.x,ir.y, cSize, pressed);
        checkButtonStatus(&noBtn, ir.x,ir.y, cSize, pressed);
        
        // update.
        GRRLIB_DrawImg(ir.x,ir.y, cursor, 0, 1,1, 0xFFFFFFFF);
        GRRLIB_Render();

        // Reset vars.
        yesClicked = false;
        noClicked = false;
    }
}