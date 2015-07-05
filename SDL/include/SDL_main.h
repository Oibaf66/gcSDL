/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

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
    slouken@libsdl.org
*/

#ifndef _SDL_main_h
#define _SDL_main_h

#include "SDL_stdinc.h"

/** @file SDL_main.h
 *  Redefine main() so that it is called by PLATFORM_main.c
 */

#if defined(__cube__)

#ifdef __cplusplus
#define C_LINKAGE	"C"
#else
#define C_LINKAGE
#endif /* __cplusplus */

/** The application's main() function must be called with C linkage,
 *  and should be declared like this:
 *      @code
 *      #ifdef __cplusplus
 *      extern "C"
 *      #endif
 *	int main(int argc, char *argv[])
 *	{
 *	}
 *      @endcode
 */
#define main	SDL_main

/** The prototype for the application's main() function */
extern C_LINKAGE int SDL_main(int argc, char *argv[]);

#endif /* __cube__ */

#endif /* _SDL_main_h */
