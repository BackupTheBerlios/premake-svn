/**********************************************************************
 * Premake - vs.h
 * Common code for Visual Studio 2002-2005 targets.
 *
 * Copyright (c) 2002-2006 Jason Perkins and the Premake project
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

typedef struct tagVsPkgData
{
	char projGuid[38];
	char toolGuid[38];
	char projExt[8];
	char projType[8];
	int  numDependencies;
} VsPkgData;

extern char vs_buffer[];

void vs_setversion(int version);
int  vs_getversion();

int  vs_write_solution();
void vs_assign_guids();
