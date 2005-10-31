/**********************************************************************
 * Premake - io.c
 * File and directory I/O routines.
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

#include <stdio.h>
#include <string.h>
#include "io.h"
#include "path.h"
#include "platform.h"

static char buffer[8192];
static FILE* file;


int io_closefile()
{
	fclose(file);
	return 1;
}


int io_copyfile(const char* src, const char* dest)
{
	return platform_copyfile(src, dest);
}


const char* io_findlib(const char* name)
{
	if (platform_findlib(name, buffer, 8192))
		return buffer;
	else
		return NULL;
}



const char* io_getcwd()
{
	platform_getcwd(buffer, 8192);
	return buffer;
}


int io_mask_close()
{
	return platform_mask_close();
}


const char* io_mask_getname()
{
	return platform_mask_getname();
}


int io_mask_getnext()
{
	return platform_mask_getnext();
}


int io_mask_isfile()
{
	return platform_mask_isfile();
}


int io_mask_open(const char* mask)
{
	return platform_mask_open(mask);
}


int io_openfile(const char* path)
{
	file = fopen(path, "w");
	if (file == NULL)
	{
		printf("** Unable to open file '%s' for writing\n", path);
		return 0;
	}
	else
	{
		return 1;
	}
}


int io_rmdir(const char* path, const char* dir)
{
	strcpy(buffer, path);
	if (strlen(buffer) > 0) 
		strcat(buffer, "/");
	strcat(buffer, dir);
	path_translateInPlace(buffer, NATIVE);
	return platform_rmdir(buffer);
}


int io_setcwd(const char* path)
{
	return platform_setcwd(path);
}


