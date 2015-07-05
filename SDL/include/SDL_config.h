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

#ifndef _SDL_config_h
#define _SDL_config_h

#include "SDL_platform.h"

/* Add any platform that doesn't build using the configure system */
#if defined(__cube__)

#include <stdarg.h>

/* Types */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
typedef unsigned int size_t;
typedef unsigned int uintptr_t;

/* Architecture */
#define SDL_BYTEORDER		SDL_BIG_ENDIAN
#define SDL_HAS_64BIT_TYPE	1

/* Useful headers */
#define HAVE_ALLOCA_H		1
#define HAVE_SYS_TYPES_H	1
#define HAVE_STDIO_H		1
#define STDC_HEADERS		1
#define HAVE_STRING_H		1
#define HAVE_STRINGS_H		1
#define HAVE_CTYPE_H		1
#define HAVE_INTTYPES_H		1
#define HAVE_MALLOC_H		1
#define HAVE_MATH_H		1
#define HAVE_SIGNAL_H		1
#define HAVE_STDINT_H		1
#define HAVE_STDLIB_H		1
// unused or unsupported
#undef HAVE_MEMORY_H
#undef HAVE_ICONV_H

/* C library functions */
#define HAVE_MALLOC	1
#define HAVE_CALLOC	1
#define HAVE_REALLOC	1
#define HAVE_FREE	1
#define HAVE_ALLOCA	1
#define HAVE_QSORT	1
#define HAVE_ABS	1
#define HAVE_BCOPY	1
#define HAVE_MEMSET	1
#define HAVE_MEMCPY	1
#define HAVE_MEMMOVE	1
#define HAVE_MEMCMP	1
#define HAVE_STRLEN	1
#define HAVE_STRLCPY	1
#define HAVE_STRLCAT	1
#define HAVE_STRDUP	1
#define HAVE_STRCHR	1
#define HAVE_STRRCHR	1
#define HAVE_STRSTR	1
#define HAVE_STRTOL	1
#define HAVE_STRTOUL	1
#define HAVE_STRTOLL	1
#define HAVE_STRTOULL	1
#define HAVE_STRTOD	1
#define HAVE_ATOI	1
#define HAVE_ATOF	1
#define HAVE_STRCMP	1
#define HAVE_STRNCMP	1
#define HAVE_STRCASECMP	1
#define HAVE_STRNCASECMP 1
#define HAVE_SSCANF	1
#define HAVE_SETJMP	1
// unused or unsupported
#undef HAVE__STRREV
#undef HAVE__STRUPR
#undef HAVE__STRLWR
#undef HAVE_INDEX
#undef HAVE_RINDEX
#undef HAVE_ITOA
#undef HAVE__LTOA
#undef HAVE__UITOA
#undef HAVE__ULTOA
#undef HAVE__I64TOA
#undef HAVE__UI64TOA
#undef HAVE__STRICMP
#undef HAVE__STRNICMP
#undef HAVE_SNPRINTF
#undef HAVE_VSNPRINTF
#undef HAVE_ICONV
#undef HAVE_SIGACTION
#undef HAVE_NANOSLEEP
#undef HAVE_CLOCK_GETTIME
#undef HAVE_DLVSYM
#undef HAVE_GETPAGESIZE
#undef HAVE_MPROTECT

/* Enable cube drivers */
#define SDL_AUDIO_DRIVER_CUBE 1
#define SDL_JOYSTICK_CUBE 1
#define SDL_THREAD_CUBE 1
#define SDL_TIMERS_CUBE 1
#define SDL_VIDEO_DRIVER_CUBE 1

/* Enable stub drivers */
#define SDL_CDROM_DISABLED 1
#define SDL_LOADSO_DISABLED 1

#else
#error only gamecube platform is supported
#endif /* __cube__ */

#endif /* _SDL_config_h */
