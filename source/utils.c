// utils.c - (C)2024 Dakota Thorpe.
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "cJSON.h"
#include "misc/utils.h"

// Function to retrieve directory names and return them as an array of strings
char **getFolders(const char *dirPath, int *numFolders) {
    struct dirent *de; // Pointer for directory entry 

    // opendir() returns a pointer of DIR type. 
    DIR *dr = opendir(dirPath); 

    if (dr == NULL) // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open directory %s\n", dirPath); 
        return NULL; 
    } 

    // Allocate memory for storing directory names
    char **folders = NULL;
    int folderCount = 0;
    int folderCapacity = 10; // Initial capacity, can be adjusted

    folders = (char **)malloc(folderCapacity * sizeof(char *));
    if (folders == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        closedir(dr);
        return NULL;
    }

    // Read directory entries
    while ((de = readdir(dr)) != NULL) {
        // Skip . and .. entries
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        // Allocate memory for directory name
        folders[folderCount] = (char *)malloc(strlen(de->d_name) + 1);
        if (folders[folderCount] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            closedir(dr);
            // Free previously allocated memory
            for (int i = 0; i < folderCount; i++) {
                free(folders[i]);
            }
            free(folders);
            return NULL;
        }

        // Copy directory name into allocated memory
        strcpy(folders[folderCount], de->d_name);
        folderCount++;

        // Resize folders array if necessary
        if (folderCount >= folderCapacity) {
            folderCapacity *= 2;
            folders = (char **)realloc(folders, folderCapacity * sizeof(char *));
            if (folders == NULL) {
                fprintf(stderr, "Memory reallocation failed\n");
                closedir(dr);
                // Free previously allocated memory
                for (int i = 0; i < folderCount; i++) {
                    free(folders[i]);
                }
                free(folders);
                return NULL;
            }
        }
    }

    closedir(dr);

    // Set the number of folders and return the array
    *numFolders = folderCount;
    return folders;
}

// Function to get audio file names from meta.json
char** GetAudioNames(char* packName, int* entryCount) {
    // Construct the file path
    char filePath[100]; // Adjust the size according to your needs
    sprintf(filePath, "sd:/carhorn/packs/%s/meta.json", packName);

    // Open the file
    FILE* fp = fopen(filePath, "r");
    if (!fp) {
        perror("Error opening file");
        return NULL;
    }

    // Determine the file size
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate memory to read the file content
    char* fileContent = (char*)malloc(fileSize + 1);
    if (!fileContent) {
        fclose(fp);
        perror("Memory allocation error");
        return NULL;
    }

    // Read the file content into memory
    size_t bytesRead = fread(fileContent, 1, fileSize, fp);
    if (bytesRead != fileSize) {
        fclose(fp);
        free(fileContent);
        perror("Error reading file");
        return NULL;
    }
    fileContent[bytesRead] = '\0'; // Null-terminate the string

    fclose(fp);

    // Parse JSON content
    cJSON* json = cJSON_Parse(fileContent);
    free(fileContent); // Free the allocated memory for file content after parsing
    if (!json) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        cJSON_Delete(json);
        return NULL;
    }

    // Get the array size (number of entries)
    int arraySize = cJSON_GetArraySize(json);

    // Allocate memory for file names array
    char** fileNames = (char**)malloc(arraySize * sizeof(char*));
    if (!fileNames) {
        cJSON_Delete(json);
        perror("Memory allocation error");
        return NULL;
    }

    FILE* quickDebugLog = fopen("carhorn_debug.log", "w");
    fprintf(quickDebugLog, "Pack Audio Entrys: %d\n", arraySize);

    // Extract file names
    for (int i = 0; i < arraySize; ++i) {
        cJSON* entry = cJSON_GetArrayItem(json, i);
        cJSON* file = cJSON_GetObjectItemCaseSensitive(entry, "file");
        if (cJSON_IsString(file) && (file->valuestring != NULL)) {
            fileNames[i] = strdup(file->valuestring);
        } else {
            fileNames[i] = NULL; // Handle cases where "file" is not a string
        }
    }

    cJSON_Delete(json); // Clean up cJSON object

    fclose(quickDebugLog);

    // Set the entry count
    *entryCount = arraySize;

    return fileNames;
}