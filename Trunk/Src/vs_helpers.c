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
#include "util.h"
#include "vs_helpers.h"

static int  targetVersion;
static char buffer[4096];


int vs_AssignPackageData()
{
	int i;

	for (i = 0; i < project->numPackages; ++i)
	{
		/* Attach a package data object to all packages */
		Package* package = project->package[i];
		PkgData* data = (PkgData*)malloc(sizeof(PkgData));
		package->data = data;
	
		/* Generate a unique ID for each project in the solution */
		generateUUID(data->projGuid);

		/* Assign project type specific data */
		if (strcmp(package->language, "c++") == 0 || strcmp(package->language, "c") == 0)
		{
			strcpy(data->toolGuid, "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942");
			strcpy(data->projExt, "vcproj");
			strcpy(data->projType, "Win32");
		}
		else if (strcmp(package->language, "c#") == 0)
		{
			strcpy(data->toolGuid, "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC");
			strcpy(data->projExt, "csproj");
			if (targetVersion < 2005)
				strcpy(data->projType, ".NET");
			else
				strcpy(data->projType, "Any CPU");
		}
		else
		{
			printf("** Error: unrecognized language '%s'\n", package->language);
			return 0;
		}

		/* Initialize remaining values */
		data->numDependencies = 0;
	}

	return 1;
}



const char* vs_FindProjectDependencies(const char* name, void* data)
{
	int i;

	/* "name" is the name of a item found in the package.links table. Look to
	 * see if this is the name of a project within the solution and if so
	 * return dependency entry for the solution */
	for (i = 0; i < project->numPackages; ++i)
	{
		if (strcmp(project->package[i]->name, name) == 0)
		{
			PkgData* data = (PkgData*)project->package[i]->data;
			sprintf(buffer, "{%s} = {%s}", data->projGuid, data->projGuid);
			return buffer;
		}
	}

	return NULL;
}



void vs_SetTargetVersion(int version)
{
	targetVersion = version;
}


void vs_WriteProjectList(FILE* file)
{
	int i;

	/* Write the project list for the solution */
	for (i = 0; i < project->numPackages; ++i)
	{
		Package* package = project->package[i];
		PkgData* data    = package->data;
		const char* name = package->name;
		const char* path = reversePath(project->path, package->path, WINDOWS, 1);

		/* VS.NET doesn't write out the leading '.\' on solution-relative
		 * paths. Not really necessary, but I'm trying to match the original */
		if (strncmp(path, ".\\", 2) == 0)
			path = path + 2;

		fprintf(file, "Project(\"{%s}\") = \"%s\", \"%s%s.%s\", \"{%s}\"\n", data->toolGuid, name, path, name, data->projExt, data->projGuid);

		/* Write project dependency information */
		fprintf(file, "\tProjectSection(ProjectDependencies) = postProject\n");
		writeList(file, package->config[0]->links, "\t\t", "\n", "", vs_FindProjectDependencies, NULL);
		fprintf(file, "\tEndProjectSection\n");

		fprintf(file, "EndProject\n");
	}
}

