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

#include "SDL_stdinc.h"

#ifndef HAVE_GETENV

/* roll our own */

static char **SDL_env = (char **)0;

/* Put a variable of the form "name=value" into the environment */
int SDL_putenv(const char *variable)
{
	const char *name, *value;
	int added;
	int len, i;
	char **new_env;
	char *new_variable;

	/* A little error checking */
	if ( ! variable ) {
		return(-1);
	}
	name = variable;
	for ( value=variable; *value && (*value != '='); ++value ) {
		/* Keep looking for '=' */ ;
	}
	if ( *value ) {
		++value;
	} else {
		return(-1);
	}

	/* Allocate memory for the variable */
	new_variable = SDL_strdup(variable);
	if ( ! new_variable ) {
		return(-1);
	}

	/* Actually put it into the environment */
	added = 0;
	i = 0;
	if ( SDL_env ) {
		/* Check to see if it's already there... */
		len = (value - name);
		for ( ; SDL_env[i]; ++i ) {
			if ( SDL_strncmp(SDL_env[i], name, len) == 0 ) {
				break;
			}
		}
		/* If we found it, just replace the entry */
		if ( SDL_env[i] ) {
			SDL_free(SDL_env[i]);
			SDL_env[i] = new_variable;
			added = 1;
		}
	}

	/* Didn't find it in the environment, expand and add */
	if ( ! added ) {
		new_env = SDL_realloc(SDL_env, (i+2)*sizeof(char *));
		if ( new_env ) {
			SDL_env = new_env;
			SDL_env[i++] = new_variable;
			SDL_env[i++] = (char *)0;
			added = 1;
		} else {
			SDL_free(new_variable);
		}
	}
	return (added ? 0 : -1);
}

/* Retrieve a variable named "name" from the environment */
char *SDL_getenv(const char *name)
{
	int len, i;
	char *value;

	value = (char *)0;
	if ( SDL_env ) {
		len = SDL_strlen(name);
		for ( i=0; SDL_env[i] && !value; ++i ) {
			if ( (SDL_strncmp(SDL_env[i], name, len) == 0) &&
			     (SDL_env[i][len] == '=') ) {
				value = &SDL_env[i][len+1];
			}
		}
	}
	return value;
}

#endif /* !HAVE_GETENV */
