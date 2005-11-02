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
#include <string.h>
#include "premake.h"


Project* project = NULL;

static Package*   my_pkg = NULL;
static PkgConfig* my_cfg = NULL;
static Option*    my_opt = NULL;

static char buffer[8192];


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
	int i, j;

	if (project != NULL)
	{
		for (i = 0; i < prj_get_numpackages(); ++i)
		{
			Package* package = project->packages[i];

			for (j = 0; j < prj_get_numconfigs(); ++j)
			{
				PkgConfig* config = package->configs[j];
				free((void*)config->defines);
				free((void*)config->incpaths);
				free((void*)config->links);
			}

			prj_freelist(package->configs);
		}

		prj_freelist(project->options);
		prj_freelist(project->configs);
		prj_freelist(project->packages);
		free(project);
		project = NULL;
	}
}


/************************************************************************
 * Return the list of defines for the current object
 ***********************************************************************/

const char** prj_get_defines()
{
	return my_cfg->defines;
}


/************************************************************************
 * Query the object directories
 ***********************************************************************/

const char* prj_get_bindir()
{
	return path_build(my_pkg->path, my_cfg->prjConfig->bindir);
}

const char* prj_get_libdir()
{
	return path_build(my_pkg->path, my_cfg->prjConfig->libdir);
}

const char* prj_get_objdir()
{
	if (my_cfg->objdir != NULL)
		return my_cfg->objdir;
	else
		return path_combine(my_pkg->objdir, my_cfg->prjConfig->name);
}

const char* prj_get_outdir()
{
	if (matches("lib", my_pkg->kind))
	{
		strcpy(buffer, prj_get_libdir());
	}
	else
	{
		strcpy(buffer, prj_get_bindir());
	}

	/* Append target path here */

	return buffer;
}


/************************************************************************
 * Query the target kind of the current object
 ***********************************************************************/

const char* prj_get_kind()
{
	return my_pkg->kind;
}

int prj_is_kind(const char* kind)
{
	return matches(my_pkg->kind, kind);
}


/************************************************************************
 * Return the list of include paths for the current object
 ***********************************************************************/

const char** prj_get_incpaths()
{
	return my_cfg->incpaths;
}


/************************************************************************
 * Query the language of the current object
 ***********************************************************************/

const char* prj_get_language()
{
	return my_pkg->lang;
}

int prj_is_lang(const char* lang)
{
	return matches(my_pkg->lang, lang);
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
