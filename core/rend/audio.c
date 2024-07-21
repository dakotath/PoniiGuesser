/**
 * @file audio.c
 * @author Dakota Thorpe.
 * @copyright &copy;2024.
 * @details Audio SFX Handler.
*/

// audio.c - (C)2024 Dakota Thorpe.

// Libs.
#include <stdio.h>
#include <stdlib.h>
#include <asndlib.h>

/**
 * @author Dakota Thorpe
 * @paragraph pSFX_p0 Plays a raw sound effect (BE Stereo 8Bit).
 * 
 * @param buffer The audio buffer.
 * @param buffer_size Buffer size.
 * @param channel Voice to play buffer on.
*/
void playSfx(const void* buffer, long buffer_size, int channel)
{
	u32 format = VOICE_MONO_8BIT;
    ASND_SetVoice(channel, format, 32000, 0, (void*)buffer, buffer_size, 255, 255, NULL);
	ASND_Pause(0);
}