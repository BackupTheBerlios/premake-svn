/**********************************************************************
 * Premake - vs2002.c
 * The Visual Studio 2002 and 2003 target
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "premake.h"
#include "vs2002.h"

int vs_target;

static char buffer[8192];

static int writeSolution();
static void assignPackageData();
static const char* listPackageDeps(const char* name);


int vs2002_generate(int target)
{
	int i;

	vs_target = target;

	printf("Generating Visual Studio %d solution and project files:\n", vs_target);

	/* Assign GUIDs to packages */
	assignPackageData();

	/* Generate the project files */
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);

		printf("...%s\n", prj_get_pkgname());

		if (prj_is_lang("c++") || prj_is_lang("c"))
		{
			if (!vs2002_cpp())
				return 0;
		}
		else if (prj_is_lang("c#"))
		{
			if (!vs2002_cs())
				return 0;
		}
		else
		{
			printf("** Warning: %s packages are not supported by this generator\n", prj_get_language());
		}
	}

	return writeSolution();
}


static int writeSolution()
{
	VsPkgData* data;
	int i, j;

	if (!io_openfile(path_join(prj_get_path(), prj_get_name(), "sln")))
		return 0;

	/* Format identification string */
	io_print("Microsoft Visual Studio Solution File, Format Version ");
	io_print("%d.00\n", (vs_target == 2003) ? 8 : 7);

	/* List packages */
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);
		data = (VsPkgData*)prj_get_data();

		io_print("Project(\"{%s}\") = \"%s\", \"%s\", \"{%s}\"\n", data->toolGuid, prj_get_pkgname(), prj_get_pkgfilename(data->projExt), data->projGuid);

		/* Write package dependencies for 2003 */
		if (vs_target == 2003)
		{
			prj_select_config(0);
			io_print("\tProjectSection(ProjectDependencies) = postProject\n");
			print_list(prj_get_links(), "\t\t", "\n", "", listPackageDeps);
			io_print("\tEndProjectSection\n");
		}

		io_print("EndProject\n");
	}

	/* List configurations */
	io_print("Global\n");
	io_print("\tGlobalSection(SolutionConfiguration) = preSolution\n");
	prj_select_package(0);
	for (i = 0; i < prj_get_numconfigs(); ++i)
	{
		prj_select_config(i);
		if (vs_target == 2003)
		{
			io_print("\t\t%s = %s\n", prj_get_cfgname(), prj_get_cfgname());
		}
		else
		{
			io_print("\t\tConfigName.%d = %s\n", i, prj_get_cfgname());
		}
	}
	io_print("\tEndGlobalSection\n");

	/* Write package dependencies for 2002 */
	if (vs_target == 2002)
	{
		io_print("\tGlobalSection(ProjectDependencies) = postSolution\n");
		for (i = 0; i < prj_get_numpackages(); ++i)
		{
			prj_select_package(i);
			prj_select_config(0);
			print_list(prj_get_links(), "\t\t", "\n", "", listPackageDeps);
		}
		io_print("\tEndGlobalSection\n");
	}

	/* Write configuration for each package */
	io_print("\tGlobalSection(ProjectConfiguration) = postSolution\n");
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);
		for (j = 0; j < prj_get_numconfigs(); ++j)
		{
			prj_select_config(j);
			data = (VsPkgData*)prj_get_data();
			io_print("\t\t{%s}.%s.ActiveCfg = %s|%s\n", data->projGuid, prj_get_cfgname(), prj_get_cfgname(), data->projType);
			io_print("\t\t{%s}.%s.Build.0 = %s|%s\n", data->projGuid, prj_get_cfgname(), prj_get_cfgname(), data->projType);
		}
	}
	io_print("\tEndGlobalSection\n");

	/* Finish */
	io_print("\tGlobalSection(ExtensibilityGlobals) = postSolution\n");
	io_print("\tEndGlobalSection\n");
	io_print("\tGlobalSection(ExtensibilityAddIns) = postSolution\n");
	io_print("\tEndGlobalSection\n");
	io_print("EndGlobal\n");

	io_closefile();
	return 1;
}


/************************************************************************
 * Assign GUIDs to the package. One is a unique ID for the package, the 
 * other is a predefined GUID to represent the project type (C#, C++)
 ***********************************************************************/

static void assignPackageData()
{
	int i;
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		VsPkgData* data = ALLOCT(VsPkgData);
		prj_select_package(i);
		prj_set_data(data);
	
		generateUUID(data->projGuid);

		if (prj_is_lang("c++") || prj_is_lang("c"))
		{
			strcpy(data->toolGuid, "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942");
			strcpy(data->projExt, "vcproj");
			strcpy(data->projType, "Win32");
		}
		else if (prj_is_lang("c#"))
		{
			strcpy(data->toolGuid, "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC");
			strcpy(data->projExt, "csproj");
			strcpy(data->projType, ".NET");
		}

		data->numDependencies = 0;
	}
}


/************************************************************************
 * List callback: scans the list of links for a package. If a link is
 * found to a sibling package, return a dependency string for the 
 * solution file. 
 ***********************************************************************/

static const char* listPackageDeps(const char* name)
{
	int i;
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		if (matches(prj_get_pkgname_for(i), name))
		{
			VsPkgData* data = (VsPkgData*)prj_get_data_for(i);
			if (vs_target == 2003)
			{
				sprintf(buffer, "{%s} = {%s}", data->projGuid, data->projGuid);
			}
			else
			{
				VsPkgData* src = (VsPkgData*)prj_get_data();
				sprintf(buffer, "{%s}.%d = {%s}", src->projGuid, src->numDependencies, data->projGuid);
				++(src->numDependencies);
			}
			return buffer;
		}
	}

	return NULL;
}
