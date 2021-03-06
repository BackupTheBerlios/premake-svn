/**********************************************************************
 * Premake - platform_posix.h
 * Windows-specific functions.
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

#include "os.h"
#if defined(PLATFORM_POSIX)

#include <stdio.h>
#include <dlfcn.h>
#include <glob.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static char buffer[8192];
static const char* maskPath;

static glob_t globbuf;
static int iGlob;


int platform_chdir(const char* path)
{
	return !chdir(path);
}


int platform_copyfile(const char* src, const char* dest)
{
	sprintf(buffer, "cp %s %s", src, dest);
	return (system(buffer) == 0);
}


static int findLibHelper(const char* lib, const char* path)
{
	struct stat sb;

	sprintf(buffer, "%s/lib%s.so", path, lib);
	if (stat(buffer, &sb) == 0 && !S_ISDIR(sb.st_mode)) return 1;

	sprintf(buffer, "%s/%s.so", path, lib);
	if (stat(buffer, &sb) == 0 && !S_ISDIR(sb.st_mode)) return 1;

	sprintf(buffer, "%s/%s", path, lib);
	if (stat(buffer, &sb) == 0 && !S_ISDIR(sb.st_mode)) return 1;

	return 0;
}

int platform_findlib(const char* name, char* buffer, int len)
{
	FILE* file;

	if (findLibHelper(name, "/usr/lib"))
	{
		strcpy(buffer, "/usr/lib");
		return 1;
	}

	file = fopen("/etc/ld.so.conf", "rt");
	if (file == NULL) 
		return 0;

	while (!feof(file))
	{
		/* Read a line and trim off any trailing whitespace */
		char linebuffer[4096];
		char* ptr;

		fgets(buffer, 4096, file);
		ptr = &buffer[strlen(buffer) - 1];
		while (isspace(*ptr))
			*(ptr--) = '\0';

		if (findLibHelper(name, buffer))
		{
			fclose(file);
			return 1;
		}
	}

	fclose(file);
	return 0;
}


int platform_getcwd(char* buffer, int len)
{
	return (getcwd(buffer, len) == 0);
}


void platform_getuuid(char* uuid)
{
	FILE* rnd = fopen("/dev/random", "rb");
	fread(uuid, 16, 1, rnd);
	fclose(rnd);
}


char platform_getseparator()
{
	return '/';
}


int platform_isAbsolutePath(const char* path)
{
	return (path[0] == '/');
}


int platform_mask_close()
{
	globfree(&globbuf);
	return 1;
}


const char* platform_mask_getname()
{
	return globbuf.gl_pathv[iGlob];
}


int platform_mask_getnext()
{
	return (++iGlob < globbuf.gl_pathc);
}


int platform_mask_isfile()
{
	return 1;
}


int platform_mask_open(const char* mask)
{
	globbuf.gl_offs = 0;
	globbuf.gl_pathc = 0;
	globbuf.gl_pathv = NULL;
	glob(mask, GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
	iGlob = -1;
	return 1;
}


int platform_mkdir(const char* path)
{
	return (mkdir(path, 0) == 0);
}


int platform_remove(const char* path)
{
	unlink(path);
	return 1;
}


int platform_rmdir(const char* path)
{
	strcpy(buffer, "rm -rf ");
	strcat(buffer, path);
	return (system(buffer) == 0);
}

#endif
