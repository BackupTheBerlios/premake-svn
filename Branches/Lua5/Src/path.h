/**********************************************************************
 * Premake - path.h
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

/* If you add items here, also add to separators[] in path.c */
enum PathKinds
{
	NATIVE,
	POSIX,
	WINDOWS
};

const char* path_build(const char* from, const char* to);
const char* path_combine(const char* path0, const char* path1);
const char* path_getdir(const char* path);
const char* path_getname(const char* path);
const char* path_join(const char* dir, const char* name, const char* ext);
void        path_translateInPlace(char* buffer, int to);
