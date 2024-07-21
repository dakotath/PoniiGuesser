// audio.h - (C)2024 Dakota Thorpe.
#ifndef AUDIO_H
#define AUDIO_H

// Libs.
#include <stdio.h>
#include <stdlib.h>
#include <asndlib.h>

void playSfx(const void* buffer, long buffer_size, int channel);

#endif