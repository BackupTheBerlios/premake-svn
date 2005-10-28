/**********************************************************************
 * Premake - project.h
 * An interface around the project data.
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

typedef struct tagOption
{
	const char* flag;
	const char* desc;
} Option;


typedef struct tagProject
{
	const char* name;
	const char* path;
	Option** options;
	int      numOptions;
} Project;

extern Project* project;


void   prj_open();
void   prj_close();

const char* prj_getpath();

void** prj_newlist(int len);
void   prj_freelist(void** list);

