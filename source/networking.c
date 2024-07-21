/**
 * @file networking.c
 * @author Dakota Thorpe
 * Contains the Networking code for PoniiGuesser
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <gctypes.h>
#include <grrlib.h>
#include <gccore.h>

#include <curl/curl.h>
#include "cJSON.h"

#include "rend/coreEngine.h" // For error handling.
#include "misc/carhorn_defs.h" // Colors
#include "misc/networking.h"

// Built-in-data.
#include "wiibg_jpg.h"
#include "font_ttf.h"
#include "segoe_slboot_ttf.h"

// local code defs.
lwp_t __networking_waitingThread;
bool __networking_isDoingNetworkOpetaion = false; // Should we display the please wait?
char* __networking_waitingText; // Text to be displayed in the wait screen.
int __networking_spinStart = 0xE052;
int __networking_spinEnd = 0xE0CB;
double __networking_spinCur = 0;

// Thread for waiting.
static void* __networking_waitingThreadFunc(void* arg) {
    // Make the font.
    GRRLIB_ttfFont* globalFont = GRRLIB_LoadTTF(segoe_slboot_ttf, segoe_slboot_ttf_size);

    // The waiting BG.
    GRRLIB_texImg* bgImg = GRRLIB_LoadTexture(wiibg_jpg);

    // Spinner.
    __networking_spinCur = __networking_spinStart;
    int spinnerW = 0;

    // Main loop.
    while(true) {
        if(__networking_isDoingNetworkOpetaion) {
            /* Render waiting prompt */
            // BG.
            GRRLIB_DrawImg(0,0, bgImg, 0, 1,1, COL_WHITE);

            char* loadChar = malloc(10);
            sprintf(loadChar, "%lc", (int)__networking_spinCur);
            spinnerW = GRRLIB_WidthTTF(globalFont, loadChar, 64);

            // Text.
            GRRLIB_PrintfTTF(0,0, globalFont, __networking_waitingText, 18, COL_GOLD);

            // Load.
            GRRLIB_PrintfTTF(((SCREEN_WIDTH/2) - (spinnerW/2)), ((SCREEN_HEIGHT/2) - (128/2)), globalFont, loadChar, 64, COL_WHITE);

            // Free
            free(loadChar);

            // Update spinner.
            __networking_spinCur+=0.5;
            if(__networking_spinCur >= __networking_spinEnd) {
                __networking_spinCur = __networking_spinStart;
            }

            // Render.
            GRRLIB_Render();
        }
        usleep(THREAD_SLEEP_TIME);
    }
    return NULL;
}

// Callback function to handle the response from curl
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Callback function to write received data to a file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

/**
 * @author Dakota Thorpe.
 * Starts networking threads and shit.
*/
void Networking_Init() {
    LWP_CreateThread(&__networking_waitingThread, __networking_waitingThreadFunc, NULL, NULL,0, LWP_PRIO_HIGHEST);
    LWP_SuspendThread(__networking_waitingThread);
}

/**
 * @author Dakota Thorpe.
 * Gets an Image ID from the PonyGuessr API.
*/
char *get_image_id() {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    // Set waiting text.
    __networking_isDoingNetworkOpetaion = true;
    __networking_waitingText = malloc(100);
    sprintf(__networking_waitingText, "Please Wait. Getting IID.");

    chunk.memory = malloc(1);  // will be grown as needed by realloc
    chunk.size = 0;             // no data at this point

    // Create a URL.
    char* reqUrl = malloc(1024);
    sprintf(reqUrl, "%s/resource/gen?runId&subtitles=false&audioLength=10&resourceType=frame", API_BASE);

    // Resume thread.
    LWP_ResumeThread(__networking_waitingThread);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, reqUrl);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            __networking_isDoingNetworkOpetaion = false;
            char* errText = malloc(256);
            sprintf(errText, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
            showErrorScreen(errText);
            return NULL;
        }

        // Parse JSON response using cJSON
        cJSON *root = cJSON_Parse(chunk.memory);
        if (root == NULL) {
            __networking_isDoingNetworkOpetaion = false;
            char* errText = malloc(256);
            sprintf(errText, "Error parsing JSON: %s", cJSON_GetErrorPtr());
            showErrorScreen(errText);
            return NULL;
        }

        cJSON *id = cJSON_GetObjectItem(root, "id");
        if (cJSON_IsString(id)) {
            char *image_id = strdup(id->valuestring);
            cJSON_Delete(root);
            curl_easy_cleanup(curl);
            free(chunk.memory);
            return image_id;
        } else {
            __networking_isDoingNetworkOpetaion = false;
            char* errTxt = malloc(256);
            sprintf(errTxt, "Error: 'id' field not found or not a string");
            cJSON_Delete(root);
            curl_easy_cleanup(curl);
            free(chunk.memory);
            showErrorScreen(errTxt);
            return NULL;
        }
    }

    curl_global_cleanup();
    free(reqUrl);
    free(__networking_waitingText);
    LWP_SuspendThread(__networking_waitingThread);
    __networking_isDoingNetworkOpetaion = false;
    return NULL;
}

/**
 * @author Dakota Thorpe
 * Downloads the generated frame from PonyGuessr.
 * 
 * @param iid The UUID of the generated image.
*/
void downloadImage(char* iid) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    // Set waiting text.
    __networking_isDoingNetworkOpetaion = true;
    __networking_waitingText = malloc(100);
    sprintf(__networking_waitingText, "Please wait. Downloading Pony Frame.");

    // Create the link.
    char *reqUrl = malloc(1024);
    sprintf(reqUrl, "%s/resource/get/%s.png", API_BASE, iid);

    // Output file.
    const char *output_filename = "sd:/ponyFrame.png";

    // Resume thread.
    LWP_ResumeThread(__networking_waitingThread);

    // Initialize cURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        // Open file for writing
        fp = fopen(output_filename, "wb");
        if (fp == NULL) {
            __networking_isDoingNetworkOpetaion = false;
            char* errText = malloc(256);
            sprintf(errText, "Error opening file %s", output_filename);
            showErrorScreen(errText);
            return NULL;
        }

        curl_easy_setopt(curl, CURLOPT_URL, reqUrl);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            __networking_isDoingNetworkOpetaion = false;
            char* errText = malloc(256);
            sprintf(errText, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
            showErrorScreen(errText);
            return NULL;
        }

        // Clean up
        curl_easy_cleanup(curl);
        fclose(fp);
    }

    // Cleanup global cURL
    curl_global_cleanup();
    free(__networking_waitingText);
    free(reqUrl);
    LWP_SuspendThread(__networking_waitingThread);
    __networking_isDoingNetworkOpetaion = false;
}

/**
 * @author Dakota Thorpe.
 * Checks to see if you were correct.
*/
// https://ponyguessr.com/api/resource/{imageId}/check?season={session}&episode={episode}.
// One time call. Cannot be called twice.
/*
Returns like:
{
    "season": 7,
    "episode": 20,
    "seekTime": 924.0842234100465,
    "expiryTs": 1721424912162,
    "correct": false
}
*/
response_t checkCorrect(int season, int episode, char* iid) {
    // Create a blank response.
    response_t apiResponse;

    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    // Set waiting text.
    __networking_isDoingNetworkOpetaion = true;
    __networking_waitingText = malloc(100);
    sprintf(__networking_waitingText, "Please Wait. Checking your answer.");

    chunk.memory = malloc(1);  // will be grown as needed by realloc
    chunk.size = 0;             // no data at this point

    // Create a URL.
    char* reqUrl = malloc(1024);
    sprintf(reqUrl, "%s/resource/%s/check?season=%d&episode=%d", API_BASE, iid, season, episode);

    // Resume thread.
    LWP_ResumeThread(__networking_waitingThread);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, reqUrl);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            __networking_isDoingNetworkOpetaion = false;
            char* errText = malloc(256);
            sprintf(errText, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
            showErrorScreen(errText);
            return apiResponse;
        }

        // Parse JSON response using cJSON
        cJSON *root = cJSON_Parse(chunk.memory);
        if (root == NULL) {
            __networking_isDoingNetworkOpetaion = false;
            char* errText = malloc(256);
            sprintf(errText, "Error parsing JSON: %s", cJSON_GetErrorPtr());
            showErrorScreen(errText);
            return apiResponse;
        }

        cJSON *cSeason = cJSON_GetObjectItem(root, "season");       // Correct season.
        cJSON *cEpisode = cJSON_GetObjectItem(root, "episode");     // Correct episode.
        cJSON *seekTime = cJSON_GetObjectItem(root, "seekTime");    // Time frame was taken.
        cJSON *expiryTs = cJSON_GetObjectItem(root, "expiryTs");    // ?
        cJSON *isCorrect = cJSON_GetObjectItem(root, "correct");    // Is the answer correct?

        // A bunch of checks just because ERROR HANDLING DAMNIT.
        if (cJSON_IsNumber(cSeason) && cJSON_IsNumber(cEpisode) && cJSON_IsNumber(seekTime) && cJSON_IsNumber(expiryTs) && cJSON_IsBool(isCorrect)) {
            // Set the data (but in the machine this time).
            int mSeason = cSeason->valueint;
            int mEpisode = cEpisode->valueint;
            float mSeekTime = (float)seekTime->valuedouble; // I have an odd feeling that this is gonna be set like X.XX instead of X.XXXXXXXXX.
            int mExpiryTs = expiryTs->valueint;
            bool mCorrect = cJSON_IsTrue(isCorrect);

            // Set the data.
            apiResponse.season = mSeason;
            apiResponse.episode = mEpisode;
            apiResponse.seekTime = mSeekTime;
            apiResponse.expiryTs = mExpiryTs;
            apiResponse.correct = mCorrect;

            cJSON_Delete(root);
            curl_easy_cleanup(curl);
            free(chunk.memory);

            LWP_SuspendThread(__networking_waitingThread);
            __networking_isDoingNetworkOpetaion = false;

            return apiResponse;
        } else {
            __networking_isDoingNetworkOpetaion = false;
            LWP_SuspendThread(__networking_waitingThread);

            char* errTxt = malloc(256);
            sprintf(errTxt, "Error: 'id' field not found or not a string");
            cJSON_Delete(root);
            curl_easy_cleanup(curl);
            free(chunk.memory);
            showErrorScreen(errTxt);
            return apiResponse;
        }
    }

    curl_global_cleanup();
    free(reqUrl);
    free(__networking_waitingText);
    LWP_SuspendThread(__networking_waitingThread);
    __networking_isDoingNetworkOpetaion = false;
    return apiResponse;
}