/**********************************************************************
 * Premake - io.h
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

int         io_closefile();
int         io_copyfile(const char* src, const char* dst);
const char* io_findlib(const char* name);
const char* io_getcwd();
int         io_mask_close();
const char* io_mask_getname();
int         io_mask_getnext();
int         io_mask_isfile();
int         io_mask_open(const char* mask);
int         io_openfile(const char* path);
int         io_rmdir(const char* path, const char* dir);
int         io_setcwd(const char* path);

