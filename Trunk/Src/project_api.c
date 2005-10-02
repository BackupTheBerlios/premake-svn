#include <stdio.h>
#include <string.h>
#include "project.h"
#include "project_api.h"
#include "util.h"

static char buffer[8192];

static Package* my_pkg = NULL;
static Config*  my_cfg = NULL;


/************************************************************************
 * Return the total number of configurations in the project
 ***********************************************************************/

int prj_get_numconfigs()
{
	return project->numConfigs;
}


/************************************************************************
 * Return the total number of packages in the project
 ***********************************************************************/

int prj_get_numpackages()
{
	return project->numPackages;
}


/************************************************************************
 * Return the configuration in the specified package that matches the
 * active configuration in the active package. If the second config of
 * the active package is selected, this will return the second config
 * of the specified package.
 ***********************************************************************/

Config* prj_get_config_for(Package* pkg)
{
	int i;
	for (i = 0; i < my_pkg->numConfigs; ++i)
	{
		if (my_pkg->config[i] == my_cfg)
			return pkg->config[i];
	}
	return NULL;
}


/************************************************************************
 * Return the path to the binaries output directory relative to the
 * active package.
 ***********************************************************************/

const char* prj_get_bindir(int pathType, int appendSeparator)
{
	return prj_get_bindir_for(my_pkg, pathType, appendSeparator);
}

const char* prj_get_bindir_for(Package* pkg, int pathType, int appendSeparator)
{
	Config* cfg = prj_get_config_for(pkg);
	strcpy(buffer, reversePath(pkg->path, cfg->projectConfig->bindir, pathType, appendSeparator));
	return buffer;
}


/************************************************************************
 * Return the name of the active configuration
 ***********************************************************************/

const char* prj_get_cfgname()
{
	return my_cfg->name;
}


/************************************************************************
 * Return the language (c++, c#, etc.) used by the active set
 ***********************************************************************/

const char* prj_get_language()
{
	return my_pkg->language;
}


/************************************************************************
 * Return the path to the static library output directory relative to 
 * the active package
 ***********************************************************************/

const char* prj_get_libdir(int pathType, int appendSeparator)
{
	return prj_get_libdir_for(my_pkg, pathType, appendSeparator);
}

const char* prj_get_libdir_for(Package* pkg, int pathType, int appendSeparator)
{
	Config* cfg = prj_get_config_for(pkg);
	strcpy(buffer, reversePath(pkg->path, cfg->projectConfig->libdir, pathType, appendSeparator));
	return buffer;
}


/************************************************************************
 * Return the path to the intermediates output directory relative to
 * the active package
 ***********************************************************************/

const char* prj_get_objdir(int pathType, int appendSeparator)
{
	sprintf(buffer, "%s/%s", my_pkg->objdir, my_cfg->name);
	return translatePath(buffer, pathType);
}


/************************************************************************
 * Returns the output directory for the active configuration
 ***********************************************************************/

const char* prj_get_outdir(int pathType, int appendSeparator)
{
	return prj_get_outdir_for(my_pkg, pathType, appendSeparator);
}

const char* prj_get_outdir_for(Package* pkg, int pathType, int appendSeparator)
{
	const char* dir;

	Config* cfg = prj_get_config_for(pkg);

	if (matches("lib", pkg->kind))
		prj_get_libdir_for(pkg, pathType, 0);
	else
		prj_get_bindir_for(pkg, pathType, 0);
	
	dir = getDirectory(cfg->target, appendSeparator);
	if (strlen(dir) > 0 || appendSeparator)
		strcat(buffer, "/");
	strcat(buffer, dir);

	return translatePath(buffer, pathType);
}


/************************************************************************
 * Return the name of the active package
 ***********************************************************************/

const char* prj_get_pkgname()
{
	return my_pkg->name;
}


/************************************************************************
 * Return the path to the active package relative to the project root
 ***********************************************************************/

const char* prj_get_pkgpath()
{
	return my_pkg->path;
}


/************************************************************************
 * Return the package target name, properly formatted for the platform
 ***********************************************************************/

const char* prj_get_target()
{
	return prj_get_target_for(my_pkg);
}

const char* prj_get_target_for(Package* pkg)
{
	const char* extension = "";

	Config* cfg = prj_get_config_for(pkg);
	const char* filename = getFilename(cfg->target, 0);

	if (matches(pkg->language, "c#"))
	{
		strcpy(buffer, filename);
		if (matches(pkg->kind, "dll"))
			extension = "dll";
		else
			extension = "exe";
	}

	else if (matches(os, "windows"))
	{
		strcpy(buffer, filename);
		if (matches(pkg->kind, "lib"))
			extension = "lib";
		else if (matches(pkg->kind, "dll"))
			extension = "dll";
		else
			extension = "exe";
	}

	else if (matches(os, "macosx"))
	{
		if (matches(pkg->kind, "winexe"))
		{
			strcpy(buffer, filename);
			strcat(buffer, ".app/Contents/MacOS/");
			strcat(buffer, filename);
		}
		else if (matches(pkg->kind, "exe"))
		{
			strcpy(buffer, filename);
		}
		else if (matches(pkg->kind, "dll"))
		{
			if (inArray(cfg->buildFlags, "dylib"))
			{
				strcpy(buffer, filename);
				extension = "dylib";
			}
			else
			{
				strcpy(buffer, "lib");
				strcat(buffer, filename);
				extension = "so";
			}
		}
		else
		{
			strcpy(buffer, "lib");
			strcat(buffer, filename);
			extension = "a";
		}
	}

	else
	{
		if (matches(pkg->kind, "lib"))
		{
			strcpy(buffer, "lib");
			strcat(buffer, filename);
			extension = "a";
		}
		else if (matches(pkg->kind, "dll"))
		{
			strcpy(buffer, "lib");
			strcat(buffer, filename);
			extension = "so";
		}
		else
		{
			strcpy(buffer, filename);
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
 * Activate a new configuration within the active package
 ***********************************************************************/

void prj_select_config(int i)
{
	my_cfg = my_pkg->config[i];
}


/************************************************************************
 * Activate a new package
 ***********************************************************************/

void prj_select_package(int i)
{
	my_pkg = project->package[i];
}
