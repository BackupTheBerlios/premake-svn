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
#include "os.h"

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
 * Locate a package by name
 ***********************************************************************/

int prj_find_package(const char* name)
{
	int i;
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		if (matches(name, project->packages[i]->name))
			return i;
	}
	return -1;
}


/************************************************************************
 * Retrieve the active configuration for the given package
 ***********************************************************************/

PkgConfig* prj_get_config_for(int i)
{
	int j;
	for (j = 0; j < prj_get_numconfigs(); ++j)
	{
		if (my_pkg->configs[j] == my_cfg)
			return project->packages[i]->configs[j];
	}
	return NULL;
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

const char* prj_get_bindir_for(int i)
{
	Package*   pkg = prj_get_package(i);
	PkgConfig* cfg = prj_get_config_for(i);

	strcpy(buffer, path_build(pkg->path, cfg->prjConfig->bindir));
	return buffer;
}

const char* prj_get_libdir()
{
	return path_build(my_pkg->path, my_cfg->prjConfig->libdir);
}

const char* prj_get_libdir_for(int i)
{
	Package*   pkg = prj_get_package(i);
	PkgConfig* cfg = prj_get_config_for(i);

	strcpy(buffer, path_build(pkg->path, cfg->prjConfig->libdir));
	return buffer;
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
	return prj_get_outdir_for(my_pkg->index);
}

const char* prj_get_outdir_for(int i)
{
	const char* targetdir;

	Package*   pkg = prj_get_package_for(i);
	PkgConfig* cfg = prj_get_config_for(i);
	
	if (matches(pkg->kind, "lib"))
		strcpy(buffer, prj_get_libdir_for(i));
	else
		strcpy(buffer, prj_get_bindir_for(i));

	targetdir = path_getdir(cfg->target);
	if (strlen(targetdir) > 0)
	{
		strcat(buffer, "/");
		strcat(buffer, targetdir);
	}

	return buffer;
}


/************************************************************************
 * Return the files associated with the active object
 ***********************************************************************/

const char** prj_get_files()
{
	return my_cfg->files;
}


/************************************************************************
 * Query the build flags
 ***********************************************************************/

int prj_has_flag(const char* flag)
{
	return prj_has_flag_for(my_pkg->index, flag);
}

int prj_has_flag_for(int i, const char* flag)
{
	PkgConfig* cfg = prj_get_config_for(i);
	const char** ptr = cfg->flags;
	while (*ptr != NULL)
	{
		if (matches(*ptr, flag))
			return 1;
		ptr++;
	}

	return 0;
}

const char** prj_get_buildoptions()
{
	return my_cfg->buildopts;
}

const char** prj_get_linkoptions()
{
	return my_cfg->linkopts;
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

const char* prj_get_language_for(int i)
{
	return project->packages[i]->lang;
}

int prj_is_lang(const char* lang)
{
	return matches(my_pkg->lang, lang);
}


/************************************************************************
 * Return the list of linker paths for the current object
 ***********************************************************************/

const char** prj_get_libpaths()
{
	return my_cfg->libpaths;
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

const char* prj_get_pkgname_for(int i)
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

Package* prj_get_package_for(int i)
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

const char* prj_get_pkgpath_for(int i)
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
 * Return the target for the active object
 ***********************************************************************/

const char* prj_get_target()
{
	return prj_get_target_for(my_pkg->index);
}

const char* prj_get_target_for(int i)
{
	const char* extension = "";

	/* Get the active configuration for this target */
	Package* pkg = prj_get_package_for(i);
	PkgConfig* cfg = prj_get_config_for(i);
	const char* filename = path_getbasename(cfg->target);

	strcpy(buffer, "");

	if (cfg->prefix != NULL)
		strcat(buffer, cfg->prefix);

	if (matches(pkg->lang, "c#"))
	{
		strcat(buffer, filename);
		if (matches(pkg->kind, "dll"))
			extension = "dll";
		else
			extension = "exe";
	}

	else if (os_is("windows"))
	{
		strcat(buffer, filename);
		if (matches(pkg->kind, "lib"))
			extension = "lib";
		else if (matches(pkg->kind, "dll"))
			extension = "dll";
		else
			extension = "exe";
	}

	else if (os_is("macosx"))
	{
		if (matches(pkg->kind, "winexe"))
		{
			strcat(buffer, filename);
			strcat(buffer, ".app/Contents/MacOS/");
			if (cfg->prefix != NULL)
				strcat(buffer, cfg->prefix);
			strcat(buffer, filename);
		}
		else if (matches(pkg->kind, "exe"))
		{
			strcat(buffer, filename);
		}
		else if (matches(pkg->kind, "dll"))
		{
			if (prj_has_flag_for(i, "dylib"))
			{
				strcat(buffer, filename);
				extension = "dylib";
			}
			else
			{
				if (cfg->prefix == NULL)
					strcat(buffer, "lib");
				strcat(buffer, filename);
				extension = "so";
			}
		}
		else
		{
			if (cfg->prefix == NULL)
				strcat(buffer, "lib");
			strcat(buffer, filename);
			extension = "a";
		}
	}

	else
	{
		if (matches(pkg->kind, "lib"))
		{
			if (cfg->prefix == NULL)
				strcat(buffer, "lib");
			strcat(buffer, filename);
			extension = "a";
		}
		else if (matches(pkg->kind, "dll"))
		{
			if (cfg->prefix == NULL)
				strcat(buffer, "lib");
			strcat(buffer, filename);
			extension = "so";
		}
		else
		{
			strcat(buffer, filename);
		}
	}

	/* Apply the file extension, which can be customized */
	if (cfg->extension != NULL)
		extension = cfg->extension;

	if (strlen(extension) > 0)
		strcat(buffer, ".");

	strcat(buffer, extension);
	
	return buffer;
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