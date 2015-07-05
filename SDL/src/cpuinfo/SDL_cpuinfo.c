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

/* CPU feature detection for SDL */

#include "SDL.h"
#include "SDL_cpuinfo.h"

/* GEKKO does not support the special cpu features */

static __inline__ int CPU_haveCPUID(void)
{
	return 0;
}

static __inline__ int CPU_getCPUIDFeatures(void)
{
	return 0;
}

static __inline__ int CPU_getCPUIDFeaturesExt(void)
{
	return 0;
}

static __inline__ int CPU_haveRDTSC(void)
{
	return 0;
}

static __inline__ int CPU_haveMMX(void)
{
	return 0;
}

static __inline__ int CPU_haveMMXExt(void)
{
	return 0;
}

static __inline__ int CPU_have3DNow(void)
{
	return 0;
}

static __inline__ int CPU_have3DNowExt(void)
{
	return 0;
}

static __inline__ int CPU_haveSSE(void)
{
	return 0;
}

static __inline__ int CPU_haveSSE2(void)
{
	return 0;
}

static __inline__ int CPU_haveAltiVec(void)
{
	return 0;
}

SDL_bool SDL_HasRDTSC(void)
{
	return SDL_FALSE;
}

SDL_bool SDL_HasMMX(void)
{
	return SDL_FALSE;
}

SDL_bool SDL_HasMMXExt(void)
{
	return SDL_FALSE;
}

SDL_bool SDL_Has3DNow(void)
{
	return SDL_FALSE;
}

SDL_bool SDL_Has3DNowExt(void)
{
	return SDL_FALSE;
}

SDL_bool SDL_HasSSE(void)
{
	return SDL_FALSE;
}

SDL_bool SDL_HasSSE2(void)
{
	return SDL_FALSE;
}

SDL_bool SDL_HasAltiVec(void)
{
	return SDL_FALSE;
}
