// http://wiibrew.org/wiki/SDL_Wii/tutorial
// http://lazyfoo.net/tutorials/SDL/16_true_type_fonts/index.php
// normal includes
#include <stdlib.h>
#include <time.h>
#include <gccore.h>
//#include <wiiuse/wpad.h> 
 
// SDL includes
#include <sdl/sdl.h>
#include <sdl/sdl_image.h>
#include <sdl/sdl_ttf.h>
#include <sdl/sdl_rotozoom.h>
 
// the surfaces
SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *text = NULL;

TTF_Font *gFont = NULL;

double rot=0.0;

#include "DejaVu.h"

 
void init(){
 
	// initialize SDL video. If there was an error SDL shows it on the screen
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 )
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError() );
		SDL_Delay( 5000 );
        exit(EXIT_FAILURE);
    }
 
	// button initialization
//	WPAD_Init();
 
    // make sure SDL cleans up before exit
    atexit(SDL_Quit);
//    SDL_ShowCursor(SDL_DISABLE);
 
    // create a new window
    screen = SDL_SetVideoMode(640, 480, 16, SDL_DOUBLEBUF);
    if ( !screen )
    {
        fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
		SDL_Delay( 5000 );
        exit(EXIT_FAILURE);
    }
 
	// load the background image using SDL_LoadBMP
	// Make sure you use "sd:/" because SDL loads from the root!
 
	//background = SDL_LoadBMP("sd:/apps/SDL/background.bmp");
	
	
	if( TTF_Init() == -1 )
	{
		printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
		exit(EXIT_FAILURE);
	}
	
	SDL_RWops *rw = SDL_RWFromConstMem(DejaVuSans_ttf, sizeof(DejaVuSans_ttf));
	
	gFont = TTF_OpenFontRW( rw, 1, 32 );
	if( gFont == NULL )
	{
		printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
		exit(EXIT_FAILURE);
	}
	else
	{
		//Render text
		SDL_Color textColor = { 255, 255, 0 };
		text = TTF_RenderText_Solid( gFont, "rotozoom!", textColor );
	}
}
 
void apply_surface ( int x, int y, SDL_Surface* source, SDL_Surface* destination ){
	
	 // make a temporary rectangle to hold the offsets
     SDL_Rect offset;
 
     // give the offsets to the rectangle
     offset.x = x;
     offset.y = y;
 
     // blit the surface
     SDL_BlitSurface( source, NULL, destination, &offset );
}
 
void cleanup(){
 
	// we have to quit SDL and free the surface of background
	SDL_Quit();
	SDL_FreeSurface(background);
	exit(EXIT_SUCCESS);
}
 
int main(int argc, char** argv){
	// main function. Always starts first
 
	// to stop the while loop
	bool done = false;
 
	// start init() function
	init();
 
	// apply surface on screen
	//apply_surface(0, 0, background, screen);
 
	// this is the endless while loop until someone presses the home button on the wiimote
	while (!done)
    {
		// scans if a button was pressed
//        WPAD_ScanPads();
//		u32 held = WPAD_ButtonsHeld(0);
 
		// if the homebutton is pressed it will set done = true and it will fill the screen
		// with a black background
/*		if(held & WPAD_BUTTON_HOME){
			done=true;
			SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
		}*/
 
		SDL_FillRect(screen, NULL, 0x000000);
		if(background)
			SDL_FreeSurface(background);
		background = rotozoomSurface(text, rot, 1.0f, 0);
		apply_surface(640/2 - background->w/2,
					  480/2 - background->h/2,
                      background,
					  screen);
		rot+=0.5;
		if(rot>360.0) rot=0.0;
 
		// SDL_Flip refreshes the screen so it will show the updated screen
		SDL_Flip(screen);
    }
 
	// start cleanup() function
	cleanup();
 
    return 0;
}