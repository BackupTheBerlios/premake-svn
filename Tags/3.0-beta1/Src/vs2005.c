//-----------------------------------------------------------------------------
// Premake - vs2005.c
//
// MS Visual Studio 2005 projects files
//
// Copyright (C) 2002-2005 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: vs2005.c,v 1.1 2005/08/22 01:43:41 jason379 Exp $
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"
#include "util.h"
#include "vs_helpers.h"

static int writeSolution();

//-----------------------------------------------------------------------------

int makeVs2005Scripts()
{
	printf("Generating Visual Studio 2005 solution and project files:\n");

	vs_SetTargetVersion(2005);
	vs_AssignPackageData();

	if (!writeSolution())
		return 0;

	return 1;
}


//-----------------------------------------------------------------------------

static int writeSolution()
{
	FILE* file;
	
	file = openFile(project->path, project->name, ".sln");
	if (file == NULL)
		return 0;

	fprintf(file, "Microsoft Visual Studio Solution File, Format Version 9.00\n");
	fprintf(file, "# Visual Studio 2005\n");

	/* List the projects that make up the solution */
	vs_WriteProjectList(file);

	fclose(file);
	return 1;
}
