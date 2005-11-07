/**********************************************************************
 * Premake - platform.h
 * Platform-specific functions.
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

int         platform_chdir(const char* path);
int         platform_copyfile(const char* src, const char* dest);
int         platform_findlib(const char* name, char* buffer, int len);
int         platform_getcwd(char* buffer, int len);
char        platform_getseparator();
int         platform_isAbsolutePath(const char* path);
int         platform_mask_close();
const char* platform_mask_getname();
int         platform_mask_getnext();
int         platform_mask_isfile();
int         platform_mask_open(const char* mask);
int         platform_mkdir(const char* path);
int         platform_rmdir(const char* path);
