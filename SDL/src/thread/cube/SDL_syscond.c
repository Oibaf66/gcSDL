/***********************************************************************
 * SDL_syscond.c
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
#include "SDL_thread.h"
#include "SDL_sysmutex_c.h"

#include <ogcsys.h>
#include <ogc/cond.h>

struct SDL_cond
{
	cond_t cond;
};

/* Helper function */
extern int clock_gettime(struct timespec *tp);

/* Create a condition variable */
SDL_cond * SDL_CreateCond(void)
{
	SDL_cond *cond;

	cond = (SDL_cond *) SDL_malloc(sizeof(SDL_cond));

	if ( cond ) {
		/* Tell lwp to create it */
		if (LWP_CondInit(&(cond->cond)) < 0)
		{
			SDL_DestroyCond(cond);
			cond = NULL;
		}
	} else {
		SDL_OutOfMemory();
	}
	return (cond);
}

/* Destroy a condition variable */
void SDL_DestroyCond(SDL_cond *cond)
{
	if ( cond ) {
		/* Tell lwp to remove it */
		LWP_CondDestroy(cond->cond);
		SDL_free(cond);
	}
}

/* Restart one of the threads that are waiting on the condition variable */
int SDL_CondSignal(SDL_cond *cond)
{
	if ( ! cond ) {
		SDL_SetError("Passed a NULL condition variable");
		return -1;
	}

	/* Tell lwp to signal the condition and wait for the thread to respond. */
	return LWP_CondSignal(cond->cond) == 0 ? 0 : -1;
}

/* Restart all threads that are waiting on the condition variable */
int SDL_CondBroadcast(SDL_cond *cond)
{
	if ( ! cond ) {
		SDL_SetError("Passed a NULL condition variable");
		return -1;
	}

	/* If there are waiting threads not already signalled, then tell
	   lwp to signal the condition and wait for the thread to respond.
	*/
	return LWP_CondBroadcast(cond->cond) == 0 ? 0 : -1;
}

/* Wait on the condition variable for at most 'ms' milliseconds. */
int SDL_CondWaitTimeout(SDL_cond *cond, SDL_mutex *mutex, Uint32 ms) //FABIO
{
	struct timespec time; 

	if (!cond)
	{
		SDL_SetError("Passed a NULL condition variable");
		return -1;
	}
	//LWP_CondTimedWait expects relative timeout
	time.tv_sec = (ms / 1000);
	time.tv_nsec = (ms % 1000) * 1000000;

	return LWP_CondTimedWait(cond->cond, mutex->id, &time);
}


/* Wait on the condition variable forever */
int SDL_CondWait(SDL_cond *cond, SDL_mutex *mutex)
{
	/* Tell lwp to wait */
	return LWP_CondWait(cond->cond, mutex->id);
}
