/**********************************************************************
 * Premake - project.c
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

#include <stdlib.h>
#include "premake.h"


Project* project = NULL;

static Package*   my_pkg = NULL;
static PkgConfig* my_cfg = NULL;
static Option*    my_opt = NULL;


/************************************************************************
 * Project lifecycle routines
 ***********************************************************************/

void   prj_open()
{
	if (project != NULL)
		prj_close();
	project = ALLOCT(Project);
}


void   prj_close()
{
	if (project != NULL)
	{
		prj_freelist(project->options);
		free(project);
		project = NULL;
	}
}


/************************************************************************
 * Return the list of links for the current object
 ***********************************************************************/

const char** prj_get_links()
{
	return my_cfg->links;
}


/************************************************************************
 * Return the name of the active object
 ***********************************************************************/

const char* prj_get_cfgname()
{
	return my_cfg->prjConfig->name;
}

const char* prj_get_pkgname()
{
	return my_pkg->name;
}

const char* prj_get_pkgnamefor(int i)
{
	return project->packages[i]->name;
}


/************************************************************************
 * Return the total number of configurations in the project
 ***********************************************************************/

int prj_get_numconfigs()
{
	return prj_getlistsize(project->configs);
}


/************************************************************************
 * Return the total number of options in the project
 ***********************************************************************/

int prj_get_numoptions()
{
	return prj_getlistsize(project->options);
}


/************************************************************************
 * Return the total number of packages in the project
 ***********************************************************************/

int prj_get_numpackages()
{
	return prj_getlistsize(project->packages);
}



/************************************************************************
 * Return the name and description of the currently selected option.
 ***********************************************************************/

const char* prj_get_optdesc()
{
	return my_opt->desc;
}

const char* prj_get_optname()
{
	return my_opt->flag;
}


/************************************************************************
 * Return the active object
 ***********************************************************************/

Package* prj_get_package()
{
	return my_pkg;
}

Package* prj_get_packagefor(int i)
{
	return project->packages[i];
}


/************************************************************************
 * Return the path to the generated object scripts.
 ***********************************************************************/

const char* prj_get_path()
{
	return project->path;
}

const char* prj_get_pkgpath()
{
	return my_pkg->path;
}

const char* prj_get_pkgpathfor(int i)
{
	return project->packages[i]->path;
}


/************************************************************************
 * Return the script name for an object.
 ***********************************************************************/

const char* prj_get_script()
{
	return project->script;
}

const char* prj_get_pkgscript()
{
	return my_pkg->script;
}


/************************************************************************
 * Activate a project object.
 ***********************************************************************/

void prj_select_config(int i)
{
	if (my_pkg == NULL)
		prj_select_package(0);
	my_cfg = my_pkg->configs[i];
}

void prj_select_option(i)
{
	my_opt = project->options[i];
}

void prj_select_package(int i)
{
	my_pkg = project->packages[i];
}


/************************************************************************
 * List management routines
 ***********************************************************************/

void** prj_newlist(int len)
{
	void** list = (void**)malloc(sizeof(void*) * (len + 1));
	list[len] = NULL;
	return list;
}


void prj_freelist(void** list)
{
	int i = 0;
	while (list[i] != NULL)
		free(list[i++]);
	free(list);
}


int prj_getlistsize(void** list)
{
	int count = 0;
	void** ptr = list;
	while (*ptr != NULL)
	{
		ptr++;
		count++;
	}
	return count;
}
