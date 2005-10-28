/**********************************************************************
 * Premake - util.c
 * Support functions.
 *
 * Copyright (c) 2002-2005 Jason Perkins.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License in the file LICENSE.txt for details.
 **********************************************************************/

#include <string.h>
#include "util.h"


/* Checks to see if two strings match. Just a more readable version
 * of the standard strcmp() function */
int matches(const char* str0, const char* str1)
{
	return (strcmp(str0, str1) == 0);
}
