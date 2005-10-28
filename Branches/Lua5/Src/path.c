/**********************************************************************
 * Premake - path.c
 * Path handling routines.
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

#include <stdlib.h>
#include <string.h>
#include "path.h"
#include "platform.h"


static const char separators[] = { '/', '/', '\\' };

static char working[8192];


const char* path_getdir(const char* path)
{
	char* ptr;

	if (path != NULL)
	{
		/* Convert path to neutral separators */
		strcpy(working, path);
		path_translateInPlace(working, POSIX);

		/* Now split at last separator */
		ptr = strrchr(working, '/');
		if (ptr != NULL)
			*ptr = '\0';
		return working;
	}
	else
	{
		return "";
	}
}


const char* path_join(const char* dir, const char* name, const char* ext)
{
	if (dir != NULL)
		strcpy(working, dir);
	else
		strcpy(working, "");

	if (strlen(working) > 0)
		strcat(working, "/");
	
	strcpy(working, name);
	
	if (ext != NULL && strlen(ext) > 0)
	{
		strcat(working, ".");
		strcat(working, ext);
	}
	
	return working;
}


void path_translateInPlace(char* buffer, int to)
{
	char* ptr;
	char  sep = (to == NATIVE) ? platform_getseparator() : separators[to];
	
	for (ptr = buffer; *ptr != '\0'; ++ptr)
	{
		if (*ptr == '\\' || *ptr == '/')
			*ptr = sep;
	}
}
