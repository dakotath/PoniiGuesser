#ifndef miscUTILS_H
#define miscUTILS_H

#include <stdlib.h>
#include <string.h>

char **getFolders(const char *dirPath, int *numFolders);
char** GetAudioNames(char* packName, int* entryCount);

#endif