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


typedef struct tagPrjConfig
{
	const char* name;
} PrjConfig;

typedef struct tagPkgConfig
{
	PrjConfig*  prjConfig;
} PkgConfig;

typedef struct tagPackage
{
	const char* name;
	PkgConfig** configs;
} Package;

typedef struct tagProject
{
	const char* name;
	const char* path;
	Option**    options;
	PrjConfig** configs;
	Package**   packages;
} Project;

extern Project* project;


void        prj_open();
void        prj_close();

const char* prj_get_cfgname();
int         prj_get_numconfigs();
int         prj_get_numoptions();
int         prj_get_numpackages();
const char* prj_get_optdesc();
const char* prj_get_optname();
const char* prj_get_path();
void        prj_select_config(int i);
void        prj_select_option(int i);
void        prj_select_package(int i);

void**      prj_newlist(int len);
void        prj_freelist(void** list);
int         prj_getlistsize(void** list);

