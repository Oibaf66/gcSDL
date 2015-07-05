
// normal includes
#include <stdlib.h>
#include <time.h>
#include <gccore.h>
//#include <wiiuse/wpad.h>
#include "select_BE.h"
 
 
// SDL includes
#include <sdl/sdl.h>

//int select_BE_raw_len
//unsigned char select_BE_raw[]

int pointer = 0;
int stop_sound =1;

void sdlcallback(void *unused, Uint8 *stream, int len)
{

	if (stop_sound) return;
	
	printf("pointer= %d\n", pointer);

	memcpy(stream, select_BE_raw + pointer, len); 
	
if (pointer < select_BE_raw_len-len) pointer +=len; else {pointer =0; stop_sound = 1;}
}


int sound_init_sdl() {
	
	SDL_AudioSpec fmt;

		
	 /* Set 16-bit stereo audio at 48Khz */
    fmt.freq = 48000;
    fmt.format = AUDIO_S16SYS; //signed Little endian/Big endian
    fmt.channels = 2;
    fmt.samples = 4096; //number of samples
    fmt.callback = sdlcallback;
    fmt.userdata = NULL;
	

    /* Open the audio device and start playing sound! */
    if (SDL_OpenAudio(&fmt, NULL) < 0 ) return -1;
	
	printf("SDL audio initiated\n");
	
	printf("freq = %d\n",fmt.freq);
	printf("channels = %d\n",fmt.channels);
	printf("buffer_len = %d\n",fmt.samples);
	printf("format = %x\n",fmt.format);

	SDL_PauseAudio(0);

	return 0;
}


 
int main(int argc, char** argv){
	// main function. Always starts first


sound_init_sdl();

stop_sound = 0;

int i;
	  
	for(i=0;i<10; i++)
	{
 		SDL_Delay(1000);
		printf("Sec=%d\n",i);
		stop_sound = 0;	
	}	 
	 
    return 0;
}