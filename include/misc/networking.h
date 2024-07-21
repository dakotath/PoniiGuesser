#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdbool.h>

#define API_BASE "https://ponyguessr.com/api"

struct MemoryStruct {
    char *memory;
    size_t size;
};

typedef struct {
    int season;
    int episode;
    float seekTime;
    int expiryTs;
    bool correct;
} response_t;

void Networking_Init();

char *get_image_id(); // Obtain an Image ID.
void downloadImage(char* iid); // Download image with specified IID.
response_t checkCorrect(int season, int episode, char* iid);

#endif