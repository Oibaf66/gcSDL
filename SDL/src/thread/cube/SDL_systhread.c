/***********************************************************************
 * SDL_systhread.c
 *
 * cubeSDL - SDL library 1.2 for GameCube
 * by infact <infact [at] quantentunnel [dot] de>
 * Source is based on code by Tantric, Softdev and others
 **********************************************************************/
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
#include "SDL_config.h"

/* Thread management routines for SDL */

#include "SDL_thread.h"
#include "../SDL_systhread.h"
#include "../SDL_thread_c.h"

#include <ogcsys.h>

/* Helper function */
void *run_thread(void *data)
{
	SDL_RunThread(data);
	/* Prevent compiler warning */
	return ((void *) 0);
}

int SDL_SYS_CreateThread(SDL_Thread *thread, void *args) //FABIO
{
	
	if ( LWP_CreateThread(&thread->handle, run_thread, args, 0, 0, 64) != 0 ) {
		SDL_SetError("Not enough resources to create thread");
		return(-1);
	}
	
	return (0);
}


void SDL_SYS_SetupThread(void)
{
	return;
}

Uint32 SDL_ThreadID(void)
{
	/* Ask lwp about the thread id */
	return (Uint32) LWP_GetSelf();
}

void SDL_SYS_WaitThread(SDL_Thread *thread)
{
	/* Tell lwp to join the thread, ignore return value */
	void *v;
	LWP_JoinThread(thread->handle, &v);
	return;
}

void SDL_SYS_KillThread(SDL_Thread *thread)
{
	return;
}
