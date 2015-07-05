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

/** @file SDL_syswm.h
 *  Include file for SDL custom system window manager hooks
 */

#ifndef _SDL_syswm_h
#define _SDL_syswm_h

#include "SDL_stdinc.h"
#include "SDL_error.h"
#include "SDL_version.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/** @file SDL_syswm.h
 *  Your application has access to a special type of event 'SDL_SYSWMEVENT',
 *  which contains window-manager specific information and arrives whenever
 *  an unhandled window event occurs.  This event is ignored by default, but
 *  you can enable it with SDL_EventState()
 */
#ifdef SDL_PROTOTYPES_ONLY
struct SDL_SysWMinfo;
typedef struct SDL_SysWMinfo SDL_SysWMinfo;
#else

/* This is the structure for custom window manager events */

/** The generic custom event structure */
struct SDL_SysWMmsg {
	SDL_version version;
	int data;
};

/** The generic custom window manager information structure */
typedef struct SDL_SysWMinfo {
	SDL_version version;
	int data;
} SDL_SysWMinfo;

#endif /* SDL_PROTOTYPES_ONLY */

/* Function prototypes */
/**
 * This function gives you custom hooks into the window manager information.
 * It fills the structure pointed to by 'info' with custom information and
 * returns 0 if the function is not implemented, 1 if the function is 
 * implemented and no error occurred, and -1 if the version member of
 * the 'info' structure is not filled in or not supported.
 *
 * You typically use this function like this:
 * @code
 * SDL_SysWMinfo info;
 * SDL_VERSION(&info.version);
 * if ( SDL_GetWMInfo(&info) ) { ... }
 * @endcode
 */
extern DECLSPEC int SDLCALL SDL_GetWMInfo(SDL_SysWMinfo *info);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_syswm_h */
