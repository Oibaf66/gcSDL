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

/**
 *  @file SDL_endian.h
 *  Functions for reading and writing endian-specific values
 */

#ifndef _SDL_endian_h
#define _SDL_endian_h

#include "SDL_stdinc.h"

/** @name SDL_ENDIANs
 *  The two types of endianness 
 */
/*@{*/
#define SDL_LIL_ENDIAN	1234
#define SDL_BIG_ENDIAN	4321
/*@}*/

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @name SDL_Swap Functions
 *  Use inline functions for compilers that support them, and static
 *  functions for those that do not.  Because these functions become
 *  static for compilers that do not support inline functions, this
 *  header should only be included in files that actually use them.
 */
/*@{*/
#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ Uint16 SDL_Swap16(Uint16 x)
{
	int result;

	__asm__("rlwimi %0,%2,8,16,23" : "=&r" (result) : "0" (x >> 8), "r" (x));
	return (Uint16)result;
}
#else
static __inline__ Uint16 SDL_Swap16(Uint16 x) {
	return SDL_static_cast(Uint16, ((x<<8)|(x>>8)));
}
#endif

#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ Uint32 SDL_Swap32(Uint32 x)
{
	Uint32 result;

	__asm__("rlwimi %0,%2,24,16,23" : "=&r" (result) : "0" (x>>24), "r" (x));
	__asm__("rlwimi %0,%2,8,8,15"   : "=&r" (result) : "0" (result),    "r" (x));
	__asm__("rlwimi %0,%2,24,0,7"   : "=&r" (result) : "0" (result),    "r" (x));
	return result;
}
#else
static __inline__ Uint32 SDL_Swap32(Uint32 x) {
	return SDL_static_cast(Uint32, ((x<<24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x>>24)));
}
#endif

#ifdef SDL_HAS_64BIT_TYPE
static __inline__ Uint64 SDL_Swap64(Uint64 x)
{
	Uint32 hi, lo;

	/* Separate into high and low 32-bit values and swap them */
	lo = SDL_static_cast(Uint32, x & 0xFFFFFFFF);
	x >>= 32;
	hi = SDL_static_cast(Uint32, x & 0xFFFFFFFF);
	x = SDL_Swap32(lo);
	x <<= 32;
	x |= SDL_Swap32(hi);
	return (x);
}
#else
/* This is mainly to keep compilers from complaining in SDL code.
 * If there is no real 64-bit datatype, then compilers will complain about
 * the fake 64-bit datatype that SDL provides when it compiles user code.
 */
#define SDL_Swap64(X)	(X)
#endif /* SDL_HAS_64BIT_TYPE */
/*@}*/

/**
 *  @name SDL_SwapLE and SDL_SwapBE Functions
 *  Byteswap item from the specified endianness to the native endianness
 */
/*@{*/
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SDL_SwapLE16(X)	(X)
#define SDL_SwapLE32(X)	(X)
#define SDL_SwapLE64(X)	(X)
#define SDL_SwapBE16(X)	SDL_Swap16(X)
#define SDL_SwapBE32(X)	SDL_Swap32(X)
#define SDL_SwapBE64(X)	SDL_Swap64(X)
#else
#define SDL_SwapLE16(X)	SDL_Swap16(X)
#define SDL_SwapLE32(X)	SDL_Swap32(X)
#define SDL_SwapLE64(X)	SDL_Swap64(X)
#define SDL_SwapBE16(X)	(X)
#define SDL_SwapBE32(X)	(X)
#define SDL_SwapBE64(X)	(X)
#endif
/*@}*/

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_endian_h */
