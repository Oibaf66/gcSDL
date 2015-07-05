
// normal includes
#include <stdlib.h>
#include <time.h>
#include <gccore.h>
//#include <wiiuse/wpad.h> 
 
// SDL includes
#include <sdl/sdl.h>
 
 
 
int main(int argc, char** argv){
	// main function. Always starts first

int i;
	  
	for(i=0;i<10; i++)
	{
 		SDL_Delay(1000);
		printf("Sec=%d\n",i);
	}

	//SYS_ResetSystem(SYS_POWEROFF, 0, 0); 
	 
    return 0;
}