/**********************************************************************
 * Premake - platform_windows.h
 * Windows-specific functions.
 *
 * Copyright (c) 2002-2005 Jason Perkins and the Premake project
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

#if defined(_WIN32)

#include <stdlib.h>
#include "platform.h"
#include "path.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static char buffer[8192];
static const char* maskPath;
static HANDLE hDir;
static WIN32_FIND_DATA entry;
static int isFirst;


int platform_chdir(const char* path)
{
	return SetCurrentDirectory(path);
}


int platform_copyfile(const char* src, const char* dest)
{
	return CopyFile(src, dest, FALSE);
}


int platform_findlib(const char* name, char* buffer, int len)
{
	HMODULE hDll = LoadLibrary(name);
	if (hDll != NULL)
	{
		GetModuleFileName(hDll, buffer, len);
		strcpy(buffer, path_getdir(buffer));
		FreeLibrary(hDll);
		return 1;
	}
	else
	{
		return 0;
	}
}

int platform_getcwd(char* buffer, int len)
{
	GetCurrentDirectory(len, buffer);
	return 1;
}


char platform_getseparator()
{
	return '\\';
}


int platform_isAbsolutePath(const char* path)
{
	return (path[0] == '/' || path[0] == '\\' || (strlen(path) > 1 && path[1] == ':'));
}


int platform_mask_close()
{
	if (hDir != INVALID_HANDLE_VALUE)
		FindClose(hDir);
	return 1;
}


const char* platform_mask_getname()
{
	strcpy(buffer, maskPath);
	if (strlen(buffer) > 0)
		strcat(buffer, "/");
	strcat(buffer, entry.cFileName);
	return buffer;
}


int platform_mask_getnext()
{
	if (hDir == INVALID_HANDLE_VALUE) 
		return 0;

	if (isFirst)
	{
		isFirst = !isFirst;
		return 1;
	}
	else
	{
		return FindNextFile(hDir, &entry);
	}
}


int platform_mask_isfile()
{
	return (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}


int platform_mask_open(const char* mask)
{
	platform_getcwd(buffer, 1024);

	maskPath = path_getdir(mask);

	hDir = FindFirstFile(mask, &entry);
	isFirst = 1;
	return (hDir != INVALID_HANDLE_VALUE);
}


int platform_mkdir(const char* path)
{
	return CreateDirectory(path, NULL);
}


int platform_rmdir(const char* path)
{
	WIN32_FIND_DATA data;
	HANDLE hDir;

	char* buffer = (char*)malloc(strlen(path) + 6);
	strcpy(buffer, path);
	strcat(buffer, "\\*.*");
	hDir = FindFirstFile(buffer, &data);
	if (hDir == INVALID_HANDLE_VALUE)
		return 0;
	free(buffer);

	do
	{
		if (strcmp(data.cFileName, ".") == 0) continue;
		if (strcmp(data.cFileName, "..") == 0) continue;
		
		buffer = (char*)malloc(strlen(path) + strlen(data.cFileName) + 2);
		strcpy(buffer, path);
		strcat(buffer, "\\");
		strcat(buffer, data.cFileName);

		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			platform_rmdir(buffer);
		else
			DeleteFile(buffer);

		free(buffer);
	} while (FindNextFile(hDir, &data));
	FindClose(hDir);

	RemoveDirectory(path);
	return 1;
}

#endif
