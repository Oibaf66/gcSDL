//http://sol.gfxile.net/gp/ch02.html

#include <stdlib.h>
#include "SDL/SDL.h"

SDL_Surface *screen;

void putpixel(int x, int y, int color)
{
    Uint8 *ptr = (Uint8*)screen->pixels + y*screen->pitch + x*2;
    *(Uint16*)ptr = color;

}

void render()
{
    if (SDL_LockSurface(screen) < 0)
        return;

    //int tick = SDL_GetTicks();

    int i, j;

    for (i = 0; i < 480; i++)
    {
        for (j = 0; j < 640; j++)
        {
            //putpixel(j, i, SDL_MapRGB(screen->format, tick, j, i));
            //putpixel(j, i, i*i + j*j + tick);
            putpixel(j, i, SDL_MapRGB(screen->format, rand(), rand(), rand()));
        }
    }

    if (SDL_MUSTLOCK(screen))
        SDL_UnlockSurface(screen);

    SDL_UpdateRect(screen, 0, 0, 640, 480);
}


// Entry point
int main(int argc, char *argv[])
{
    // Initialize SDL's subsystems - in this case, only video.
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    // Register SDL_Quit to be called at exit; makes sure things are
    // cleaned up when we quit.
    atexit(SDL_Quit);

    // Attempt to create a 640x480 window with 32bit pixels.
    screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);

    // If we fail, return error.
    if ( screen == NULL )
    {
        fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
        exit(1);
    }

    // Main loop: loop forever.
    while (1)
    {
        // Render stuff
        render();
        // Poll for events, and handle the ones we care about.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    return 0;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_QUIT:
                return(0);
            }
        }
        //if (SDL_GetTicks() > 10000)             return 0;
        //SDL_Delay(50);
    }
    return 0;
}

