//-----------------------------------------------------------------------------
// Premake - vs_helpers.c
//
// Support functions for the different versions of Visual Studio.
//
// Copyright (C) 2002-2005 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: vs_helpers.c,v 1.1 2005/08/22 01:43:41 jason379 Exp $
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"
#include "project_api.h"
#include "util.h"
#include "vs_helpers.h"

static int  targetVersion;
static char buffer[4096];


/************************************************************************
 * Assign GUIDs to the package. One is a unique ID for the package, the 
 * other is a predefined GUID to represent the project type (C#, C++)
 ***********************************************************************/

void vs_AssignPackageData()
{
	int i;

	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		/* Attach a package data object to all packages */
		PkgData* data = (PkgData*)malloc(sizeof(PkgData));
		prj_select_package(i);
		prj_set_packagedata(data);
	
		/* Generate a unique ID for each project in the solution */
		generateUUID(data->projGuid);

		/* Assign project type specific data */
		if (prj_is_language("c++") || prj_is_language("c"))
		{
			strcpy(data->toolGuid, "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942");
			strcpy(data->projExt, "vcproj");
			strcpy(data->projType, "Win32");
		}
		else if (prj_is_language("c#"))
		{
			strcpy(data->toolGuid, "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC");
			strcpy(data->projExt, "csproj");
			strcpy(data->projType, ".NET");
		}

		/* Initialize remaining values */
		data->numDependencies = 0;
	}
}


/************************************************************************
 * List callback: scans the list of links for a package. If a link is
 * found to a sibling package, return a dependency string for the 
 * solution file. 
 ***********************************************************************/

const char* vs_FindProjectDependencies(const char* name, void* data)
{
	int i;
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		if (matches(project->package[i]->name, name))
		{
			PkgData* data = (PkgData*)project->package[i]->data;
			if (targetVersion == 2003)
			{
				sprintf(buffer, "{%s} = {%s}", data->projGuid, data->projGuid);
			}
			else
			{
				PkgData* src = (PkgData*)prj_get_packagedata();
				sprintf(buffer, "{%s}.%d = {%s}", src->projGuid, src->numDependencies, data->projGuid);
				++(src->numDependencies);
			}
			return buffer;
		}
	}

	return NULL;
}


/************************************************************************
 * Store the target version. This is used to modify the behavior of
 * some of the function here based on VS.NET version.
 ***********************************************************************/

void vs_SetTargetVersion(int version)
{
	targetVersion = version;
}


/************************************************************************
 * Write out the list of projects for a solution.
 ***********************************************************************/

void vs_WriteProjectList(FILE* file)
{
	int i;
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		PkgData* data;
		const char* name;
		const char* path;

		prj_select_package(i);
		data = (PkgData*)prj_get_packagedata();
		name = prj_get_pkgname();
		path = prj_get_pkgpathfromprj(WINDOWS, 1);

		/* VS.NET doesn't write out the leading '.\' on solution-relative
		 * paths. Not really necessary, but I'm trying to match the original */
		if (strncmp(path, ".\\", 2) == 0)
			path = path + 2;

		fprintf(file, "Project(\"{%s}\") = \"%s\", \"%s.%s\", \"{%s}\"\n", data->toolGuid, name, path, data->projExt, data->projGuid);

		/* Write project dependency information */
		if (targetVersion == 2003)
		{
			prj_select_config(0);
			fprintf(file, "\tProjectSection(ProjectDependencies) = postProject\n");
			writeList(file, prj_get_links(), "\t\t", "\n", "", vs_FindProjectDependencies, NULL);
			fprintf(file, "\tEndProjectSection\n");
		}

		fprintf(file, "EndProject\n");
	}
}

