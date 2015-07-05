/***********************************************************************
 * SDL_cube_main.c
 *
 * cubeSDL - SDL library 1.2 for GameCube
 * by infact <infact [at] quantentunnel [dot] de>
 * Source is based on code by Tantric, Softdev and others
 **********************************************************************/

/* Include the SDL main definition header */
#include "SDL_main.h"

/* redefined main */
#ifdef main
# undef main
#endif /* main */

#include <gccore.h>
#include <ogcsys.h>
#include <fat.h>

#include "../../video/cube/SDL_cubevideo.h"

#if CUBESDL_DEBUG
#include <debug.h>

/* Debug IP settings */
const char *tcp_localip = "192.168.23.45";
const char *tcp_netmask = "255.255.255.0";
const char *tcp_gateway = "192.168.23.1";
#endif

/* Misc. Variables */
int resetSystem = 0;

extern void SDL_Quit();

static void CubeResetCallBack()
{
	// quit the app
	SDL_Quit();
	
	SYS_ResetSystem (SYS_RESTART, 0, 0);
	
	//exit(0);
	
	//resetSystem = 1;
}

/* Initialisation of the gamecube platform and jump to the real app */
int main(int argc, char *argv[])
{
	/* cube target does not support command line arguments, but some
	 * apps want to write to them, which leads to a crash, for now
	 * just fake them.
	 */
	int new_argc = 1;
	char *new_argv[] = { "sdl_app" };

	// libogc init
	CUBE_InitVideoSystem();
	PAD_Init();
	fatInitDefault();

	printf ("init done.\n\n");

#if CUBESDL_DEBUG
	DEBUG_Init(GDBSTUB_DEVICE_TCP, 2345);
	printf ("\ndebug inited.");
	_break();
#endif

	// not sure, if this works: do something, when reset button is pressed
	SYS_SetResetCallback (CubeResetCallBack);

	// execute main app
	SDL_main (new_argc, new_argv);

	//printf ("\n\nsdl_app exited, you may reset the console now!\n");

	// will add press-key-to-exit here
	//while (!resetSystem); 

	SYS_ResetSystem (SYS_RESTART, 0, 0);

	return 0; 

	// this resets the console if the app exits, not leaving
	// any time to read error messages, etc.
	//return(SDL_main(argc, argv));
}
