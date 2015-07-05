/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken at libsdl.or
*/
#include "SDL_config.h"

// Public includes.
#include "SDL_timer.h"

// Audio internal includes.
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_sysaudio.h"
#include "../SDL_audio_c.h"

// Cube audio internal includes.
#include "SDL_cubeaudio.h"

#include <stdio.h>

// for memalign
#include <malloc.h>

static const char CUBEAUD_DRIVER_NAME[] = "cube";

#define DMA_BUFFER_SIZE (SAMPLES_PER_DMA_BUFFER*2*sizeof(short))

static lwp_t athread;
static CubeAudio *current = NULL;

/****************************************************************************
 * Audio Threading
 ***************************************************************************/
static void *
AudioThread (CubeAudio *private)
{
	u32 buffer_size;
	Uint8 whichab = 1;

	while (1)
	{
		LWP_SuspendThread(athread);
		if (private->stopaudio)
			break;

		DCZeroRange(private->dma_buffers[whichab], DMA_BUFFER_SIZE);
		buffer_size = DMA_BUFFER_SIZE;

		// Is the device ready?
		if (current_audio && !current_audio->paused)
		{
			SDL_LockMutex(current_audio->mixer_lock);

			if (current_audio->convert.needed)
			{
				// Get the client to produce audio
				current_audio->spec.callback(
						current_audio->spec.userdata,
						current_audio->convert.buf,
						current_audio->convert.len);

				// Convert the audio
				SDL_ConvertAudio(&current_audio->convert);

				// Copy from SDL buffer to DMA buffer
				memcpy(private->dma_buffers[whichab], current_audio->convert.buf, current_audio->convert.len_cvt);
				buffer_size = current_audio->convert.len_cvt;
			} else {
				current_audio->spec.callback(
					current_audio->spec.userdata,
					(Uint8 *)(private->dma_buffers[whichab]),
					DMA_BUFFER_SIZE);
				buffer_size = DMA_BUFFER_SIZE;
			}

			SDL_UnlockMutex(current_audio->mixer_lock);
		}
		else if (current_audio && (current_audio->spec.format&0x8000)==0) // hack
		{
			int i;
			// if it's an unsigned format use 0x8000 for silence (16-bit)
			short fill = 0x8000;

			// 0x80 for 8-bit formats
			if (current_audio->spec.format&0x08)
				fill |= 0x80;

			for (i=0; i < SAMPLES_PER_DMA_BUFFER*2; i++)
				private->dma_buffers[whichab][i] = fill;
		}

		AESND_SetVoiceBuffer(private->voice, private->dma_buffers[whichab], buffer_size);
		whichab ^= 1;
	}
	return NULL;
}

/****************************************************************************
 * DMACallback
 * signal audio thread that more samples are required
 ***************************************************************************/
static void
DMACallback(AESNDPB *pb, u32 state)
{
	if (state == VOICE_STATE_STREAM)
		LWP_ResumeThread(athread);
}

void CUBE_AudioStop(CubeAudio *private)
{
	if (private==NULL) {
		if (current==NULL)
			return;
		private = current;
	}

	if (private->voice) {
		AESND_SetVoiceStop(private->voice, 1);
		AESND_FreeVoice(private->voice);
		private->voice = NULL;
	}

	private->stopaudio = true;
	if (athread != LWP_THREAD_NULL) {
		LWP_ResumeThread(athread);
		LWP_JoinThread(athread, NULL);
		athread = LWP_THREAD_NULL;
	}

	AESND_Pause(1);
	// this function is broken
	//AESND_Reset();
}

int CUBE_AudioStart(CubeAudio *private)
{
	if (private==NULL) {
		if (current==NULL)
			return -1;
		private = current;
	}

	memset(private->dma_buffers, 0, sizeof(private->dma_buffers));
	private->stopaudio = false;
	private->voice = AESND_AllocateVoice(DMACallback);
	if (private->voice==NULL)
		return -1;

	if (LWP_CreateThread(&athread, (void*(*)(void*))AudioThread, private, private->astack, AUDIOSTACK, 80) < 0) {
		AESND_FreeVoice(private->voice);
		private->voice = NULL;
		return -1;
	}

	// start audio
	// this is retarded. Why isn't there one function to do all this shit?
	AESND_SetVoiceFormat(private->voice, private->format);
	AESND_SetVoiceFrequency(private->voice, private->freq);
	AESND_SetVoiceBuffer(private->voice, private->dma_buffers[0], DMA_BUFFER_SIZE);
	AESND_SetVoiceStream(private->voice, true);
	AESND_SetVoiceStop(private->voice, 0);
	AESND_Pause(0);

	current = private;
	return 1;
}

static int CUBEAUD_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	u32 format;
	CubeAudio *private = (CubeAudio*)(this->hidden);

	if (spec->freq <= 0 || spec->freq > 144000)
		spec->freq = DSP_DEFAULT_FREQ;

	spec->samples = DMA_BUFFER_SIZE;

	// no support for little endian or 16 bit unsigned
	switch (spec->format) {
#ifdef VOICE_MONO8_UNSIGNED
		case AUDIO_U8:
			format = VOICE_MONO8_UNSIGNED;
			break;
#endif
		case AUDIO_S8:
			format = VOICE_MONO8;
			break;
		default:
		case AUDIO_U16LSB:
		case AUDIO_U16MSB:
		case AUDIO_S16LSB:
			spec->format = AUDIO_S16MSB;
			// fallthrough
		case AUDIO_S16MSB:
			format = VOICE_MONO16;
			spec->samples >>= 1;
	}

	if (spec->channels > 2)
		spec->channels = 2;
	if (spec->channels == 2) {
		++format;
		spec->samples >>= 1;
	}

	spec->padding	= 0;
	SDL_CalculateAudioSpec(spec);

	private->format = format;
	private->freq = spec->freq;

	return CUBE_AudioStart(private);
}

static void CUBEAUD_CloseAudio(_THIS)
{
	CUBE_AudioStop((CubeAudio*)(this->hidden));
	current = NULL;
}

static void CUBEAUD_DeleteDevice(_THIS)
{
	CUBE_AudioStop((CubeAudio*)(this->hidden));

	free(this->hidden);
	SDL_free(this);
}

static SDL_AudioDevice *CUBEAUD_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	athread = LWP_THREAD_NULL;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = (CubeAudio*)memalign(32, sizeof(CubeAudio));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		SDL_free(this);
		return NULL;
	}
	SDL_memset(this->hidden, 0, sizeof(CubeAudio));

	// Initialise the Cube side of the audio system
	AESND_Init();
	AESND_Pause(1);

	/* Set the function pointers */
	this->OpenAudio = CUBEAUD_OpenAudio;
	//this->WaitAudio = CUBEAUD_WaitAudio;
	//this->PlayAudio = CUBEAUD_PlayAudio;
	//this->GetAudioBuf = CUBEAUD_GetAudioBuf;
	this->CloseAudio = CUBEAUD_CloseAudio;
	this->free = CUBEAUD_DeleteDevice;

	return this;
}

static int CUBEAUD_Available(void)
{
	return 1;
}

AudioBootStrap CUBEAUD_bootstrap = {
	CUBEAUD_DRIVER_NAME, "SDL cube audio driver",
	CUBEAUD_Available, CUBEAUD_CreateDevice
};
